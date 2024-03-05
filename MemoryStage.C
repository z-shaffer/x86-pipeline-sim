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
#include "MemoryStage.h"
#include "Memory.h"
#include "Tools.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"

bool MemoryStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
	M * mreg = (M *) pregs[MREG];
	W * wreg = (W *) pregs[WREG];
	uint64_t icode = mreg->geticode()->getOutput(), valE = mreg->getvalE()->getOutput(), valA = mreg->getvalA()->getOutput();
	uint64_t dstE = mreg->getdstE()->getOutput(), dstM = mreg->getdstM()->getOutput();
	bool error = 0; 
	bool *eptr; 
	eptr = &error; 
	Memory * mem = Memory::getInstance(); 
	valM = 0;
	stat = mreg->getstat()->getOutput();
	bool mem_read = icode == IMRMOVQ || icode == IPOPQ || icode == IRET;
	bool mem_write = icode == IRMMOVQ || icode == IPUSHQ || icode == ICALL;
	int32_t addr = mem_addr(icode, valE, valA); 
	if (mem_read) { valM =  mem->getLong(addr, *eptr); } 
	if (mem_write) { mem->putLong(valA, addr, *eptr); }
	stat = m_stat(stat, error);
	setWInput(wreg, icode, stat, valE, valM, dstE, dstM);
	return false;
}

void MemoryStage::doClockHigh(PipeReg ** pregs)
{
	W * wreg = (W *) pregs[WREG];
	M * mreg = (M *) pregs[MREG];
  
	mreg->getstat()->normal();
	mreg->geticode()->normal();
	mreg->getCnd()->normal();
	mreg->getvalE()->normal();
	mreg->getvalA()->normal();
	mreg->getdstE()->normal();
	mreg->getdstM()->normal();
	wreg->geticode()->normal();
	wreg->getstat()->normal();
	wreg->getvalE()->normal();
	wreg->getvalM()->normal();
	wreg->getdstE()->normal();
	wreg->getdstM()->normal();
}

void MemoryStage::setWInput(W * wreg, uint64_t icode, uint64_t stat, uint64_t valE, 
                           uint64_t valM, uint64_t dstE, uint64_t dstM)
{
	wreg->geticode()->setInput(icode);
	wreg->getstat()->setInput(stat);
	wreg->getvalE()->setInput(valE);
	wreg->getvalM()->setInput(valM);
	wreg->getdstE()->setInput(dstE);
	wreg->getdstM()->setInput(dstM);
}

uint64_t MemoryStage::getm_valM()
{
	return valM;
}

uint64_t MemoryStage::mem_addr(uint64_t M_icode, uint64_t M_valE, uint64_t M_valA)
{
	if (M_icode == IRMMOVQ || M_icode == IPUSHQ || M_icode == ICALL || M_icode == IMRMOVQ)
	{
		return M_valE;
	}
	else if (M_icode == IPOPQ || M_icode == IRET)
	{
		return M_valA;
	}
	return 0;
}

uint64_t MemoryStage::getm_stat()
{
	return stat;
}

uint64_t MemoryStage::m_stat(uint64_t M_stat, bool mem_error)
{
	if (mem_error)
	{
		return SADR;
	}
	else 
	{
		return M_stat;
	}
}
