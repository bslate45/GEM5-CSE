#include "cpu/pred/gshare.hh"
#include "base/bitfield.hh"
#include "base/intmath.hh"

GshareBP::GshareBP(const GshareBPParams *params)
    : BPredUnit(params), 
      globalHistoryReg(params->numThreads, 0),
      instShiftAmt(params->instShiftAmt),
      globalHistoryBits(ceilLog2(params->PHTPredictorSize)),
      PHTPredictorSize(params->PHTPredictorSize),
      PHTCtrBits(params->PHTCtrBits)
{
	if (!isPowerOf2(PHTPredictorSize)) {
		fatal("Invalid size: PHT predictor!\n");
	}

	historyRegisterMask = mask(globalHistoryBits);
	PHTCtrs.resize(PHTPredictorSize);

	for(int i = 0; i < PHTPredictorSize; i++) {
		PHTCtrs[i].setBits(PHTCtrBits);
	}

    	PHTThreshold  = (ULL(1) << (PHTCtrBits  - 1)) - 1;
}

/* lookup method */
bool GshareBP::lookup(ThreadID tid, Addr branchAddr, void * &bpHistory) {
	unsigned PHTCtrsIdx = ((branchAddr >> instShiftAmt) 
				^ globalHistoryReg[tid]) 
				& historyRegisterMask;

	assert(PHTCtrsIdx < PHTPredictorSize);

	bool PHTPrediction = PHTCtrs[PHTCtrsIdx].read() 
			     > PHTThreshold;
	bool finalPrediction;

	if (PHTPrediction) {
		globalHistoryReg[tid] = (globalHistoryReg[tid] << 1) | 1;
		finalPrediction = true;
	} else {
		globalHistoryReg[tid] = (globalHistoryReg[tid] << 1);
		finalPrediction = false;
	}

	globalHistoryReg[tid] &= historyRegisterMask;

	BPHistory *history = new BPHistory;
	history->finalPred = finalPrediction;
	history->globalHistoryReg = globalHistoryReg[tid];
	bpHistory = static_cast<void*>(history);

	updateGlobalHistReg(tid, finalPrediction);

	return finalPrediction;
}

/* update method */
void GshareBP::update(ThreadID tid, Addr branchAddr, bool taken, void *bpHistory, bool squashed) {
	
	assert(bpHistory);

	BPHistory *history = static_cast<BPHistory *>(bpHistory);

	if (squashed) {
		globalHistoryReg[tid] = (history->globalHistoryReg << 1) | taken;
		return;
	}

	unsigned PHTCtrsIdx = ((branchAddr >> instShiftAmt) 
				^ history->globalHistoryReg) 
				& historyRegisterMask;

	assert(PHTCtrsIdx < PHTPredictorSize);

	if (taken) {

		PHTCtrs[PHTCtrsIdx].increment();

	} else {

		PHTCtrs[PHTCtrsIdx].decrement();

	}	

	delete history;
}

/* uncondbranch method */
void GshareBP::uncondBranch(ThreadID tid, Addr pc, void * &bpHistory) {
	BPHistory *history = new BPHistory;
	history->globalHistoryReg = globalHistoryReg[tid];
	history->finalPred = true;
	bpHistory = static_cast<void*>(history);
	updateGlobalHistReg(tid, true);
}

/* btbupdate method */
void GshareBP::btbUpdate(ThreadID tid, Addr branchAddr, void * &bpHistory) {
	globalHistoryReg[tid] &= (historyRegisterMask & ~ULL(1));
}

/* squash method */
void GshareBP::squash(ThreadID tid, void *bpHistory) {
	BPHistory *history = static_cast<BPHistory*>(bpHistory);
	globalHistoryReg[tid] = history->globalHistoryReg;

	delete history;
}

/* updateGlobalHistReg method */
void GshareBP::updateGlobalHistReg(ThreadID tid, bool taken) {

	globalHistoryReg[tid] = taken ? (globalHistoryReg[tid] << 1) | 1 : (globalHistoryReg[tid] << 1);
	globalHistoryReg[tid] &= historyRegisterMask;
}

GshareBP*
GshareBPParams::create() {
	return new GshareBP(this);
}
