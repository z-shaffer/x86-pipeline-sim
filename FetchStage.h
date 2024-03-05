//class to perform the combinational logic of
//the Fetch stage
class FetchStage: public Stage
{
   private:
      void setDInput(D * dreg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                     uint64_t rA, uint64_t rB,
                     uint64_t valC, uint64_t valP);
      bool F_stall;
      bool D_stall;
      bool D_Bubble;
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t selectPC(F * freg, M * mreg, W * wreg);
      bool needRegIds(uint64_t f_icode);
      bool needValC(uint64_t f_icode);
      uint64_t predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP);
      uint64_t PCincrement(uint64_t f_pc, bool RegIds, bool needValC);
      void getRegIds(uint64_t f_pc, bool * eptr, uint64_t & rA, uint64_t & rB);
      uint64_t buildValC(uint64_t f_pc, bool * eptr, bool nRegIds);
      bool instr_valid(uint64_t f_icode);
      uint64_t f_stat(bool mem_error, bool instr_valid, uint64_t f_icode); 
      uint64_t f_icode(uint64_t f_icode, bool mem_error);
      uint64_t f_ifun(uint64_t f_ifun, bool mem_error);
      bool fStall(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB, uint64_t D_icode, uint64_t M_icode ); 
      bool dStall(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB); 
      bool D_bubble(uint64_t E_icode, uint64_t e_Cnd, uint64_t D_icode, uint64_t M_icode, uint64_t E_dstM, uint64_t rA, uint64_t rB );
      void calculateControlSignals(uint64_t E_icode, uint64_t d_srcA, uint64_t d_srcB, uint64_t e_Cnd, uint64_t D_icode, uint64_t M_icode, uint64_t E_dstM);
      void bubbleD(D * dreg);
      void normalD(D * dreg);
};
