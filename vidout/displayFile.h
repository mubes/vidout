#ifndef DISPLAYFILE_H_
#define DISPLAYFILE_H_

#include <stdint.h>
#include <stdbool.h>

struct displayFile

{
  /* Text related elements */
  
  uint32_t yres;       /* Number of lines of characters */
  uint32_t xres;       /* Number of columns of characters */
  uint32_t xp;         /* Current cursor X position */
  uint32_t yp;         /* Current cursor Y position */   
  char *s;             /* Character storage */

  /* Graphic related elements */
  
  uint32_t gxstartW;   /* Start position of graphic frame in X (in terms of 32 bit words) */
  uint32_t gystart;    /* Start position of graphic frame in Y on main screen */
  uint32_t gxlenW;     /* Length in X (in terms of 32 bit words) */
  uint32_t gylen;      /* Length in Y */
  uint32_t curX;       /* Current X position (in pixels within the window */
  uint32_t curY;       /* Current Y position (in pixels within the window */
  uint32_t *g;         /* Graphic storage (or NULL for no graphic window) */
};

/* Utility routines for calculating storage to reserve for specified size windows */
#define DF_SIZE(y,x) (sizeof(struct displayFile)+y*x)
#define DF_GSIZE(y,x)  ((y)*(x)/8)

/* ============================================================================================ */

struct displayFile *DF_create(uint8_t yres, uint8_t xres, void *s, char c);

/* Text surface routines */
/* ===================== */

/* Information routines */
int32_t DF_getXres(struct displayFile *d);
int32_t DF_getYres(struct displayFile *d);
int32_t DF_getXpos(struct displayFile *d);
int32_t DF_getYpos(struct displayFile *d);

/* Position setting */
bool DF_gotoXY(struct displayFile *d, uint8_t x, uint8_t y);
int32_t DF_incY(struct displayFile *d);
int32_t DF_incX(struct displayFile *d);

/* Writing */
int32_t DF_setChar(struct displayFile *d, uint32_t count, char c);
int32_t DF_setToEol(struct displayFile *d, char c);
bool DF_setScr(struct displayFile *d, char c);
int32_t DF_putChar(struct displayFile *d, uint8_t x, uint8_t y, char c);
int32_t DF_writeString(struct displayFile *d, char *s);

/* Get text line at specified index */
char *DF_getLine(struct displayFile *d, uint8_t yp);

/* Graphic surface routines */
/* ======================== */

/* Setting routines */
int32_t DF_appendG( struct displayFile *d, uint32_t yres, uint32_t xres, void *s);
int32_t DF_setGstart( struct displayFile *d, uint32_t x, uint32_t y );

/* Information routines */
uint32_t *DF_getG(struct displayFile *d, uint32_t yp);
uint32_t DF_getGXstartW(struct displayFile *d);
uint32_t DF_getGXlenW(struct displayFile *d);
uint32_t DF_getGXlen(struct displayFile *d);
uint32_t DF_getGYlen(struct displayFile *d);

/* Drawing routines for graphic surface */

#define TOPLEFT     1
#define TOPRIGHT    2
#define BOTLEFT     4
#define BOTRIGHT    8
#define TOPLINE     16
#define BOTLINE     32
#define LEFTLINE    64
#define RIGHTLINE   128

int32_t DF_gotoXYG(struct displayFile *d, uint32_t x, uint32_t y);
int32_t DF_lineTo(struct displayFile *d, int32_t h2, int32_t v2, bool fg);

uint32_t DF_plotG(struct displayFile *d, uint32_t x, uint32_t y, bool isSet);
void DF_line(struct displayFile *d, int32_t h1, int32_t v1, int32_t h2, int32_t v2, bool fg);
void DF_rect(struct displayFile *d, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t squareEdges, bool fg);
void DF_roundRect(struct displayFile *d, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t r,
		  uint8_t squareEdges, bool fg);
void DF_fillRect(struct displayFile *d, uint32_t x, uint32_t y, uint32_t w, uint32_t h, bool fg);
void DF_fillRoundRect(struct displayFile *d, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t r,
		      uint8_t squareEdges, bool fg);
void DF_circle(struct displayFile *d, uint32_t x0, uint32_t y0, uint32_t r, bool fg);
void DF_fillCircle(struct displayFile *d, uint32_t x0, uint32_t y0, uint32_t r, bool fg);
void DF_drawTriangle(struct displayFile *d, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t x2,
		     uint32_t y2, bool fg);
void DF_fillTriangle(struct displayFile *d, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t x2,
		     uint32_t y2, bool fg);
void DF_clearG( struct displayFile *d, bool fg);

/* ============================================================================================ */
#endif
