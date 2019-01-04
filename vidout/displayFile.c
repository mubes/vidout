/*
 *
 * [ In this file, the graphics drawing routines ( DF_plotG, DF_line,     ]
 * [ DF_rect, DF_roundRect, DF_fillRect, DF_fillRoundRect, DF_fillCircle  ]
 * [ derrive from work that was originally (c) Adafruit, distributed      ]
 * [ under BSD. Other materials in this file are new and (c) Dave Marples ]
 *
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2012 Adafruit Industries.  All rights reserved.
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
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "displayFile.h"

/* ========================================================================== */

struct displayFile *DF_create(uint8_t yres, uint8_t xres, void *s, char c)

{
    struct displayFile *d = s;

    d->yres = yres;
    d->xres = xres;
    d->s    = s + sizeof(struct displayFile);

    DF_setScr(d, c);

    return d;
}

/* ========================================================================== */

int32_t DF_putChar(struct displayFile *d, uint8_t x, uint8_t y, char c)

{
    if ((x >= d->xres) || (y >= d->yres)) { return 0; }
    d->s[y * d->xres + x] = c;
    return 1;
}

/* ========================================================================== */

bool DF_gotoXY(struct displayFile *d, uint8_t x, uint8_t y)

{
    if ((x >= d->xres) || (y >= d->yres)) { return false; }
    d->xp = x;
    d->yp = y;
    return true;
}

/* ========================================================================== */

int32_t DF_incY(struct displayFile *d)

{
    if (d->yp >= d->yres) { return -1; }

    d->yp++;
    d->xp = 0;

    return d->yp;
}

/* ========================================================================== */

int32_t DF_incX(struct displayFile *d)

{
    d->xp++;
    if (d->xp >= d->xres) {
        if (DF_incY(d) < 0) { return -1; }
    }

    return d->xp;
}

/* ========================================================================== */

int32_t DF_writeString(struct displayFile *d, char *s)

{
    char *sw = s;
    while (*sw) {
        if ((d->xp >= d->xres) || (d->yp >= d->yres)) { break; }
        if (*sw == '\n') {
            DF_incY(d);
            sw++;
        } else {
            d->s[d->yp * d->xres + d->xp] = *sw++;
            DF_incX(d);
        }
    }

    return sw - s;
}

/* ========================================================================== */

int32_t DF_setChar(struct displayFile *d, uint32_t count, char c)

{
    uint32_t itCount = count;
    uint32_t xp      = d->xp;
    uint32_t yp      = d->yp;

    while ((itCount--) && (yp < d->yres)) {
        d->s[yp * d->xres + xp++] = c;
        DF_incX(d);
    }

    return (count - itCount);
}

/* ========================================================================== */

int32_t DF_setToEol(struct displayFile *d, char c)

{
    int32_t itCount = d->xres - d->xp;
    int32_t ret     = itCount;

    char *f = &(d->s[d->yp * d->xres + d->xp]);

    while (itCount--)
        *f++ = c;

    return ret;
}

/* ========================================================================== */

bool DF_setScr(struct displayFile *d, char c)

{
    memset(d->s, c, d->xres * d->yres);
    d->xp = d->yp = 0;

    return true;
}

/* ========================================================================== */

int32_t DF_getXpos(struct displayFile *d) { return d->xp; }

/* ========================================================================== */

int32_t DF_getYpos(struct displayFile *d) { return d->yp; }

/* ========================================================================== */

int32_t DF_getXres(struct displayFile *d) { return d->xres; }

/* ========================================================================== */

int32_t DF_getYres(struct displayFile *d) { return d->yres; }

/* ========================================================================== */

char *DF_getLine(struct displayFile *d, uint8_t yp)

{
    if (yp >= d->yres) { return NULL; }

    return &(d->s[yp * d->xres]);
}

/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* Graphic element related routines                                           */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */

int32_t DF_appendG(struct displayFile *d, uint32_t yres, uint32_t xres, void *s)

