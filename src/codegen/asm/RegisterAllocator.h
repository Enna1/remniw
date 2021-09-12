#pragma once

#include "LiveInterval.h"
#include "Register.h"
#include "llvm/Support/Debug.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <unordered_map>

#define DEBUG_TYPE "remniw-lsra"

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
    std::vector<LiveInterval *> LiveIntervals;
    std::priority_queue<LiveInterval *, std::vector<LiveInterval *>,
                        LiveIntervalStartPointIncreasingOrderCompare>
        Unhandled;
    std::vector<LiveInterval *> Fixed;
    std::vector<LiveInterval *> Active;
    std::vector<LiveInterval *> Spilled;
    llvm::SmallVector<bool, 32> FreeRegisters;
    std::unordered_map<uint32_t, uint32_t> VirtRegToAllocatedRegMap;
    uint32_t StackSlotIndex;
public:
    LinearScanRegisterAllocator(
        std::unordered_map<uint32_t, remniw::LiveRanges> &RegLiveRangesMap) {
        initIntervalSets(RegLiveRangesMap);
        initFreeRegisters();
    }

    ~LinearScanRegisterAllocator() {
        for (auto *LI : Fixed)
            delete LI;
        for (auto *LI : Spilled)
            delete LI;
        for (auto *LI : Active)
            delete LI;
    }

    void LinearScan() {
        StackSlotIndex = 0;
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
        LLVM_DEBUG({
            llvm::outs() << "===== LSRA ===== \n";
            dumpRegAllocResults();
        });
    }

    std::unordered_map<uint32_t, uint32_t> &getVirtRegToAllocatedRegMap() {
        return VirtRegToAllocatedRegMap;
    }

    std::size_t getSpilledRegCount() { return Spilled.size(); }

private:
    void initIntervalSets(std::unordered_map<uint32_t, LiveRanges> &RegLiveRangesMap) {
        Active.clear();
        for (const auto &p : RegLiveRangesMap) {
            if (Register::isVirtualRegister(p.first)) {
                Unhandled.push(new LiveInterval({p.second.Ranges.back().StartPoint,
                                                 p.second.Ranges.back().EndPoint, p.first,
                                                 p.second.Ranges.back().UsedAcrossCall}));
            }
            if (Register::isPhysicalRegister(p.first)) {
                for (const auto &Range : p.second.Ranges) {
                    Fixed.push_back(new LiveInterval({Range.StartPoint, Range.EndPoint,
                                                      p.first, Range.UsedAcrossCall}));
                }
            }
        }
    }

    void initFreeRegisters() {
        FreeRegisters.resize(16 /*number of registers*/ + 1);
        FreeRegisters[Register::NoRegister /*0*/] = false;
        // Caller saved registers
        FreeRegisters[Register::RAX /*1*/] = true;
        FreeRegisters[Register::RDI /*2*/] = true;
        FreeRegisters[Register::RSI /*3*/] = true;
        FreeRegisters[Register::RDX /*4*/] = true;
        FreeRegisters[Register::RCX /*5*/] = true;
        FreeRegisters[Register::R8 /*6*/] = true;
        FreeRegisters[Register::R9 /*7*/] = true;
        FreeRegisters[Register::R10 /*8*/] = true;
        FreeRegisters[Register::R11 /*9*/] = true;
        // Callee saved registers
        FreeRegisters[Register::RSP /*10*/] = false;
        FreeRegisters[Register::RBP /*11*/] = false;
        FreeRegisters[Register::RBX /*12*/] = true;
        FreeRegisters[Register::R12 /*13*/] = true;
        FreeRegisters[Register::R13 /*14*/] = true;
        FreeRegisters[Register::R14 /*15*/] = true;
        FreeRegisters[Register::R15 /*16*/] = true;
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

            if (LI->UsedAcrossCall && !Register::isCalleeSavedRegister(Reg))
                continue;
            if (!LI->UsedAcrossCall && !Register::isCallerSavedRegister(Reg))
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

    void dumpRegAllocResults() {
        for (auto p : VirtRegToAllocatedRegMap) {
            std::cout << "Virtual Register: " << p.first << " assigned "
                      << Register::convertRegisterToString(p.second) << std::endl;
        }
        for (auto LI : Fixed) {
            std::cout << "Fixed Physical Register: "
                      << Register::convertRegisterToString(LI->Reg) << ", ["
                      << LI->StartPoint << "," << LI->EndPoint << ")" << std::endl;
        }
        std::cout << "======\n";
    }
};

}  // namespace remniw
