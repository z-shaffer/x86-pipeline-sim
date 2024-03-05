#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "ExecuteStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "ConditionCodes.h"
#include "Tools.h"
#include "MemoryStage.h"


uint64_t dstE;
uint64_t valE;


bool ExecuteStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
	M * mreg = (M *) pregs[MREG];
	E * ereg = (E *) pregs[EREG];
	W * wreg = (W *) pregs[WREG];
	MemoryStage * mstage = (MemoryStage *)stages[MSTAGE];
	uint64_t icode = ereg->geticode()->getOutput(), ifun = ereg->getifun()->getOutput(), valA = ereg->getvalA()->getOutput();
	uint64_t dstM = ereg->getdstM()->getOutput(), stat = ereg->getstat()->getOutput();
	uint64_t m_stat = mstage->getm_stat(), W_stat = wreg->getstat()->getOutput();
	dstE = ereg->getdstE()->getOutput();
	uint64_t anum = aluA(icode, valA, ereg->getvalC()->getOutput());
	uint64_t bnum = aluB(icode, ereg->getvalB()->getOutput());
	uint64_t fnum = alufun(icode, ereg->getifun()->getOutput());
	valE = ALU(anum, bnum, fnum);
	bool cc = set_cc(icode, m_stat, W_stat);
	if (cc == true)
	{
		CC(valE, fnum, anum, bnum);
	}
	Cnd = cond(icode, ifun);
	dstE = e_dstE(icode, dstE, Cnd);
	M_bubble = calculateControlSignals(m_stat, W_stat);
	setMInput(mreg, stat, icode, Cnd, valE, valA, dstE, dstM);
	return false;
}

void ExecuteStage::doClockHigh(PipeReg ** pregs)
{
	E * ereg = (E *) pregs[EREG];
   
	ereg->getstat()->normal();
	ereg->geticode()->normal();
	ereg->getifun()->normal();
	ereg->getvalC()->normal();
	ereg->getvalA()->normal();
	ereg->getvalB()->normal();
	ereg->getdstE()->normal();
	ereg->getdstM()->normal();
	ereg->getsrcA()->normal();
	ereg->getsrcB()->normal();

	if (M_bubble == false)
	{
		normalM(pregs);
	}
	else
	{
		bubbleM(pregs);
	}
}

void ExecuteStage::setMInput(M * mreg, uint64_t stat, uint64_t icode, 
                           uint64_t Cnd, uint64_t valE, uint64_t valA,
                           uint64_t dstE, uint64_t dstM)
{
	mreg->getstat()->setInput(stat);
	mreg->geticode()->setInput(icode);
	mreg->getCnd()->setInput(Cnd);
	mreg->getvalE()->setInput(valE);
	mreg->getvalA()->setInput(valA);
	mreg->getdstE()->setInput(dstE);
	mreg->getdstM()->setInput(dstM);
}

uint64_t ExecuteStage::aluA(uint64_t E_icode, uint64_t E_valA, uint64_t E_valC)
{
	if (E_icode == IRRMOVQ || E_icode == IOPQ)
	{
		return E_valA;
	}
	else if (E_icode == IIRMOVQ || E_icode == IRMMOVQ || E_icode == IMRMOVQ)
	{
		return E_valC;
	}
	else if (E_icode == ICALL || E_icode == IPUSHQ)
	{
		return -8;
	}
	else if (E_icode == IRET || E_icode == IPOPQ)
	{
		return 8;
	}
	return 0;

}

uint64_t ExecuteStage::aluB(uint64_t E_icode, uint64_t E_valB)
{
	if (E_icode == IRMMOVQ || E_icode == IMRMOVQ || E_icode == IOPQ || E_icode == ICALL ||
		E_icode == IPUSHQ || E_icode == IRET || E_icode == IPOPQ)
	{
		return E_valB;
	}
	
	return 0;
	
}

uint64_t ExecuteStage::alufun(uint64_t E_icode, uint64_t E_ifun)
{
	if (E_icode == IOPQ)
	{
		return E_ifun;
	}
	else
	{
		return ADDQ;
	}
}

bool ExecuteStage::set_cc(uint64_t E_icode, uint64_t m_stat, uint64_t W_stat)
{
	return E_icode == IOPQ && (m_stat != SADR && m_stat != SINS && m_stat != SHLT) &&
		(W_stat != SADR && W_stat != SINS && W_stat != SHLT);
}

