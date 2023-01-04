TARGET = libophiuchus.a
CFLAGS = -Wall -O2 -Wno-unused-but-set-variable # -g -DDEBUG
CC = cc
FRAMES = -framework IOKit -framework CoreFoundation

SRC = ScsiCmdExec.c Inquiry.c ReadBlockLimits.c Read.c Write.c \
	ModeSenseSelect.c CheckSenseStat.c Rewind.c TestUnitReady.c \
	LoadUnload.c Erase .c Space.c WriteFilemarks.c ReserveRelease.c \
	DensityCode.c DebugLog.c Ophiuchus.c
OBJ = ScsiCmdExec.o Inquiry.o ReadBlockLimits.o Read.o Write.o \
	ModeSenseSelect.o CheckSenseStat.o Rewind.o TestUnitReady.o \
	LoadUnload.o Erase.o Space.o WriteFilemarks.o ReserveRelease.o \
	DensityCode.o DebugLog.o Ophiuchus.o
HDR = ScsiCmdExec.h Inquiry.h ReadBlockLimits.h Read.h Write.h \
	ModeSenseSelect.h CheckSenseStat.h Rewind.h TestUnitReady.h \
	LoadUnload.h Erase.h Space.h WriteFilemarks.h ReserveRelease.h \
	DensityCode.h DebugLog.h ScsiDevInfo.h

all:	${TARGET}

${TARGET}: ${OBJ}
	ar cru $@ ${OBJ}
	ranlib $@

clean:
	/bin/rm -f *.o ${TARGET}

Inquiry.o: Inquiry.h CheckSenseStat.h ScsiDevInfo.h ScsiCmdExec.h DebugLog.h
ReadBlockLimits.o: ReadBlockLimits.h CheckSenseStat.h ScsiDevInfo.h \
	ScsiCmdExec.h DebugLog.h
Read.o: Read.h CheckSenseStat.h ScsiDevInfo.h ScsiCmdExec.h DebugLog.h
Write.o: Write.h CheckSenseStat.h ScsiDevInfo.h ScsiCmdExec.h DebugLog.h
ModeSenseSelect.o: ModeSenseSelect.h CheckSenseStat.h ScsiDevInfo.h \
	ScsiCmdExec.h DebugLog.h
CheckSenseStat.o: CheckSenseStat.h ScsiDevInfo.h ScsiCmdExec.h DebugLog.h
Rewind.o: Rewind.h CheckSenseStat.h ScsiDevInfo.h ScsiCmdExec.h DebugLog.h
TestUnitReady.o: TestUnitReady.h CheckSenseStat.h ScsiDevInfo.h ScsiCmdExec.h \
	DebugLog.h
LoadUnload.o: LoadUnload.h CheckSenseStat.h ScsiDevInfo.h ScsiCmdExec.h \
	DebugLog.h
Erase.o: Erase.h CheckSenseStat.h ScsiDevInfo.h ScsiCmdExec.h DebugLog.h
Space.o: Space.h CheckSenseStat.h ScsiDevInfo.h ScsiCmdExec.h DebugLog.h
WriteFilemarks.o: WriteFilemarks.h CheckSenseStat.h ScsiDevInfo.h \
	ScsiCmdExec.h DebugLog.h
ReserveRelease.o: ReserveRelease.h CheckSenseStat.h ScsiDevInfo.h \
	ScsiCmdExec.h DebugLog.h
DensityCode.o: DensityCode.h ScsiDevInfo.h DebugLog.h
DebugLog.o: DebugLog.h
Ophiuchus.o: Ophiuchus.h Inquiry.h ReadBlockLimits.h CheckSenseStat.h \
	ScsiDevInfo.h DebugLog.h Space.h
