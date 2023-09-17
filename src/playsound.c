#include "spu2regs.h"
#include "sputter.h"

static const char *TESTFILE = "host0:sine.adp";
static const int filesize = 5888;

static const char *TESTFILE2 = "host0:never.adp";
static const int filesize2 = 461680;

static int channel = 0;
static int voice = 22;

static int param_idx = 0;

static const u32 SPU_DST_ADDR = (0x3800 << 1);
static const u32 SPU_DST_ADDR2 = (0x4800 << 1);

static void initRegs() {
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLR, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_PITCH, 0x2);
    // sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR1, SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0x6F, 0xf, 0xf));
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR1, SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0, 0x7f, 0xf));
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR2, SD_SET_ADSR2(SD_ADSR_SR_LINEARi, 0x36, SD_ADSR_RR_LINEARd, 0x10));

    sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_VOLR, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_VOLL, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_PITCH, 0x1000);
    sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_ADSR1, SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0, 0x7f, 0xf));
    sceSdSetParam(SD_VOICE(channel, (voice + 1)) | SD_VPARAM_ADSR2, SD_SET_ADSR2(SD_ADSR_SR_EXPi, 0x7f, SD_ADSR_RR_LINEARd, 0x10));

    sceSdSetParam(0 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(0 | SD_PARAM_MVOLR, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_AVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_AVOLR, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLR, 0x3fff);

    sceSdSetSwitch(channel | SD_SWITCH_VMIXL, (1 << (voice + 1)));
    sceSdSetSwitch(channel | SD_SWITCH_VMIXR, (1 << (voice + 1)));
    // sceSdSetSwitch(channel | SD_SWITCH_VMIXL, (1 << (voice)));
    // sceSdSetSwitch(channel | SD_SWITCH_VMIXR, (1 << (voice)));
}

unsigned int switchparams(void *common) {
    static u16 params[8] = {0x0, 0x500, 0x1fff, 0x3000, 0x4000, 0x4fff, 0x6fff, 0x7fff};
    u8 *idx = (u8 *)common;

    u32 envx = *SD_VP_ENVX(0, 0);
    printf("before: %04x\n", envx);

    *SD_VP_ENVX(0, 0) = 0;

    envx = *SD_VP_ENVX(0, 0);
    printf("after: %04x\n", envx);

    *idx = (*idx + 1) % 8;

    iop_sys_clock_t time = {};
    USec2SysClock(1000000, &time);

    // Return cycles to wait until next call
    return time.lo;
}

unsigned int setnax(void *common) {

    u32 nax = sceSdGetAddr(SD_VADDR_NAX | SD_VOICE(0, 1));
    printf("before: %08x\n", nax);

    //*SD_VA_NAX(0, 1) = 0x2800;
    sceSdSetAddr(SD_VADDR_NAX | SD_VOICE(0, 1), 0x2800 << 1);

    nax = sceSdGetAddr(SD_VADDR_NAX | SD_VOICE(0, 1));
    printf("after: %08x\n", nax);

    iop_sys_clock_t time = {};
    USec2SysClock(2 * USEC_SECOND, &time);

    return time.lo;
}

void playSound() {
    iop_sys_clock_t time = {};
    USec2SysClock(USEC_SECOND, &time);

    initRegs();

    sceSdSetAddr(SD_VOICE(channel, voice) | SD_VADDR_SSA, SPU_DST_ADDR);
    sceSdSetAddr(SD_VOICE(channel, voice + 1) | SD_VADDR_SSA, SPU_DST_ADDR2);

    loadSound(TESTFILE, channel, (u32 *)SPU_DST_ADDR);
    loadSound(TESTFILE2, channel, (u32 *)SPU_DST_ADDR2);

    // sceSdSetSwitch(channel | SD_SWITCH_NON, 1 << voice);
    sceSdSetSwitch(channel | SD_SWITCH_PMON, (1 << (voice + 1)));

    printf("starting voices\n");

    u32 kon = (1 << voice) | (1 << (voice + 1));
    // u32 kon = (1 << (voice + 1));
    // kon = 0b11110;

    sceSdSetSwitch(channel | SD_SWITCH_KON, kon);

    // iop_sys_clock_t time = {};
    // USec2SysClock(2000000, &time);
    // SetAlarm(&time, &koff, NULL);

    // iop_sys_clock_t time2 = {};
    // USec2SysClock(2500000, &time2);
    // SetAlarm(&time, &koff2, NULL);

    // sceSdSetSwitch(channel | SD_SWITCH_KON, 1 << voice);

    // SetAlarm(&time, &setnax, &param_idx);
}
