//class to perform the combinational logic of
//the Decode stage
class DecodeStage: public Stage
{
   private:
      void setEInput(E * ereg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t valC, uint64_t valA,
                           uint64_t valB, uint64_t dstE, uint64_t dstM,
			   uint64_t rA, uint64_t rB);
 	uint64_t rA;
	uint64_t rB;
	bool E_bubble;
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t d_srcA(uint64_t D_icode, uint64_t D_rA);
      uint64_t d_srcB(uint64_t D_icode, uint64_t D_rB);
      uint64_t d_dstE(uint64_t D_icode, uint64_t D_rB);
      uint64_t d_dstM(uint64_t D_icode, uint64_t D_rA);
      uint64_t d_valA(uint64_t D_rA, bool &error, M * mreg, W * wreg, uint64_t icode, uint64_t valP, MemoryStage * mstage);
      uint64_t d_valB(uint64_t D_rB, bool &error, M * mreg, W * wreg, MemoryStage * mstage);
      uint64_t getd_srcA();
      uint64_t getd_srcB();
      void normalE(E * ereg);
      void bubbleE(E * ereg);
      void calculateControlSignals(uint64_t icode, uint64_t dstM, uint64_t rA, uint64_t rB, uint64_t e_Cnd);
};
