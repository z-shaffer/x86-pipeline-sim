#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "E.h"
#include "Stage.h"
#include "FetchStage.h"
#include "Status.h"
#include "Debug.h"
#include "Memory.h"
#include "Tools.h"
#include "Instructions.h"
#include "MemoryStage.h"
#include "DecodeStage.h"
#include "ExecuteStage.h"

/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool FetchStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
	F * freg = (F *) pregs[FREG];
	D * dreg = (D *) pregs[DREG];
	uint64_t f_pc = 0, icode = 0, ifun = 0, valC = 0, valP = 0;
	uint64_t rA = RNONE, rB = RNONE, stat = SAOK;
	DecodeStage * dstage = (DecodeStage *)stages[DSTAGE];
	ExecuteStage * estage = (ExecuteStage *)stages[ESTAGE];

	//Fetching the instruction will allow the icode, ifun,
	//rA, rB, and valC to be set.
	M * mreg = (M *) pregs[MREG];
	W * wreg = (W *) pregs[WREG];
	Memory * m = Memory::getInstance();
	bool error = 0;
	bool *eptr;
	eptr = &error;
	f_pc = selectPC(freg, mreg, wreg);
	uint8_t byte = m->getByte((int32_t) f_pc, *eptr);
   	if (error == false)
	{
		icode = Tools::getBits(byte, 4, 7);
		ifun = Tools::getBits(byte, 0, 3);
	}
	icode = f_icode(icode, error);
	ifun = f_ifun(ifun, error);
	bool nRegIds = needRegIds(icode);
	bool nValC = needValC(icode);
	valP = PCincrement(f_pc, nRegIds,
				nValC);
	if (nRegIds == true)
	{
		getRegIds(f_pc, eptr, rA, rB);
	}

	if (nValC == true)
	{
		valC = buildValC(f_pc, eptr, nRegIds);
	}
	bool instrValid = instr_valid(icode);
	stat = f_stat(error, instrValid, icode);

	E * ereg = (E *) pregs[EREG];
   
	uint64_t E_icode = ereg->geticode()->getOutput();
	uint64_t E_dstM = ereg->getdstM()->getOutput();
	uint64_t e_Cnd = estage->getCnd();
	uint64_t D_icode = dreg->geticode()->getOutput();
	uint64_t M_icode = mreg->geticode()->getOutput();
	freg->getpredPC()->setInput(predictPC(icode, valC, valP));

	setDInput(dreg, stat, icode, ifun, rA, rB, valC, valP);
	calculateControlSignals(E_icode, dstage->getd_srcA(), dstage->getd_srcB(), e_Cnd, D_icode, M_icode, E_dstM);
	return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeReg ** pregs)
{
	F * freg = (F *) pregs[FREG];
	D * dreg = (D *) pregs[DREG];
	if (F_stall == false)
	{
		freg->getpredPC()->normal();
	}

	if (D_Bubble == true)
	{
		bubbleD(dreg);
	}
	else if (D_stall == false)
	{
		normalD(dreg);
	}
}

/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: dreg - pointer to the D register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
*/
void FetchStage::setDInput(D * dreg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t rA, uint64_t rB,
                           uint64_t valC, uint64_t valP)
{
	dreg->getstat()->setInput(stat);
	dreg->geticode()->setInput(icode);
	dreg->getifun()->setInput(ifun);
	dreg->getrA()->setInput(rA);
	dreg->getrB()->setInput(rB);
	dreg->getvalC()->setInput(valC);
	dreg->getvalP()->setInput(valP);
}
 
uint64_t FetchStage::selectPC(F * freg, M * mreg, W * wreg)
{
	if (mreg->geticode()->getOutput() == 7 && !(mreg->getCnd()->getOutput()))
	{
		return (mreg->getvalA()->getOutput());
	}
	else if (wreg->geticode()->getOutput() == 9)
	{
		return (wreg->getvalM()->getOutput());
	}
	else
	{
		return (freg->getpredPC()->getOutput());
	}
}

bool FetchStage::needRegIds(uint64_t f_icode)
{
	return (f_icode == IRRMOVQ || f_icode == IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ || f_icode == IOPQ
		 || f_icode == IPUSHQ || f_icode == IPOPQ);
}

bool FetchStage::needValC(uint64_t f_icode)
{
	return (f_icode ==  IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ || f_icode == IJXX || f_icode == ICALL);
}

uint64_t FetchStage::predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP)
{
	if (f_icode == IJXX || f_icode == ICALL)
	{
		return f_valC;
	}
	else
	{
		return f_valP;
	}
}

