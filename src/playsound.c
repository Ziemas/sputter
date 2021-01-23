#include "sputter.h"

static const char* TESTFILE = "host0:sine.adp";
static const int filesize = 5888;

static const char* TESTFILE2 = "host0:never.adp";
static const int filesize2 = 461680;

static int channel = 0;
static int voice = 1;

static const u32 SPU_DST_ADDR = (0x2800 << 1);
static const u32 SPU_DST_ADDR2 = (0x4800 << 1);

static void initRegs() {
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLR, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_PITCH, 0x4);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR1, SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0, 0xf, 0xf));
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR2, SD_SET_ADSR2(SD_ADSR_SR_EXPd, 127, SD_ADSR_RR_EXPd, 0));

    sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_VOLR, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_VOLL, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_PITCH, 0x1000);
    sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_ADSR1, SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0, 0x7f, 0xf));
    sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_ADSR2, SD_SET_ADSR2(SD_ADSR_SR_EXPi, 0x7f, SD_ADSR_RR_EXPd, 0));

    sceSdSetParam(0 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(0 | SD_PARAM_MVOLR, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_AVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_AVOLR, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLR, 0x3fff);

    sceSdSetSwitch(channel | SD_SWITCH_VMIXL, (1 << (voice + 1)));
    sceSdSetSwitch(channel | SD_SWITCH_VMIXR, (1 << (voice + 1)));
    sceSdSetSwitch(channel | SD_SWITCH_PMON, (1 << (voice + 1)));
    sceSdSetSwitch(channel | SD_SWITCH_NON, 0);
}

u32 loadSound(const char *filename, u32 size, u32 *spudst) {
    u8 *buffer = AllocSysMemory(ALLOC_FIRST, size, NULL);
    if (buffer == NULL) {
        printf("Alloc failed\n");
        return 1;
    }

    u32 fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("Opening testfile failed\n");
        return 1;
    }

    // skip header data
    lseek(fd, 16, SEEK_SET);
    read(fd, buffer, size);
    close(fd);

    int trans = sceSdVoiceTrans(channel, SD_TRANS_WRITE | SD_TRANS_MODE_DMA, buffer, spudst, size);
    if (trans < 0) {
        printf("Bad transfer\n");
        return 1;
    }

    int err = sceSdVoiceTransStatus(channel, SPU_WAIT_FOR_TRANSFER);
    if (err < 0) {
        printf("failed to wait for transfer %d", err);

    }

    FreeSysMemory(buffer);

    printf("Loaded sound %s size %lu to %08lx\n", filename, size, ((u32)spudst) >> 1);

    return 0;
}

void playSound() {
    initRegs();

    sceSdSetAddr(SD_VOICE(channel, voice) | SD_VADDR_SSA, SPU_DST_ADDR);
    sceSdSetAddr(SD_VOICE(channel, voice + 1) | SD_VADDR_SSA, SPU_DST_ADDR2);


    loadSound(TESTFILE, filesize, (u32 *)SPU_DST_ADDR);
    loadSound(TESTFILE2, filesize2, (u32 *)SPU_DST_ADDR2);

    printf("starting voices\n");

    u32 kon = (1 << voice) | (1 << (voice + 1));

    sceSdSetSwitch(channel | SD_SWITCH_KON, kon);
    //sceSdSetSwitch(channel | SD_SWITCH_KON, 1 << voice);
}
