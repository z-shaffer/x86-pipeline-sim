#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "WritebackStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"


/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool WritebackStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
	W * wreg = (W *) pregs[WREG];
	if (wreg->getstat()->getOutput() != SAOK)
	{
		return true;
	}
	else
	{
		return false; 
	}
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void WritebackStage::doClockHigh(PipeReg ** pregs)
{
	W * wreg = (W *) pregs[WREG];
	RegisterFile * rc = RegisterFile::getInstance();
	bool error = 0;
	bool *eptr;
  	eptr = &error;
	rc->writeRegister(wreg->getvalE()->getOutput(), (int32_t) wreg->getdstE()->getOutput(), *eptr);
	rc->writeRegister(wreg->getvalM()->getOutput(), (int32_t) wreg->getdstM()->getOutput(), *eptr);

}
   
