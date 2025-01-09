#include "sputter.h"

static u32 transpos[5];
static int buf[64];
static u8 autodmabuf[4096];

void fastwait() {
    int result;
    do
        result = sceSdVoiceTransStatus(1, 0);
    while (!result);
}

u8 seen[0x1000] = {};

void sync_buf() {
    memset(buf, 0, sizeof(buf));
    memset(autodmabuf, 0, sizeof(autodmabuf));

    sceSdSetCoreAttr(9, 30);
    sceSdSetSwitch(0x1401, 0x800000);
    sceSdSetParam(0x2F, 255);
    sceSdSetParam(0x32F, 15);
    sceSdSetParam(0x42F, 5);
    sceSdSetAddr(8303, 20480);
    sceSdSetSwitch(5377, 0x800000);

    int lastVal = 0;
    int pos = 0;
    int timeout = 0;
    u32 i = 0;

    do {
        sceSdVoiceTrans(1, 1, (u8 *)&buf, (u32 *)0x3000, 64);
        //sceSdVoiceTransStatus(1, SPU_WAIT_FOR_TRANSFER);
        fastwait();

        u32 status = sceSdBlockTransStatus(0, 0) & 0xFFFFFF;
        transpos[pos] = status - (u32)&autodmabuf;

        printf("status %08lx\n", transpos[pos]);
        seen[i] = transpos[pos];
        i++;

        if (buf[0] != lastVal) {
            //printf("saw new val\n");
            sceSdBlockTrans(0, SD_TRANS_STOP, 0, 0);
            sceSdBlockTrans(0, SD_TRANS_LOOP | SD_TRANS_WRITE_FROM, autodmabuf, 4096, &autodmabuf[2048]);
            lastVal = buf[0];
            if (transpos[pos] != 2048) {
                printf("Found pos %d at %08lx\n", pos, transpos[pos]);
                pos++;
            }
            timeout = 0;
        }

        timeout++;
        if (timeout > 10000) {
            timeout = 0;
            printf("Timeout, %08lx\n", transpos[pos]);
            transpos[pos] = 3072;
            pos++;
        }

    } while (pos < 5);
}

void bufdetect() {
    sceSdSetCoreAttr(0 | SD_CORE_SPDIF_MODE, 0x801); // SPDIF DVD | SPDIF BITSREAM
    memset(autodmabuf, 0x55, sizeof(autodmabuf));

    sceSdBlockTrans(0, SD_TRANS_LOOP, autodmabuf, 0x1000u);


    u32 status = sceSdBlockTransStatus(0, 0) & 0xFFFFFF;
    while (1) {
        u32 status2 = sceSdBlockTransStatus(0, 0) & 0xFFFFFF;
        if (status == status2) {
            continue;
        }
        status = status2;
        printf("dma at %x\n", status2);
    }


    //int timeout = 10000;
    //u16 sample = 0;
    //while (timeout--) {
    //    sceSdVoiceTrans(1, 1, (u8 *)&buf, (u32 *)0x4400, 64);
    //    fastwait();

    //    sample = buf[0];

    //    if (sample == 0x5555) {
    //        for (int i = 0; i < 10; i++)
    //        {
    //            printf("%04x", buf[i]);
    //        }
    //        printf("\n");
    //        break;
    //    }
    //}

    //printf("sample %04x\n", sample);

    //return;

    int it = 0;
    int test = 0;
    do {
        sceSdSetCoreAttr(0 | SD_CORE_SPDIF_MODE, 0x801); // SPDIF DVD | SPDIF BITSREAM

        sceSdBlockTrans(0, SD_TRANS_LOOP, autodmabuf, 0x1000u);
        sync_buf();

        for (int i = 1; i < 5; i++) {
            u32 p = transpos[i];
            if (p > 0x700 || p < 0x200)
                test++;

            printf("transpos %08lx\n", transpos[i]);
        }

        for (int i = 0; i < 0x20; i++) {
            printf("%04x\n", seen[i]);
        }

        if (test)
            printf("TEST FAILED\n");
        else {
            printf("TEST PASSED\n");
            break;
        }

        sceSdBlockTrans(0, SD_TRANS_STOP, 0, 0);

        test = 0;
        it++;
    } while (it < 50);

    printf("exit\n");
    //printf("passed after %d iterations", it);
}
