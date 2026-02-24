#include "spu2regs.h"
#include "sputter.h"

static int channel = 0;
static int voice = 1;
static int sema = 0;

static void initRegs() {
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLR, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_VOLL, 0x3fff);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_PITCH, 0x1000);
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR1, SD_SET_ADSR1(SD_ADSR_AR_EXPi, 0, 0x7f, 0xf));
    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_ADSR2, SD_SET_ADSR2(SD_ADSR_SR_EXPd, 127, SD_ADSR_RR_LINEARd, 0x10));

    sceSdSetParam(0 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(0 | SD_PARAM_MVOLR, 0x3fff);

    sceSdSetParam(0 | SD_PARAM_EVOLL, 0x7fff);
    sceSdSetParam(0 | SD_PARAM_EVOLR, 0x7fff);

    sceSdSetParam(1 | SD_PARAM_AVOLL, 0x7fff);
    sceSdSetParam(1 | SD_PARAM_AVOLR, 0x7fff);

    sceSdSetParam(1 | SD_PARAM_MVOLL, 0x3fff);
    sceSdSetParam(1 | SD_PARAM_MVOLR, 0x3fff);

    // sceSdSetSwitch(channel | SD_SWITCH_VMIXL, (1 << (voice)));
    // sceSdSetSwitch(channel | SD_SWITCH_VMIXR, 0);

    // sceSdSetSwitch(channel | SD_SWITCH_VMIXEL, 0);
    // sceSdSetSwitch(channel | SD_SWITCH_VMIXER, (1 << (voice)));

    // sceSdSetParam(channel | SD_PARAM_MMIX, 0b0000000000);
    sceSdSetParam(channel | SD_PARAM_MMIX, 0xffff);
}

static int spu2irq(int c, void *) {
    iSignalSema(sema);

    return 0;
}

void irqtest() {
    iop_sema_t sp = {};
    sp.attr = SA_THFIFO;
    sp.initial = 0;
    sp.max = 2;

    sema = CreateSema(&sp);

    initRegs();

    sceSdSetSpu2IntrHandler(spu2irq, NULL);
    sceSdSetAddr(SD_ADDR_IRQA, 0x1c000 << 1);
    sceSdSetCoreAttr(SD_CORE_IRQ_ENABLE, 1);

    sceSdSetParam(SD_VOICE(channel, voice) | SD_VPARAM_PITCH, 0);
    //sceSdSetAddr(SD_VADDR_NAX | SD_VOICE(channel, voice), 0x1c002 << 1);

    while (1) {
        int err = WaitSema(sema);
        if (err) {
            printf("sema error %d\n", err);
            return;
        }

        printf("hit irq\n");
        sceSdSetCoreAttr(SD_CORE_IRQ_ENABLE, 1);
    }
}
