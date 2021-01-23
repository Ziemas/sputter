#include "sputter.h"

static u8 noiseIdx = 0;
static int voice = 1;

unsigned int newNoise(void *common) {
    static u8 noiseValues[8] = {0x07, 0x0E, 0x15, 0x1c, 0x23, 0x2a, 0x31, 0x3f};
    u8 *idx = (u8 *)common;

    sceSdSetCoreAttr(SD_CORE_NOISE_CLK | 0, noiseValues[*idx]);
    *idx = (*idx + 1) % 8;

    iop_sys_clock_t time = {};
    USec2SysClock(2000000, &time);

    // Return cycles to wait until next call
    return time.lo;
}

void noiseTest() {
    iop_sys_clock_t time = {};

    // 2 seconds
    USec2SysClock(2000000, &time);

    sceSdSetCoreAttr(SD_CORE_NOISE_CLK | 0, 0x03);

    sceSdSetParam(SD_VOICE(0, 0) | SD_VPARAM_VOLR, 0x3fff);
    sceSdSetParam(SD_VOICE(0, 0) | SD_VPARAM_VOLL, 0x3fff);
    sceSdSetParam(SD_VOICE(0, 0) | SD_VPARAM_PITCH, 0x1000);
    sceSdSetParam(SD_VOICE(0, 0) | SD_VPARAM_ADSR1, SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0, 0xf, 0xf));
    sceSdSetParam(SD_VOICE(0, 0) | SD_VPARAM_ADSR2, SD_SET_ADSR2(SD_ADSR_SR_EXPd, 127, SD_ADSR_RR_EXPd, 0));

    sceSdSetParam(0 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(0 | SD_PARAM_MVOLR, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_AVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_AVOLR, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLR, 0x3fff);

    sceSdSetSwitch(0 | SD_SWITCH_NON, (1 << 0));

    sceSdSetSwitch(0 | SD_SWITCH_KON, (1 << 0));

    printf("hi %08lx lo %08lx\n", time.hi, time.lo);
    printf("Setting alarm\n");
    SetAlarm(&time, &newNoise, &noiseIdx);
    while (1)
        ;
}
