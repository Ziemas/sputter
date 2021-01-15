#include "sputter.h"

//#define TESTFILE "host0:sine.adp"
//const int size = 5888;

#define TESTFILE "host:never.adp"
const int size = 461680;

static int channel = 0;
static int voice = 1;

#define SPU_DST_ADDR (0x2800 << 1)

void playInitRegs() {
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLR, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_PITCH, 0x1000);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR1, SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0, 0xf, 0xf));
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR2, SD_SET_ADSR2(SD_ADSR_SR_EXPd, 127, SD_ADSR_RR_EXPd, 0));

    sceSdSetParam(0 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(0 | SD_PARAM_MVOLR, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_AVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_AVOLR, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLR, 0x3fff);
}

void playSound() {
    playInitRegs();

    u32 fd = open(TESTFILE, O_RDONLY);
    if (fd < 0) {
        printf("Opening testfile failed\n");
        return;
    }
    // skip header data
    lseek(fd, 16, SEEK_SET);

    // No getstat on pcsx2 :(
    //io_stat_t stat = {};
    //getstat(TESTFILE, &stat);

    //printf("file %s, size %d\n", TESTFILE, stat.size);

    u8 *buffer = AllocSysMemory(ALLOC_FIRST, size, NULL);
    if (buffer == NULL) {
        printf("Alloc failed\n");
        return;
    }

    read(fd, buffer, size);

    int trans = sceSdVoiceTrans(channel, SD_TRANS_WRITE | SD_TRANS_MODE_DMA, buffer, (u32 *)SPU_DST_ADDR, size);
    if (trans < 0) {
        printf("Bad transfer\n");
        return;
    }

    sceSdVoiceTransStatus(channel, SPU_WAIT_FOR_TRANSFER);

    sceSdSetAddr(SD_VOICE(channel, voice), SPU_DST_ADDR);
    sceSdSetSwitch(channel | SD_SWITCH_KON, (1 << voice));

    while (1)
        ;
}
