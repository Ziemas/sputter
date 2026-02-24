# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

IOP_BIN = sputter.irx
IOP_OBJS = src/sputter.o src/naxtest.o src/noisetest.o src/playsound.o \
			src/envx.o src/blockread.o src/bufdetect.o src/dmatest.o \
			src/reverb.o src/memdump.o src/slide.o src/voltest.o src/dmaspeed.o \
			src/tsatest.o src/irqtest.o imports.o
IOP_LIBS =
#IOP_TABS = stdio.tab libsd.tab ioman.tab thbase.tab sysmem.tab sysclib.tab

IOP_CFLAGS += -Wall -fno-common -Werror-implicit-function-declaration -std=c99

TEST_OBJS = ee/loader.o
TEST_EXE = loader.elf
TEST_LIBS = -ldebug
TEST_CFLAGS += -Wall -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -std=c99 -O0 -g
TEST_LDFLAGS = $(EE_LDFLAGS) -L$(PS2DEV)/gsKit/lib -L$(PS2SDK)/ports/lib

all: $(IOP_BIN) $(EE_LIB) $(TEST_EXE) never.adp sine.adp click.adp crash.adp

$(TEST_EXE): $(EE_OBJS) $(TEST_OBJS)
	$(DIR_GUARD)
	$(EE_CC) -T$(EE_LINKFILE) $(TEST_CFLAGS) -o $(TEST_EXE) $(TEST_OBJS) $(TEST_LDFLAGS) $(EXTRA_LDFLAGS) $(TEST_LIBS)

%.adp : %.wav
	$(PS2SDK)/bin/adpenc -L $< $@


#$(PS2SDK)/bin/adpenc -L $< $@

clean:
	rm -f $(IOP_BIN) $(IOP_OBJS) *.adp

run: $(IOP_BIN)
	ps2client execiop host:$(IOP_BIN)

sim: $(IOP_BIN)
	PCSX2 --elf="hello.elf"

#PCSX2 --irx=$(IOP_BIN)

reset:
	ps2client reset

include $(PS2SDK)/Defs.make
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.iopglobal
include $(PS2SDK)/samples/Makefile.eeglobal
