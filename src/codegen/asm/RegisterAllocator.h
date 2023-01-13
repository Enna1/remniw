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
    : public std::binary_function<LiveInterval *, LiveInterval *, bool> {
    bool operator()(const LiveInterval *LHS, const LiveInterval *RHS) const {
        return LHS->StartPoint > RHS->StartPoint ||
               (LHS->StartPoint == RHS->StartPoint && LHS->EndPoint > RHS->EndPoint);
    }
};

struct LiveIntervalEndPointIncreasingOrderCompare
    : public std::binary_function<LiveInterval *, LiveInterval *, bool> {
    bool operator()(const LiveInterval *LHS, const LiveInterval *RHS) const {
        return LHS->EndPoint < RHS->EndPoint;
    }
};

class LinearScanRegisterAllocator {
private:
    const TargetInfo &TI;
    llvm::BumpPtrAllocator Alloc;
    std::priority_queue<LiveInterval *, std::vector<LiveInterval *>,
                        LiveIntervalStartPointIncreasingOrderCompare>
        Unhandled;
    std::vector<LiveInterval *> Fixed;
    std::vector<LiveInterval *> Active;
    std::vector<LiveInterval *> Spilled;
    llvm::SmallVector<bool> FreeRegisters;
    llvm::DenseMap<uint32_t, uint32_t> VirtRegToAllocatedRegMap;
    uint32_t StackSlotIndex;

public:
    LinearScanRegisterAllocator(const TargetInfo &TI): TI(TI) {
        TI.getFreeRegistersForRegisterAllocator(FreeRegisters);
    }

    ~LinearScanRegisterAllocator() {
        // for (auto *LI : Fixed)
        //     delete LI;
        // for (auto *LI : Spilled)
        //     delete LI;
        // for (auto *LI : Active)
        //     delete LI;
    }

    void
    LinearScan(const std::unordered_map<uint32_t, remniw::LiveRanges> &RegLiveRangesMap) {
        // Reset the internal states
        Alloc.Reset();
        Fixed.clear();
        Active.clear();
        Spilled.clear()
        VirtRegToAllocatedRegMap.clear();
        StackSlotIndex = 0;
        initIntervalSets(RegLiveRangesMap);

        // Do linear scan register allocation
        while (!Unhandled.empty()) {
            LiveInterval *LI = Unhandled.top();
            Unhandled.pop();
            std::sort(Active.begin(), Active.end(),
                      LiveIntervalEndPointIncreasingOrderCompare());
            ExpireOldIntervals(LI);
            uint32_t PhysReg = getFreePhysReg(LI);
            if (PhysReg != Register::NoRegister) {
                FreeRegisters[PhysReg] = false;
                VirtRegToAllocatedRegMap[LI->Reg] = PhysReg;
                Active.push_back(LI);
            } else {
                spillAtInterval(LI);
            }
        }
    }

    const llvm::DenseMap<uint32_t, uint32_t> &getVirtRegToAllocatedRegMap() {
        return VirtRegToAllocatedRegMap;
    }

    std::size_t getSpilledRegCount() { return Spilled.size(); }

    void dumpRegAllocResults() {
        for (auto p : VirtRegToAllocatedRegMap) {
            llvm::outs() << "Virtual Register: " << p.first << " assigned "
                         << p.second << "\n";
        }
        for (auto LI : Fixed) {
            llvm::outs() << "Fixed Physical Register: "
                         << LI->Reg << ", [" << LI->StartPoint
                         << "," << LI->EndPoint << ")"
                         << "\n";
        }
    }

private:
    void
    initIntervalSets(const std::unordered_map<uint32_t, LiveRanges> &RegLiveRangesMap) {
        for (const auto &p : RegLiveRangesMap) {
            if (Register::isVirtualRegister(p.first)) {
                Unhandled.push(new (Alloc) LiveInterval({p.second.Ranges.back().StartPoint,
                                                 p.second.Ranges.back().EndPoint, p.first,
                                                 p.second.Ranges.back().UsedAcrossCall}));
            }
            if (Register::isPhysicalRegister(p.first)) {
                for (const auto &Range : p.second.Ranges) {
                    Fixed.push_back(new (Alloc) LiveInterval({Range.StartPoint, Range.EndPoint,
                                                      p.first, Range.UsedAcrossCall}));
                }
            }
        }
    }

    void ExpireOldIntervals(LiveInterval *LI) {
        for (auto it = Active.begin(); it != Active.end();) {
            LiveInterval *ActiveLI = *it;
            if (ActiveLI->EndPoint > LI->StartPoint) {
                return;
            }
            uint32_t AllocatedReg = VirtRegToAllocatedRegMap[ActiveLI->Reg];
            if (Register::isPhysicalRegister(AllocatedReg)) {
                FreeRegisters[AllocatedReg] = true;
            }
            delete ActiveLI;
            it = Active.erase(it);
        }
    }

    uint32_t getFreePhysReg(LiveInterval *LI) {
        for (uint32_t Reg = 0, e = FreeRegisters.size(); Reg != e; ++Reg) {
            if (FreeRegisters[Reg] == false)
                continue;
            bool ConflictWithFixed = false;
            for (auto FixReg : Fixed) {
                if (FixReg->Reg != Reg)
                    continue;
                if (FixReg->EndPoint > LI->StartPoint ||
                    FixReg->StartPoint < LI->EndPoint) {
                    ConflictWithFixed = true;
                    break;
                }
            }
            if (ConflictWithFixed)
                continue;

            if (LI->UsedAcrossCall && !TI.isCalleeSavedRegister(Reg))
                continue;
            if (!LI->UsedAcrossCall && !TI.isCallerSavedRegister(Reg))
                continue;

            // Find an available PhysReg
            return Reg;
        }

        // No avaliable PhysReg
        return Register::NoRegister;
    }

    void spillAtInterval(LiveInterval *LI) {
        if (!Active.empty() && Active.back()->EndPoint > LI->EndPoint) {
            VirtRegToAllocatedRegMap[LI->Reg] =
                VirtRegToAllocatedRegMap[Active.back()->Reg];
            VirtRegToAllocatedRegMap[Active.back()->Reg] =
                Register::index2StackSlot(StackSlotIndex++);
            Spilled.push_back(Active.back());
            Active.pop_back();
            Active.push_back(LI);
        } else {
            VirtRegToAllocatedRegMap[LI->Reg] =
                Register::index2StackSlot(StackSlotIndex++);
            Spilled.push_back(LI);
        }
    }
};

}  // namespace remniw

#undef DEBUG_TYPE
