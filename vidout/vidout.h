#ifndef	_VIDEO_H_
#define	_VIDEO_H_
#include "stm32f10x.h"
#include "displayFile.h"

/* Easy configuration options */
/* ========================== */

//#define HIRES                          /* Define this for high definition in X */
#define BUSY_DEBUG                       /* Define this to enable a busy flag */
#define HIGHPRI_IRQ (0)                  /* This is the HSYNC interrupt and needs to be very high priority */
#define LOWPRI_IRQ  (1)                  /* This is the line preparation (SPI) interrupt and can have a */
                                         /* lower priority, but you may have to raise it if you see corruption. */

/* Internals */
/* ========= */

#ifndef HIRES
#define XSIZE      50                    /* Number of characters wide the display file is (columns) */
#define YSIZE      18                    /* Number of characters deep the display file is (rows) */
#define YSTRETCH    1                    /* How much extra to stretch in Y per pixel */
#define FRAME_YDISPLACEMENT 10           /* How many visible lines to output before starting to display */
#else
#define XSIZE     100
#define YSIZE      18
#define YSTRETCH    1
#define FRAME_YDISPLACEMENT 10
#endif

#define ROUNDUP4(x) (((x+3)/4)*4)
#define XEXTENTB   (ROUNDUP4(XSIZE))     /* What the X resolution is in bytes */

#ifdef BUSY_DEBUG
#define SETUP_BUSY GPIOB->CRH=((GPIOB->CRH)&0xFFF0FFFF)|0x30000  
#define AM_IDLE    GPIOB->BRR=(1<<12)
#define AM_BUSY    GPIOB->BSRR=(1<<12)
#else
#define SETUP_BUSY {}
#define AM_IDLE    {}
#define AM_BUSY    {}
#endif

/* ============================================================================================ */

uint32_t vidxSizeG(void);
uint32_t vidySizeG(void);
struct displayFile *vidInit(void);

/* ============================================================================================ */
#endif	/*  _VIDEO_H_ */

