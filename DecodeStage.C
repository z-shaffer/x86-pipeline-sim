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
#include "MemoryStage.h"
#include "DecodeStage.h"
#include "ExecuteStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"

bool DecodeStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
	E * ereg = (E *) pregs[EREG];
	D * dreg = (D *) pregs[DREG];
	M * mreg = (M *) pregs[MREG];
	W * wreg = (W *) pregs[WREG];
	MemoryStage * mstage = (MemoryStage *)stages[MSTAGE];
	ExecuteStage * estage = (ExecuteStage *)stages[ESTAGE];
	uint64_t icode = dreg->geticode()->getOutput(), ifun = dreg->getifun()->getOutput(), valC = dreg->getvalC()->getOutput(), valP = dreg->getvalP()->getOutput(), valA = 0, valB = 0;
	rA = dreg->getrA()->getOutput();
	rB = dreg->getrB()->getOutput();
	uint64_t dstE = RNONE, dstM = RNONE, stat = dreg->getstat()->getOutput();
	bool error = 0;
	bool *eptr;
	eptr = &error;
	rA = d_srcA(icode, rA);
	rB = d_srcB(icode, rB);
	valA = d_valA(rA, *eptr, mreg, wreg, icode, valP, mstage);
	valB = d_valB(rB, *eptr, mreg, wreg, mstage);
  	dstE = d_dstE(icode, dreg->getrB()->getOutput());
  	dstM = d_dstM(icode, dreg->getrA()->getOutput());

   	setEInput(ereg, stat, icode, ifun, valC, valA, valB, dstE, dstM, rA, rB);
   	
	calculateControlSignals(ereg->geticode()->getOutput(),ereg->getdstM()->getOutput(), rA, rB, estage->getCnd());
	return false;
}

void DecodeStage::doClockHigh(PipeReg ** pregs)
{
	E * ereg = (E *) pregs[EREG];

	if (E_bubble == false)
	{
		normalE(ereg);
	}
	else
	{
		bubbleE(ereg);
	}
}

void DecodeStage::setEInput(E * ereg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t valC, uint64_t valA,
                           uint64_t valB, uint64_t dstE, uint64_t dstM,
			   uint64_t rA, uint64_t rB)
{
	ereg->getstat()->setInput(stat);
	ereg->geticode()->setInput(icode);
	ereg->getifun()->setInput(ifun);
	ereg->getvalC()->setInput(valC);
	ereg->getvalA()->setInput(valA);
	ereg->getvalB()->setInput(valB);
	ereg->getdstE()->setInput(dstE);
	ereg->getdstM()->setInput(dstM);
	ereg->getsrcA()->setInput(rA);
	ereg->getsrcB()->setInput(rB);
}

uint64_t DecodeStage::d_srcA(uint64_t d_icode, uint64_t D_rA)
{
	if (d_icode == IRRMOVQ || d_icode == IRMMOVQ || d_icode == IOPQ || d_icode == IPUSHQ)
	{
		return D_rA;
	}
	else if (d_icode == IPOPQ || d_icode == IRET)
	{
		return RSP;
	}
	else
	{
		return RNONE;
	}
}  

uint64_t DecodeStage::d_srcB(uint64_t d_icode, uint64_t D_rB)
{
	if (d_icode == IMRMOVQ || d_icode == IRMMOVQ || d_icode == IOPQ)
	{
		return D_rB;
	}
	else if (d_icode == IPUSHQ || d_icode == IPOPQ || d_icode == ICALL || d_icode == IRET)
	{
		return RSP;
	}
	else
	{
		return RNONE;
	}
} 

 uint64_t DecodeStage::d_dstE(uint64_t d_icode, uint64_t D_rB)
{
	if (d_icode == IRRMOVQ || d_icode == IIRMOVQ || d_icode == IOPQ)
	{
		return D_rB;
	}
	else if (d_icode == IPUSHQ || d_icode == IPOPQ || d_icode == ICALL || d_icode == IRET)
	{
		return RSP;
	}
	else
	{
		return RNONE;
	}
} 

 uint64_t DecodeStage::d_dstM(uint64_t d_icode, uint64_t D_rA)
{
	if (d_icode == IMRMOVQ || d_icode == IPOPQ)
	{
		return D_rA;
	}
	else
	{
		return RNONE;
	}
}  

uint64_t DecodeStage::d_valA(uint64_t d_rA, bool &error, M * mreg, W * wreg, uint64_t D_icode, uint64_t valP, MemoryStage * mstage) // Sel+Fwd A
{
	if (D_icode == ICALL || D_icode == IJXX) {
		return valP; }
	else if (d_rA == RNONE) {
		return 0; }
	else if (d_rA == ExecuteStage::gete_dstE()) {
		return ExecuteStage::gete_valE(); }
	else if (d_rA == mreg->getdstM()->getOutput()) {
		return mstage->getm_valM(); }
	else if (d_rA ==  mreg->getdstE()->getOutput()) {
		return mreg->getvalE()->getOutput(); }
	else if (d_rA == wreg->getdstM()->getOutput()) {
		return wreg->getvalM()->getOutput(); }
	else if (d_rA == wreg->getdstE()->getOutput()) {
		return wreg->getvalE()->getOutput(); }
	RegisterFile * rc = RegisterFile::getInstance();
	return rc->readRegister(d_rA, error);
}

uint64_t DecodeStage::d_valB(uint64_t d_rB, bool &error, M * mreg, W * wreg, MemoryStage * mstage) // FwdB
{
	if (d_rB == RNONE) {
		return 0; }
	else if (d_rB == ExecuteStage::gete_dstE()) {
			return ExecuteStage::gete_valE(); }
	else if (d_rB == mreg->getdstM()->getOutput()) {
			return mstage->getm_valM(); }
	else if (d_rB == mreg->getdstE()->getOutput()) {
		return mreg->getvalE()->getOutput(); }
	else if (d_rB == wreg->getdstM()->getOutput()) {
		return wreg->getvalM()->getOutput(); }
	else if (d_rB == wreg->getdstE()->getOutput()) {
		return wreg->getvalE()->getOutput(); }
	RegisterFile * rc = RegisterFile::getInstance();
	return rc->readRegister(d_rB, error);
		
}

uint64_t DecodeStage::getd_srcA() { return rA; }
uint64_t DecodeStage::getd_srcB() { return rB; }

void DecodeStage::normalE(E * ereg)
{
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
}

void DecodeStage::bubbleE(E * ereg)
{
	ereg->getstat()->bubble(SAOK);
	ereg->geticode()->bubble(INOP);
	ereg->getifun()->bubble();
	ereg->getvalC()->bubble();
	ereg->getvalA()->bubble();
	ereg->getvalB()->bubble();
	ereg->getdstE()->bubble(RNONE);
	ereg->getdstM()->bubble(RNONE);
	ereg->getsrcA()->bubble(RNONE);
	ereg->getsrcB()->bubble(RNONE);	
}

void DecodeStage::calculateControlSignals(uint64_t icode, uint64_t dstM, uint64_t rA, uint64_t rB, uint64_t e_Cnd)
{	
	E_bubble = ( icode == IJXX && !e_Cnd ) ||  ( (icode == IMRMOVQ || icode == IPOPQ ) && ( dstM == rA || dstM ==  rB ) );
}
