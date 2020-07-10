#include "sputter.h"
#include <ioman.h>
#include <libsd.h>
#include <modload.h>
#include <stdio.h>
#include <sysmem.h>
#include <thbase.h>
#include <types.h>

IRX_ID("sputter", 1, 1);

s32 _start()
{
    if (sceSdInit(0) < 0) {
        printf("SD INIT FAILED");
        return -1;
    }

    naxTest();

    return 0;
}