uint64_t FetchStage::PCincrement(uint64_t f_pc, bool RegIds, bool needValC)
{
	if (RegIds == true && needValC == true)
	{
		return f_pc + 10;
	}
	else if (RegIds == true)
	{
		return f_pc + 2;
	}
	else if (needValC == true)
	{
		return f_pc + 9;
	}
	else
	{
		return f_pc + 1;
	}
}

void FetchStage::getRegIds(uint64_t f_pc, bool * eptr, uint64_t & rA, uint64_t & rB) 
{
	Memory * m = Memory::getInstance();
	uint8_t bytes = m->getByte(f_pc + 1, *eptr);

	rA = Tools::getBits(bytes, 4, 7);
	rB = Tools::getBits(bytes, 0, 3);
}

uint64_t FetchStage::buildValC(uint64_t f_pc, bool * eptr, bool nRegIds)
{
	Memory * m = Memory::getInstance();
	if (nRegIds == true)
	{
		uint8_t array[8];
		for (int i = 0; i < 8; i++)
		{
			array[i] = m->getByte(f_pc + 2 + i, *eptr);
		} 
		return Tools::buildLong(array);
	}
	else
	{
		uint8_t array[8];
		for (int i = 0; i < 8; i++)
		{
			array[i] = m->getByte(f_pc + 1 + i, *eptr);
		} 
		return Tools::buildLong(array);
	}
}

bool FetchStage::instr_valid(uint64_t f_icode)
{
	return (f_icode == INOP || f_icode == IHALT || f_icode == IRRMOVQ || f_icode == IIRMOVQ || 
			f_icode == IRMMOVQ || f_icode == IMRMOVQ || f_icode == IOPQ || f_icode == IJXX || 
			f_icode == ICALL || f_icode == IRET || f_icode == IPUSHQ || f_icode == IPOPQ);
}

uint64_t FetchStage::f_stat(bool mem_error, bool instr_valid, uint64_t f_icode)
{
	if (mem_error == true)
	{
		return SADR;
	}
	else if (instr_valid == false)
	{
		return SINS;
	}
	else if (f_icode == IHALT)
	{
		return SHLT;
	}
	return SAOK;
}

uint64_t FetchStage::f_icode(uint64_t f_icode, bool mem_error) 
{
	if (mem_error)
	{
		return INOP;
	}
	return f_icode;
}

uint64_t FetchStage::f_ifun(uint64_t f_ifun, bool mem_error) 
{
	if (mem_error)
	{
		return FNONE;
	}
	return f_ifun;
}

bool FetchStage::fStall(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB, uint64_t D_icode, uint64_t M_icode)
{
	return ((E_icode == IMRMOVQ || E_icode == IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB)) || (IRET == D_icode || IRET == E_icode || IRET == M_icode) ;
}

bool FetchStage::dStall(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB)
{
	return (E_icode == IMRMOVQ || E_icode == IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB);
}

bool FetchStage::D_bubble(uint64_t E_icode, uint64_t e_Cnd, uint64_t D_icode, uint64_t M_icode, uint64_t E_dstM, uint64_t rA, uint64_t rB)
{
	return (E_icode == IJXX && !e_Cnd) || (!((E_icode == IMRMOVQ || E_icode == IPOPQ) && (E_dstM == rA || E_dstM == rB )) && (IRET == D_icode || IRET == E_icode || IRET == M_icode));
}

void FetchStage::bubbleD(D * dreg)
{
	dreg->getstat()->bubble(SAOK);
	dreg->geticode()->bubble(INOP);
	dreg->getifun()->bubble();
	dreg->getrA()->bubble(RNONE);
	dreg->getrB()->bubble(RNONE);
	dreg->getvalC()->bubble();
	dreg->getvalP()->bubble();
}

void FetchStage::normalD(D * dreg)
{
	dreg->getstat()->normal();
	dreg->geticode()->normal();
	dreg->getifun()->normal();
	dreg->getrA()->normal();
	dreg->getrB()->normal();
	dreg->getvalC()->normal();
	dreg->getvalP()->normal();
}

void FetchStage::calculateControlSignals(uint64_t E_icode, uint64_t d_srcA, uint64_t d_srcB, uint64_t e_Cnd, uint64_t D_icode, uint64_t M_icode, uint64_t E_dstM)
{
	F_stall = fStall(E_icode, E_dstM, d_srcA, d_srcB, D_icode, M_icode);
	D_stall = dStall(E_icode, E_dstM, d_srcA, d_srcB);
	D_Bubble = D_bubble(E_icode, e_Cnd, D_icode, M_icode, E_dstM, d_srcA, d_srcB);
}