uint64_t ExecuteStage::e_dstE(uint64_t E_icode, uint64_t E_dstE, uint64_t e_Cnd)
{
	if (E_icode == IRRMOVQ && !e_Cnd)
	{
		return RNONE;
	}
	else
	{
		return E_dstE;
	}
}

void ExecuteStage::CC(uint64_t e_valE, uint64_t ALUfun, uint64_t A, uint64_t B)
{
	bool error = 0;
	bool *eptr;
	eptr = &error;
	ConditionCodes * CCo = ConditionCodes::getInstance();
	if ((ALUfun == 0 && Tools::addOverflow(A, B) == true) || (ALUfun == 1 && Tools::addOverflow(-A, B) == true))
	{
		CCo->setConditionCode(1, OF, *eptr);
	}
	else
	{
		CCo->setConditionCode(0, OF, *eptr);
	}

	if (e_valE == 0)
	{
		CCo->setConditionCode(1, ZF, *eptr);
		CCo->setConditionCode(0, SF, *eptr);


	}
	else if (Tools::sign(e_valE))
	{
		CCo->setConditionCode(0, ZF, *eptr);
		CCo->setConditionCode(1, SF, *eptr);
	}
	else
	{
		CCo->setConditionCode(0, ZF, *eptr);
		CCo->setConditionCode(0, SF, *eptr);

	}
}

uint64_t ExecuteStage::ALU(uint64_t A, uint64_t B, uint64_t ALUfun)
{
	if (ALUfun == 0)
	{
		return A + B;
	}
	else if (ALUfun == 1)
	{
		return B - A;
	}
	else if (ALUfun == 3)
	{
		return (A ^ B);
	}
	else if (ALUfun == 2)
	{
		return A & B;
	}
	return 0;
}

uint64_t ExecuteStage::gete_dstE()
{
	return dstE;
}

uint64_t ExecuteStage::gete_valE()
{
	return valE;
}

uint64_t ExecuteStage::cond(uint64_t icode, uint64_t ifun)
{
	bool error = 0;
	bool *eptr;
	eptr = &error;
	ConditionCodes * CCo = ConditionCodes::getInstance();
	bool over = CCo->getConditionCode(OF, *eptr);
	bool zero = CCo->getConditionCode(ZF, *eptr);
	bool sign = CCo->getConditionCode(SF, *eptr);
	if (icode == IJXX || icode == ICMOVXX)
	{
		if (ifun == UNCOND)
		{
			return 1;
		}
		else if (ifun == LESSEQ)
		{
			return ((sign ^ over) | zero);
		}
		else if (ifun == LESS)
		{
			return (sign ^ over);
		}
		else if (ifun == EQUAL)
		{
			return zero;
		}
		else if (ifun == NOTEQUAL)
		{
			return !zero;
		}
		else if (ifun == GREATER)
		{
			return (!(sign ^ over) & !zero);
		}
		else if (ifun == GREATEREQ)
		{
			return !(sign ^ over);
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

}

bool ExecuteStage::calculateControlSignals(uint64_t m_stat, uint64_t W_stat)
{
	return (m_stat == SADR || m_stat == SINS || m_stat == SHLT) || (W_stat == SADR || W_stat == SINS || W_stat == SHLT);
}

void ExecuteStage::normalM(PipeReg ** pregs)
{
	M * mreg = (M *) pregs[MREG];

	mreg->getstat()->normal();
	mreg->geticode()->normal();
	mreg->getCnd()->normal();
	mreg->getvalE()->normal();
	mreg->getvalA()->normal();
	mreg->getdstE()->normal();
	mreg->getdstM()->normal();

}

void ExecuteStage::bubbleM(PipeReg ** pregs)
{
	M * mreg = (M *) pregs[MREG];

	mreg->getstat()->bubble(SAOK);
	mreg->geticode()->bubble(INOP);
	mreg->getCnd()->bubble();
	mreg->getvalE()->bubble();
	mreg->getvalA()->bubble();
	mreg->getdstE()->bubble(RNONE);
	mreg->getdstM()->bubble(RNONE);

}

uint64_t ExecuteStage::getCnd() { return Cnd; }
