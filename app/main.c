/*
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2019 Dave Marples. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * VGA VIDEO Output on a STM32F103C8
 * =================================
 *
 * Example main file.
 */

#include <stdlib.h>
#include <sys/types.h>
#include "vidout.h"

// ============================================================================================
// ============================================================================================
// ============================================================================================
// System Setup
// ============================================================================================
// ============================================================================================
// ============================================================================================

#define GX 6 * 32
#define GY 80

uint8_t gmem[DF_GSIZE(GY, GX)];
int     main(void)

{
    /* Initialise a screen (This starts output) */
    struct displayFile *d = vidInit();

    /* Apprend a graphic buffer of specified size */
    DF_appendG(d, GY, GX, gmem);

    /* Locate the graphic buffer onscreen and clear it */
    DF_setGstart(d, 32, 20);
    DF_clearG(d, false);

    /* Fill the graphic buffer with some junk by way of example */
    DF_line(d, 0, 0, GX, GY, true);
    DF_line(d, GX, 0, 0, GY, true);
    DF_circle(d, GX / 2, GY / 2, 20, true);

    for (uint32_t t = 16; t <= GX - 16; t += 16) {
        DF_line(d, t, 0, t, 5, true);
        DF_line(d, t, GY, t, GY - 5, true);
    }

    for (uint32_t t = 8; t <= GY - 8; t += 8) {
        DF_line(d, 0, t, 5, t, true);
        DF_line(d, GX, t, GX - 5, t, true);
    }

    for (int32_t t = -GX; t < GX; t += 4) {
        DF_lineTo(d, GX / 2 + t, random() % (GY - 10), true);
    }

    /* Fill the text window with some junk by way of example */
    for (uint32_t e = 0; e < XSIZE; e++) {
        DF_putChar(d, e, 0, '0' + e % 10);
    }
    for (uint32_t e = 1; e < YSIZE; e++) {
        DF_putChar(d, 0, e, e < 10 ? ' ' : '0' + e / 10);
        DF_putChar(d, 1, e, '0' + e % 10);
    }
    DF_gotoXY(d, XSIZE / 2 - 5, 4);

    DF_writeString(d, "Testing");

    for (uint32_t t = 0; t < 256; t++)
        DF_putChar(d, 4 + t % (XSIZE - 5), 10 + t / (XSIZE - 5), t);

    DF_putChar(d, XSIZE - 1, YSIZE - 1, 'X');

    /* Now a bit of animation, for fun */
    uint32_t z  = 0;
    int      zd = 1;
    while (1) {
        z += zd;
        if ((z == 300) || (z == 0)) zd = -zd;

        for (uint32_t t = 0; t < 40000; t++) {
            __asm__("NOP;");
        }
        DF_setGstart(d, 110, z);
        DF_gotoXY(d, 2, 4);
        DF_setToEol(d, ' ');
        DF_gotoXY(d, 2 + (z / 8) % (XSIZE - 10), 4);
        DF_writeString(d, " Testing");
    }
}
// ============================================================================================
