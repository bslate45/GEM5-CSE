#ifndef __CPU_PRED_GSHARE_PRED_HH__
#define __CPU_PRED_GSHARE_PRED_HH__

#include <vector>

#include "base/types.hh"
#include "cpu/pred/bpred_unit.hh"
#include "cpu/pred/sat_counter.hh"
#include "params/GshareBP.hh"


class GshareBP : public BPredUnit
{
  public:
    GshareBP(const GshareBPParams *params);
    bool lookup(ThreadID tid, Addr branch_addr, void * &bp_history);
    void update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history, bool squashed);
    void uncondBranch(ThreadID tid, Addr pc, void * &bpHistory);
    void btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history);
    void squash(ThreadID tid, void *bp_history);

  private:
    void updateGlobalHistReg(ThreadID tid, bool taken);

    struct BPHistory {
        unsigned globalHistoryReg;
        bool finalPred;
    };

    std::vector<unsigned> globalHistoryReg;
    std::vector<SatCounter> PHTCtrs;

    unsigned instShiftAmt;
    unsigned globalHistoryBits;
    unsigned historyRegisterMask;
    unsigned PHTPredictorSize;
    unsigned PHTCtrBits;
    unsigned PHTPredictorMask;
    unsigned PHTThreshold;
};

#endif // __CPU_PRED_GSHARE_PRED_HH__