{
    /* Make sure we've got an integer number of words of width */
    d->gxlenW = xres >> 5;
    d->gylen  = yres;
    d->g      = s;

    return 0;
}

/* ========================================================================== */

int32_t DF_setGstart(struct displayFile *d, uint32_t x, uint32_t y)

{
    d->gxstartW = x >> 5;
    d->gystart  = y;

    return 0;
}

/* ========================================================================== */

uint32_t *DF_getG(struct displayFile *d, uint32_t yp)

{
    if ((NULL == d->g) || (yp < d->gystart) || (yp >= (d->gystart + d->gylen))) return NULL;

    return &d->g[(yp - d->gystart) * d->gxlenW];
}

/* ========================================================================== */

uint32_t DF_getGXstartW(struct displayFile *d) { return d->gxstartW; }

/* ========================================================================== */

uint32_t DF_getGXlenW(struct displayFile *d) { return d->gxlenW; }

/* ========================================================================== */

uint32_t DF_getGXlen(struct displayFile *d) { return 32 * DF_getGXlenW(d); }

/* ========================================================================== */

uint32_t DF_getGYlen(struct displayFile *d) { return d->gylen; }

/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */
/* Graphic plotting routines                                                  */
/* ========================================================================== */
/* ========================================================================== */
/* ========================================================================== */

static void _circleHelper(struct displayFile *d, int32_t x0, int32_t y0, int32_t r, uint8_t cornername, bool fg)

{
    int32_t f, ddF_x, ddF_y, x, y;

    f     = 1 - r;
    ddF_x = 1;
    ddF_y = -2 * r;
    x     = 0;
    y     = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        if (cornername & 0x4) {
            DF_plotG(d, x0 + x, y0 + y, fg);
            DF_plotG(d, x0 + y, y0 + x, fg);
        }
        if (cornername & 0x2) {
            DF_plotG(d, x0 + x, y0 - y, fg);
            DF_plotG(d, x0 + y, y0 - x, fg);
        }
        if (cornername & 0x8) {
            DF_plotG(d, x0 - y, y0 + x, fg);
            DF_plotG(d, x0 - x, y0 + y, fg);
        }
        if (cornername & 0x1) {
            DF_plotG(d, x0 - y, y0 - x, fg);
            DF_plotG(d, x0 - x, y0 - y, fg);
        }
    }
}

/* ========================================================================== */

static void _fillCircleHelper(struct displayFile *d, int32_t x0, int32_t y0, int32_t r, uint8_t cornername, int32_t delta, bool fg)

{
    int32_t f, ddF_x, ddF_y, x, y;

    f     = 1 - r;
    ddF_x = 1;
    ddF_y = -2 * r;
    x     = 0;
    y     = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (cornername & 0x1) {
            DF_line(d, x0 + x, y0 - y, x0 + x, y0 - y + 2 * y + 1 + delta, fg);
            DF_line(d, x0 + y, y0 - x, x0 + y, y0 - x + 2 * x + 1 + delta, fg);
        }
        if (cornername & 0x2) {
            DF_line(d, x0 - x, y0 - y, x0 - x, y0 - y + 2 * y + 1 + delta, fg);
            DF_line(d, x0 - y, y0 - x, x0 - y, y0 - x + 2 * x + 1 + delta, fg);
        }
    }
}

/* ========================================================================== */

uint32_t DF_plotG(struct displayFile *d, uint32_t x, uint32_t y, bool isSet)

{
    if ((!d->g) || (x >= (d->gxlenW << 5)) || (y >= d->gylen)) { return -1; }

    uint8_t *w = ((uint8_t *)d->g) + (y * (d->gxlenW) << 2) + (x >> 3);
    if (isSet) {
        *w |= (0x80 >> (x % 8));
    } else {
        *w &= ~(0x80 >> (x % 8));
    }

    return 0;
}

/* ========================================================================== */

void DF_line(struct displayFile *d, int32_t h1, int32_t v1, int32_t h2, int32_t v2, bool fg)

