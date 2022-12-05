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
 * Mapping font into output buffer
 * ===============================
 *
 * rasterLine runs _very_ frequently and is time critical. Give it the maximum
 * chance of working well by optimising as much as possible.  Note that it's placed
 * into RAM to avoid wait states for flash access.  Wait states are _not_ good.
 * To get this routine to actually appear in RAM, modify your linker script DATA
 * section to include the line "*(.ramprog .ramprog.*). This will allow the code
 * to be copied along with initialized data.
 */

#include "displayFile.h"
#include "rasterLine.h"

#ifdef MONITOR_OUTPUT
#include "itm_messages.h"
#include "orblcd_protocol.h"
#endif

#define OPTIMISED_RASTERLINE_BITS (4)

/* ============================================================================================ */

__attribute__((__section__(".ramprog"))) void rasterLine(struct displayFile *d, const struct rasterFont *f, uint32_t *w, uint32_t rl)

{
    /* Bit manipulation optimisation for simple division case that can be done with shifts
     * Warning - this routine assumes that the font is 8 bits wide, if it's not then a smarter
     * rasteriser needs to be substituted in here.
     */

    uint32_t  *w2         = 0;
    int c                 = 0;
    char *    displayLine = DF_getLine(d, rl >> OPTIMISED_RASTERLINE_BITS);
    uint32_t  index       = rl & ((1 << OPTIMISED_RASTERLINE_BITS) - 1);

    int32_t chrs = DF_getXres(d);
    uint32_t *g = DF_getG(d, rl);     /* ...any graphics element to be rasterised */
    if (g)
      {
	w2 = w+DF_getGXstartW(d);
	c = DF_getGXlenW(d);
      }

    /* This could overrun, but we make it a constraint in the definition that the buffer has to be word aligned */
    while (chrs > 0) {
        *w   = ((f->d[((displayLine[3] - f->firstChr) << OPTIMISED_RASTERLINE_BITS) + index]) << 24) |
               ((f->d[((displayLine[2] - f->firstChr) << OPTIMISED_RASTERLINE_BITS) + index]) << 16) |
               ((f->d[((displayLine[1] - f->firstChr) << OPTIMISED_RASTERLINE_BITS) + index]) << 8) |
               ((f->d[((displayLine[0] - f->firstChr) << OPTIMISED_RASTERLINE_BITS) + index]));
	if ((g) && (w>=w2) && (c))
	  {
	    /* Fold in the graphics */
	    *w |= *g++;
	    c--;
	  }

#ifdef MONITOR_OUTPUT
	/* Send this to the monitor if appropriate */
	ITM_Send32(LCD_DATA_CHANNEL,*w);
#endif

	w++;
        displayLine += 4;
        chrs -= 4;
    }
}

/* ============================================================================================ */
