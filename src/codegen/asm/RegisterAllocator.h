#pragma once

#include "codegen/asm/LiveInterval.h"
#include "codegen/asm/Register.h"
#include "codegen/asm/TargetInfo.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/Debug.h"
#include <algorithm>
#include <queue>
#include <vector>

#define DEBUG_TYPE "remniw-RegisterAllocator"

namespace remniw {

struct LiveIntervalStartPointIncreasingOrderCompare
    : public std::binary_function<const LiveInterval &, const LiveInterval &, bool> {
    bool operator()(const LiveInterval &LHS, const LiveInterval &RHS) const {
        return LHS.StartPoint > RHS.StartPoint ||
               (LHS.StartPoint == RHS.StartPoint && LHS.EndPoint > RHS.EndPoint);
    }
};

struct LiveIntervalEndPointIncreasingOrderCompare
    : public std::binary_function<const LiveInterval &, const LiveInterval &, bool> {
    bool operator()(const LiveInterval &LHS, const LiveInterval &RHS) const {
        return LHS.EndPoint < RHS.EndPoint;
    }
};

class LinearScanRegisterAllocator {
private:
    const TargetInfo &TI;
    std::priority_queue<LiveInterval, std::vector<LiveInterval>,
                        LiveIntervalStartPointIncreasingOrderCompare>
        Unhandled;
    llvm::SmallVector<LiveInterval> Fixed;
    llvm::SmallVector<LiveInterval> Active;
    llvm::SmallVector<LiveInterval> Spilled;
    llvm::DenseMap<uint32_t, bool> FreeRegisters;
    llvm::DenseMap<uint32_t, uint32_t> VirtRegToAllocatedRegMap;
    uint32_t StackSlotIndex;

public:
    LinearScanRegisterAllocator(const TargetInfo &TI): TI(TI) {
        llvm::ArrayRef<uint32_t> FreeRegs = TI.getFreeRegistersForRegisterAllocator();
        for (auto Reg : FreeRegs)
            FreeRegisters[Reg] = true;
    }

    void
    doRegAlloc(const std::unordered_map<uint32_t, remniw::LiveRanges> &RegLiveRangesMap) {
        // Reset the internal states
        Fixed.clear();
        Active.clear();
        Spilled.clear();
        VirtRegToAllocatedRegMap.clear();
        StackSlotIndex = 0;
        initIntervalSets(RegLiveRangesMap);

        // Do linear scan register allocation
        while (!Unhandled.empty()) {
            const LiveInterval &LI = Unhandled.top();
            std::sort(Active.begin(), Active.end(),
                      LiveIntervalEndPointIncreasingOrderCompare());
            expireOldIntervals(LI);
            uint32_t PhysReg = getFreePhysReg(LI);
            if (PhysReg != Register::NoRegister) {
                FreeRegisters[PhysReg] = false;
                VirtRegToAllocatedRegMap[LI.Reg] = PhysReg;
                Active.push_back(LI);
            } else {
                spillAtInterval(LI);
            }
            Unhandled.pop();
        }

        LLVM_DEBUG(printRegAllocResults(););
    }

    const llvm::DenseMap<uint32_t, uint32_t> &getVirtRegToAllocatedRegMap() {
        return VirtRegToAllocatedRegMap;
    }

    std::size_t getSpilledRegCount() { return Spilled.size(); }

    void printRegAllocResults() {
        for (auto p : VirtRegToAllocatedRegMap) {
            llvm::outs() << "Virtual Register: " << p.first << " assigned " << p.second
                         << "\n";
        }
        for (const auto &LI : Fixed) {
            llvm::outs() << "Fixed Physical Register: " << LI.Reg << ", ["
                         << LI.StartPoint << "," << LI.EndPoint << ")"
                         << "\n";
        }
    }

private:
    void
    initIntervalSets(const std::unordered_map<uint32_t, LiveRanges> &RegLiveRangesMap) {
        for (const auto &p : RegLiveRangesMap) {
            if (Register::isVirtualRegister(p.first)) {
                Unhandled.push(LiveInterval {p.second.Ranges.back().StartPoint,
                                             p.second.Ranges.back().EndPoint, p.first,
                                             p.second.Ranges.back().UsedAcrossCall});
            }
            if (Register::isPhysicalRegister(p.first)) {
                for (const auto &Range : p.second.Ranges) {
                    Fixed.push_back(LiveInterval {Range.StartPoint, Range.EndPoint,
                                                  p.first, Range.UsedAcrossCall});
                }
            }
        }
    }

    void expireOldIntervals(const LiveInterval &LI) {
        for (auto it = Active.begin(); it != Active.end();) {
            LiveInterval &ActiveLI = *it;
            if (ActiveLI.EndPoint > LI.StartPoint) {
                return;
            }
            uint32_t AllocatedReg = VirtRegToAllocatedRegMap[ActiveLI.Reg];
            if (Register::isPhysicalRegister(AllocatedReg)) {
                FreeRegisters[AllocatedReg] = true;
            }
            it = Active.erase(it);
        }
    }

    uint32_t getFreePhysReg(const LiveInterval &LI) {
        for (const auto &Entry : FreeRegisters) {
            if (Entry.second == false)
                continue;
            const auto &Reg = Entry.first;
            bool ConflictWithFixed = false;
            for (const auto &FixReg : Fixed) {
                if (FixReg.Reg != Reg)
                    continue;
                if (FixReg.EndPoint > LI.StartPoint || FixReg.StartPoint < LI.EndPoint) {
                    ConflictWithFixed = true;
                    break;
                }
            }
            if (ConflictWithFixed)
                continue;

            if (LI.UsedAcrossCall && !TI.isCalleeSavedRegister(Reg))
                continue;
            if (!LI.UsedAcrossCall && !TI.isCallerSavedRegister(Reg))
                continue;

            // Find an available PhysReg
            return Reg;
        }

        // No avaliable PhysReg
        return Register::NoRegister;
    }

    void spillAtInterval(const LiveInterval &LI) {
        if (!Active.empty() && Active.back().EndPoint > LI.EndPoint) {
            VirtRegToAllocatedRegMap[LI.Reg] =
                VirtRegToAllocatedRegMap[Active.back().Reg];
            VirtRegToAllocatedRegMap[Active.back().Reg] =
                Register::index2StackSlot(StackSlotIndex++);
            Spilled.push_back(Active.back());
            Active.pop_back();
            Active.push_back(LI);
        } else {
            VirtRegToAllocatedRegMap[LI.Reg] =
                Register::index2StackSlot(StackSlotIndex++);
            Spilled.push_back(LI);
        }
    }
};

}  // namespace remniw

#undef DEBUG_TYPE