{
    int32_t dh, dv, err, e2, sh, sv;

    dh = ((h2 < h1) ? (h1 - h2) : (h2 - h1));
    dv = ((v2 < v1) ? (v1 - v2) : (v2 - v1));

    sh = (h1 < h2) ? 1 : -1;
    sv = (v1 < v2) ? 1 : -1;

    err = dh - dv;

    do {
        DF_plotG(d, h1, v1, fg);
        e2 = 2 * err;
        if (e2 > -dv) {
            err -= dv;
            h1 += sh;
        }
        if (e2 < dh) {
            err += dh;
            v1 += sv;
        }
    } while ((h1 != h2) || (v1 != v2));
}

/* ========================================================================== */

void DF_rect(struct displayFile *d, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t squareEdges, bool fg)

{
    DF_roundRect(d, x, y, w, h, TOPLEFT | TOPRIGHT | BOTLEFT | BOTRIGHT | squareEdges, 1, fg);
}

/* ========================================================================== */

void DF_roundRect(struct displayFile *d, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t r, uint8_t squareEdges, bool fg)

{
    if (!(squareEdges & TOPLINE))
        DF_line(d, x + (squareEdges & TOPLEFT ? 0 : r), y, x + w - (squareEdges & TOPRIGHT ? 0 : r), y, fg);
    if (!(squareEdges & BOTLINE))
        DF_line(d, x + (squareEdges & BOTLEFT ? 0 : r), y + h - 1, x + w - (squareEdges & BOTRIGHT ? 0 : r), y + h - 1, fg);
    if (!(squareEdges & LEFTLINE))
        DF_line(d, x, y + (squareEdges & TOPLEFT ? 0 : r), x, y + h - (squareEdges & BOTLEFT ? 0 : r), fg);
    if (!(squareEdges & RIGHTLINE))
        DF_line(d, x + w - 1, y + (squareEdges & TOPRIGHT ? 0 : r), x + w - 1, y + h - (squareEdges & BOTRIGHT ? 0 : r), fg);

    // draw four corners
    if (!(squareEdges & TOPLEFT)) _circleHelper(d, x + r, y + r, r, 1, fg);
    if (!(squareEdges & TOPRIGHT)) _circleHelper(d, x + w - r - 1, y + r, r, 2, fg);
    if (!(squareEdges & BOTRIGHT)) _circleHelper(d, x + w - r - 1, y + h - r - 1, r, 4, fg);
    if (!(squareEdges & BOTLEFT)) _circleHelper(d, x + r, y + h - r - 1, r, 8, fg);
}

/* ========================================================================== */

void DF_fillRect(struct displayFile *d, uint32_t x, uint32_t y, uint32_t w, uint32_t h, bool fg)

{
    while (h--) {
        DF_line(d, x, y, x + w, y, fg);
        y++;
    }
}

/* ========================================================================== */

void DF_fillRoundRect(struct displayFile *d, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t r, uint8_t squareEdges, bool fg)

{
    DF_fillRect(d, x + r, y, w - 2 * r, h, fg);

    if (squareEdges & TOPLEFT) DF_fillRect(d, x, y, r, r, fg);
    if (squareEdges & TOPRIGHT) DF_fillRect(d, x + w - r, y, r, r, fg);
    if (squareEdges & BOTLEFT) DF_fillRect(d, x, y + h - r, r, r, fg);
    if (squareEdges & BOTRIGHT) DF_fillRect(d, x + w - r, y + h - r, r, r, fg);

    /* draw four corners */
    _fillCircleHelper(d, x + w - r - 1, y + r, r, 1, h - 2 * r - 1, fg);
    _fillCircleHelper(d, x + r, y + r, r, 2, h - 2 * r - 1, fg);
}

/* ========================================================================== */

void DF_fillCircle(struct displayFile *d, uint32_t x0, uint32_t y0, uint32_t r, bool fg)

{
    DF_line(d, x0, y0 - r, x0, (y0 - r) + 2 * r + 1, fg);
    _fillCircleHelper(d, x0, y0, r, 3, 0, fg);
}

/* ========================================================================== */

