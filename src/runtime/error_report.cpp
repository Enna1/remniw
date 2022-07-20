//===----------------------------------------------------------------------===//
//
// The idea and implementation of remniw's guarded_pool_allocator is heavily
// borromed from llvm-project/compiler-rt/lib/gwp_asan/
//
//===----------------------------------------------------------------------===//

#include "error_report.h"
#include "guarded_pool_allocator.h"
#include <cassert>
#include <signal.h>
#include <string.h>
#include <unistd.h>

namespace {

aphotic_shield::GuardedPoolAllocator *GPAForSignalHandler;

void Print(const char *Data) {
    (void)write(2, Data, strlen(Data));
}

bool __aphotic_shield_error_is_mine(const aphotic_shield::AllocatorState *State,
                                    uintptr_t ErrorPtr) {
    assert(State && "State should not be nullptr.");
    if (State->FailureType != aphotic_shield::Error::UNKNOWN &&
        State->FailureAddress != 0)
        return true;

    return State->GuardedPagePool <= ErrorPtr && ErrorPtr < State->GuardedPagePoolEnd;
}

uintptr_t
__aphotic_shield_get_internal_crash_address(const aphotic_shield::AllocatorState *State) {
    return State->FailureAddress;
}

static const aphotic_shield::AllocationMetadata *
addrToMetadata(const aphotic_shield::AllocatorState *State,
               const aphotic_shield::AllocationMetadata *Metadata, uintptr_t Ptr) {
    return &Metadata[State->getNearestSlot(Ptr)];
}

aphotic_shield::Error
__aphotic_shield_diagnose_error(const aphotic_shield::AllocatorState *State,
                                const aphotic_shield::AllocationMetadata *Metadata,
                                uintptr_t ErrorPtr) {
    if (!__aphotic_shield_error_is_mine(State, ErrorPtr))
        return aphotic_shield::Error::UNKNOWN;

    // We set State->FailureType in GuardedPoolAllocator::deallocate, if we encounter
    // invalid-free or double-free.
    if (State->FailureType != aphotic_shield::Error::UNKNOWN)
        return State->FailureType;

    // Let's try and figure out what the source of this error is.
    if (State->isGuardPage(ErrorPtr)) {
        size_t Slot = State->getNearestSlot(ErrorPtr);
        const aphotic_shield::AllocationMetadata *SlotMeta =
            addrToMetadata(State, Metadata, State->slotToAddr(Slot));

        // Ensure that this slot was allocated once upon a time.
        if (!SlotMeta->Addr)
            return aphotic_shield::Error::UNKNOWN;

        if (SlotMeta->Addr < ErrorPtr)
            return aphotic_shield::Error::BUFFER_OVERFLOW;
        return aphotic_shield::Error::BUFFER_UNDERFLOW;
    }

    // Access wasn't a guard page, check for use-after-free.
    const aphotic_shield::AllocationMetadata *SlotMeta =
        addrToMetadata(State, Metadata, ErrorPtr);
    if (SlotMeta->IsDeallocated) {
        return aphotic_shield::Error::USE_AFTER_FREE;
    }

    // If we have reached here, the error is still unknown.
    return aphotic_shield::Error::UNKNOWN;
}

void dumpReport(uintptr_t ErrorPtr, const aphotic_shield::AllocatorState *State,
                const aphotic_shield::AllocationMetadata *Metadata, void *Context) {
    assert(State && "dumpReport missing Allocator State.");
    assert(Metadata && "dumpReport missing Metadata.");

    if (!__aphotic_shield_error_is_mine(State, ErrorPtr))
        return;

    Print("*** APHOTIC-SHIELD detected a memory error ***\n");

    // ErrorPtr is initialized to siginfo->si_addr which indicates the segmentation
    // fault(SIGSEGV) signal is caused by accessing this memory address.
    // If we set a more accurate crash address in AllocatorState::FailureAddress(e.g.
    // SIGSEGV is trapped in GuardedPoolAllocator::deallocate when invalid-free or
    // double-free happens), update ErrorPtr to it.
    uintptr_t InternalErrorPtr = __aphotic_shield_get_internal_crash_address(State);
    if (InternalErrorPtr != 0u)
        ErrorPtr = InternalErrorPtr;

    aphotic_shield::Error E = __aphotic_shield_diagnose_error(State, Metadata, ErrorPtr);

    constexpr size_t kErrorMessageBufferLen = 128;
    char ErrorMessageBuffer[kErrorMessageBufferLen] = "";
    switch (E) {
    case aphotic_shield::Error::UNKNOWN:
        snprintf(ErrorMessageBuffer, kErrorMessageBufferLen,
                 "Unknown error on address 0x%zx\n", ErrorPtr);
        Print(ErrorMessageBuffer);
        Print("APHOTIC-SHIELD cannot provide any more information about this error.\n");
    case aphotic_shield::Error::USE_AFTER_FREE:
        snprintf(ErrorMessageBuffer, kErrorMessageBufferLen,
                 "Use After Free on address 0x%zx\n", ErrorPtr);
        Print(ErrorMessageBuffer);
        break;
    case aphotic_shield::Error::DOUBLE_FREE:
        snprintf(ErrorMessageBuffer, kErrorMessageBufferLen,
                 "Double Free on address 0x%zx\n", ErrorPtr);
        Print(ErrorMessageBuffer);
        break;
    case aphotic_shield::Error::INVALID_FREE:
        snprintf(ErrorMessageBuffer, kErrorMessageBufferLen,
                 "Invalid (Wild) Free on address 0x%zx\n", ErrorPtr);
        Print(ErrorMessageBuffer);
        break;
    case aphotic_shield::Error::BUFFER_OVERFLOW:
        snprintf(ErrorMessageBuffer, kErrorMessageBufferLen,
                 "Buffer Overflow on address 0x%zx\n", ErrorPtr);
        Print(ErrorMessageBuffer);
        break;
    case aphotic_shield::Error::BUFFER_UNDERFLOW:
        snprintf(ErrorMessageBuffer, kErrorMessageBufferLen,
                 "Buffer Underflow on address 0x%zx\n", ErrorPtr);
        Print(ErrorMessageBuffer);
        break;
    }
}

static void sigSegvHandler(int sig, siginfo_t *info, void *ucontext) {
    // Print error message.
    dumpReport(reinterpret_cast<uintptr_t>(info->si_addr),
               GPAForSignalHandler->getAllocatorState(),
               GPAForSignalHandler->getMetadataRegion(), ucontext);
    // Raise the default handler, cause a segmentation fault (core dumped).
    signal(SIGSEGV, SIG_DFL);
    raise(SIGSEGV);
}

}  // namespace

namespace aphotic_shield {

void installSegvSignalHandler(GuardedPoolAllocator *GPA) {
    assert(GPA && "GPA wasn't provided to installSignalHandlers");
    GPAForSignalHandler = GPA;
    struct sigaction Action = {};
    Action.sa_sigaction = sigSegvHandler;
    Action.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &Action, nullptr);
}

}  // namespace aphotic_shield