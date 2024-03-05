//class to perform the combinational logic of
//the Execute stage
class ExecuteStage: public Stage
{
   private:
      void setMInput(M * mreg, uint64_t stat, uint64_t icode, 
                           uint64_t Cnd, uint64_t valE, uint64_t valA,
                           uint64_t dstE, uint64_t dstM);
      bool M_bubble;
      uint64_t Cnd;
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t aluA(uint64_t E_icode, uint64_t E_valA, uint64_t E_valC);
      uint64_t aluB(uint64_t E_icode, uint64_t E_valB);
      uint64_t alufun(uint64_t E_icode, uint64_t E_ifun);
      bool set_cc(uint64_t E_icode, uint64_t m_stat, uint64_t W_stat);
      uint64_t e_dstE(uint64_t E_icode, uint64_t E_dstE, uint64_t e_Cnd);
      void CC(uint64_t e_valE, uint64_t ALUfun, uint64_t A, uint64_t B);
      uint64_t ALU(uint64_t A, uint64_t B, uint64_t ALUfun);
      static uint64_t gete_dstE();
      static uint64_t gete_valE();
      uint64_t cond(uint64_t icode, uint64_t ifun);
      bool calculateControlSignals(uint64_t m_stat, uint64_t W_stat);
      void normalM(PipeReg ** pregs);
      void bubbleM(PipeReg ** pregs);
      uint64_t getCnd();

};