void DF_circle(struct displayFile *d, uint32_t x0, uint32_t y0, uint32_t r, bool fg)

{
    _circleHelper(d, x0, y0, r, TOPLEFT | TOPRIGHT | BOTLEFT | BOTRIGHT, fg);
}

/* ========================================================================== */

void DF_drawTriangle(struct displayFile *d, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, bool fg)

{
    DF_line(d, x0, y0, x1, y1, fg);
    DF_line(d, x1, y1, x2, y2, fg);
    DF_line(d, x2, y2, x0, y0, fg);
}

/* ========================================================================== */

void DF_fillTriangle(struct displayFile *d, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, bool fg)

{
    int z;
    int dx1, dx2, dx3;     // Interpolation deltas
    int sx1, sx2, sy;      // Scanline co-ordinates

    if (y0 > y1) {
        z  = y0;
        y0 = y1;
        y1 = z;
        z  = x0;
        x0 = x1;
        x1 = z;
    }
    if (y1 > y2) {
        z  = y2;
        y2 = y1;
        y1 = z;
        z  = x2;
        x2 = x1;
        x1 = z;
    }

    if (y0 > y1) {
        z  = y1;
        y1 = y0;
        y0 = z;
        z  = x0;
        x0 = x1;
        x1 = z;
    }

    sx2 = (int)x0 * (int)100000;     // Use fixed point math for x axis values
    sx1 = sx2;
    sy  = y0;
    // Calculate interpolation deltas
    if (y1 - y0 > 0)
        dx1 = ((x1 - x0) * 100000) / (y1 - y0);
    else
        dx1 = 0;
    if (y2 - y0 > 0)
        dx2 = ((x2 - x0) * 100000) / (y2 - y0);
    else
        dx2 = 0;
    if (y2 - y1 > 0)
        dx3 = ((x2 - x1) * 100000) / (y2 - y1);
    else
        dx3 = 0;

    // Render scanlines
    if (dx1 > dx2) {
        for (; sy <= y1; sy++, sx1 += dx2, sx2 += dx1) {
            DF_line(d, sx1 / 100000, sy, sx1 / 100000 + (sx2 - sx1) / 100000, sy, fg);
        }
        sx2 = x1 * 100000;
        sy  = y1;

        for (; sy < y2; sy++, sx1 += dx2, sx2 += dx3) {
            DF_line(d, sx1 / 100000, sy, sx1 / 100000 + (sx2 - sx1) / 100000, sy, fg);
        }
    } else {
        for (; sy <= y1; sy++, sx1 += dx1, sx2 += dx2) {
            DF_line(d, sx1 / 100000, sy, sx1 / 100000 + (sx2 - sx1) / 100000, sy, fg);
        }
        sx1 = x1 * 100000;
        sy  = y1;

        for (; sy < y2; sy++, sx1 += dx3, sx2 += dx2) {
            DF_line(d, sx1 / 100000, sy, sx1 / 100000 + (sx2 - sx1) / 100000, sy, fg);
        }
    }
}

/* ========================================================================== */

void DF_clearG(struct displayFile *d, bool fg)

{
    uint32_t *p = d->g;

    if (!d->g) { return; }

    /* Flush the memory to the default value */
    for (uint32_t t = 0; t < d->gxlenW * d->gylen; t++) {
        *p++ = fg ? 0xFFFFFFFF : 0;
    }
}

/* ========================================================================== */

int32_t DF_gotoXYG(struct displayFile *d, uint32_t x, uint32_t y)

{
    if ((x < (d->gxlenW << 5)) && (y < d->gylen)) {
        d->curX = x;
        d->curY = y;
        return 0;
    } else {
        return -1;
    }
}

/* ========================================================================== */

int32_t DF_lineTo(struct displayFile *d, int32_t h2, int32_t v2, bool fg)

{
    if ((h2 < (d->gxlenW << 5)) && (v2 < d->gylen)) { DF_line(d, d->curX, d->curY, h2, v2, fg); }

    return DF_gotoXYG(d, h2, v2);
}

/* ========================================================================== */
