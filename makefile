CC = g++
CFLAGS = -g -c -Wall -std=c++11 -O0
OBJ = yess.o Memory.o Loader.o RegisterFile.o ConditionCodes.o PipeReg.o Simulate.o Tools.o D.o E.o F.o \
FetchStage.o M.o PipeRegField.o W.o DecodeStage.o ExecuteStage.o MemoryStage.o WritebackStage.o 

.C.o:
	$(CC) $(CFLAGS) $< -o $@

yess: $(OBJ)

Tools.o: Tools.h

ConditionCodes.o: ConditionCodes.h Tools.h

Loader.o: Loader.h Memory.h

Memory.o: Memory.h Tools.h

RegisterFile.o: RegisterFile.h Tools.h

ExecuteStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h E.h W.h Stage.h ExecuteStage.h Status.h Debug.h Instructions.h Tools.h \
ConditionCodes.h MemoryStage.h

DecodeStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h Stage.h MemoryStage.h DecodeStage.h Status.h \
Debug.h Instructions.h


MemoryStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h Stage.h MemoryStage.h Status.h Debug.h Tools.h \
Instructions.h Memory.h

WritebackStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h Stage.h WritebackStage.h Status.h Debug.h Instructions.h

D.o: Instructions.h RegisterFile.h PipeReg.h PipeRegField.h D.h Status.h

E.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h E.h Status.h

F.o: PipeRegField.h PipeReg.h F.h

FetchStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h E.h Stage.h FetchStage.h Status.h Debug.h Tools.h \
Memory.h Instructions.h DecodeStage.h ExecuteStage.h

M.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h M.h Status.h

PipeReg.o: PipeReg.h 

PipeRegField.o: PipeRegField.h 

Simulate.o: F.h D.h E.h M.h W.h Stage.h Simulate.h Memory.h RegisterFile.h ConditionCodes.h PipeRegField.h \
PipeReg.h ExecuteStage.h MemoryStage.h FetchStage.h DecodeStage.h WritebackStage.h

W.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h W.h Status.h

yess.o: Debug.h Memory.h Loader.h RegisterFile.h ConditionCodes.h PipeReg.h Stage.h Simulate.h 

clean:
	rm $(OBJ) yess

run:
	make clean
	make yess
	  ./run.sh
