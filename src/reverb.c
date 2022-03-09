#include "sputter.h"

static int channel = 0;
static int voice = 1;

static const u32 SPU_DST_ADDR = (0x2800 << 1);
static const int filesize = 461680;

static void initRegs() {
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLR, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_PITCH, 0x1000);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR1, SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0, 0xf, 0xf));
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR2, SD_SET_ADSR2(SD_ADSR_SR_EXPd, 127, SD_ADSR_RR_LINEARd, 0x10));

    sceSdSetParam(0 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(0 | SD_PARAM_MVOLR, 0x3fff);
    sceSdSetParam(0 | SD_PARAM_EVOLL, 0x7fff);
    sceSdSetParam(0 | SD_PARAM_EVOLR, 0x7fff);
    sceSdSetParam(1 | SD_PARAM_AVOLL, 0x7fff);
    sceSdSetParam(1 | SD_PARAM_AVOLR, 0x7fff);
    sceSdSetParam(1 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLR, 0x3fff);

    sceSdSetSwitch(channel | SD_SWITCH_VMIXL, (1 << (voice)));
    sceSdSetSwitch(channel | SD_SWITCH_VMIXR, (1 << (voice)));
    //sceSdSetSwitch(channel | SD_SWITCH_VMIXL, 0);
    //sceSdSetSwitch(channel | SD_SWITCH_VMIXR, 0);
    sceSdSetSwitch(channel | SD_SWITCH_VMIXEL, (1 << (voice)));
    sceSdSetSwitch(channel | SD_SWITCH_VMIXER, (1 << (voice)));
    sceSdSetParam(channel | SD_PARAM_MMIX, 0xffff);

}

unsigned int play(void *common) {
    sceSdSetSwitch(channel | SD_SWITCH_KON, (1 << voice));

    iop_sys_clock_t time = {};
    USec2SysClock(2000000, &time);

    // Return cycles to wait until next call
    return time.lo;
}

void reverbtest() {
    initRegs();

    sceSdEffectAttr attr = {};
    attr.core = 0;
    attr.mode = 5 | 0x100; // hall and clear wa with chan 0
    attr.depth_L = 0x7fff;
    attr.depth_R = 0x7fff;

    sceSdSetEffectAttr(0, &attr);
    sceSdSetCoreAttr(channel | SD_CORE_EFFECT_ENABLE, 1);

    sceSdSetAddr(SD_VOICE(channel, voice) | SD_VADDR_SSA, SPU_DST_ADDR);
    loadSound("host:click.adp", 0, (u32 *)SPU_DST_ADDR);

    iop_sys_clock_t time = {};

    // 2 seconds
    USec2SysClock(2000000, &time);
    sceSdSetSwitch(channel | SD_SWITCH_KON, (1 << voice));

    SetAlarm(&time, &play, NULL);
}
