#include "sputter.h"

static const char *TESTFILE2 = "host0:takanaka.adp";
static const int filesize2 = 1240720;

// static const u32 SPU_IRQ_LOC = (0x9a748 << 1);
static const u32 SPU_IRQ_LOC = (0x3000 << 1);


    //sceSdSetTransIntrHandler(0, transHandler, &loc);
static int transHandler(int channel, void *data) {
    printf("transhandler hit at %08x\n", *(int *)data);

    return 0;
}

static const u32 SPU_DST_ADDR = (0x5450 << 1);
static u8 buffer[0x40] = {};
static int loc = SPU_DST_ADDR;

static int irqHandler(int core, void *data) {
    printf("core: %d, irqhandler hit at %08x\n", core, *(int *)data);

    return 0;
}

void dmatest() {
    sceSdSetSpu2IntrHandler(irqHandler, &loc);
    sceSdSetAddr(SD_ADDR_IRQA, SPU_IRQ_LOC);
    sceSdSetCoreAttr(SD_CORE_IRQ_ENABLE, 1);

    sceSdSetAddr(SD_ADDR_TSA, SPU_DST_ADDR);

    for (int i = 0; i < 0x50; i++) {
        printf("Transfer with to %08x IRQA = %08x\n", SPU_DST_ADDR, SPU_DST_ADDR + i);
        sceSdSetAddr(SD_ADDR_IRQA, SPU_DST_ADDR + i);
        sceSdSetCoreAttr(SD_CORE_IRQ_ENABLE, 1);

        sceSdVoiceTrans(0, SD_TRANS_WRITE, buffer, (u32 *)SPU_DST_ADDR, 0x40);
        sceSdVoiceTransStatus(0, SPU_WAIT_FOR_TRANSFER);

        loc++;
    }
}
