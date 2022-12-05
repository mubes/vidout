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
 * Pinout;
 *   PA1 = VSYNC (Pin 14 on VGA connector)
 *   PA8 = HSYNC (Pin 13 on VGA connector)
 *   PA7 = Video (Pin 1, 2 or 3 on VGA connector for R, G or B respectively).
 *
 * Note that the spec calls for PA1/PA8 to be 5V and PA7 to be 1V max so you might want to
 * put a series resistor on PA7. The input impedence of VGA is 75R so something in the
 * region of 220R should be fine.
 *
 * To use this subsystem just make sure you've not got anything on the high priority
 * interrupt levels (by default this is configured for the highest two, but the SPI
 * interrupt can have lower priority if you've got other things you need to do).
 *
 * Just call vidInit to start video output and to get an object back that you can manipulate
 * to create output.  Both text and graphic output are supported. See the example main
 * for how to use it, and the API exposed by displayFile too.
 *
 * In general this file should be fire and forget. Once video is up and running it doesn't
 * need any further maintainence or input from you.
 */

#include "displayFile.h"
#include "rasterLine.h"
#include "vidout.h"

#ifdef MONITOR_OUTPUT
#include "itm_messages.h"
#include "orblcd_protocol.h"
#endif

#include "font-8x16basic.cinc" /* The font to use */

/* Setup materials section */
/* ======================= */

/* Setup pinning and perhiperals to be used ... if these are changed then  */
/* be careful to ensure that clocks/power are enabled to the replacements. */
#define SETUP_VSYNC GPIOA->CRL = ((GPIOA->CRL) & 0xFFFFFF0F) | 0x30 /* 50MHz, PushPull output */
#define VSYNC_HIGH GPIOA->BSRR = (1 << 1)
#define VSYNC_LOW GPIOA->BRR = (1 << 1)

#define SETUP_HSYNC GPIOA->CRH = ((GPIOA->CRH) & 0xFFFFFFF0) | 0x0B      /* 50MHz, Alternate PushPull */
#define SETUP_VOUT GPIOA->CRL = ((GPIOA->CRL) & 0x0FFFFFFF) | 0xB0000000 /* 50MHz, Alternate PushPull */

#define TIM TIM1
#define TIM_IRQHandler TIM1_CC_IRQHandler
#define SPI SPI1

#define DMA DMA1
#define DMA_CHANNEL DMA1_Channel3
#define DMA_CHANNEL_IRQn DMA1_Channel3_IRQn
#define DMA_CHANNEL_IRQHandler DMA1_Channel3_IRQHandler

/* Screen definition section */
/* ========================= */

/* What the screen looks like */
#define YEXTENT (YSIZE * FONTHEIGHT * (YSTRETCH + 1)) /* How much raster we need to display all of Y */

/* Display protocol material */
/* ========================= */

/* Various elements of the display protocol ... all timing stems from these... */
#define FRAME_START 0          /* Start of frame - VSYNC pulse */
#define FRAME_BACKPORCH 2      /* End of VSYNC, start of back porch + blanking */
#define FRAME_BACKPORCH_END 22 /* End of Backportch region */
#define FRAME_OUTPUT_START (FRAME_BACKPORCH_END + FRAME_YDISPLACEMENT) /* Active frame output start */
#define FRAME_OUTPUT_END (FRAME_OUTPUT_START + YEXTENT + 1)            /* Active frame output end */
#define FRAME_END 624                                                  /* End of frame ... start next one */

/* Clock ticks for each element of a frame (in terms of 72MHz clock pulses */
#define LINEPERIOD (2048)         /* Length of a line */
#define HORIZSYNCPULSEWIDTH (144) /* Horizontal pulse width (2uS) */
#define SYNCPLUSPORCH (280)       /* Sync + porch period, adjust if needed to centralise the image */

/* Internal Setup */
/* ============== */

/* Definition of the screen ... done here to avoid it going on the stack */
char storage[DF_SIZE(YSIZE, XSIZE)];

/* Material related to this instance */
/* ================================= */

static volatile struct videoMachine {
    const struct rasterFont *f;     /* the font in use */
    struct displayFile *     d;     /* The display file being output */
    uint8_t  lineBuff[2][XEXTENTB]; /* Line buffer containing the constructed raster for output (roundup to word) */
    uint32_t scanLine;              /* The current line being scanned on the screen */
    uint32_t stretchLine;           /* Counter for line stretching */
    int32_t  opLine;                /* Line of frame being output */
    uint32_t readLine;              /* Line currently being written/read from */
} _v = { .f = &font };

/* If you are building without the Standard Perhiperal Library (the best way) then these are */
/* undefined, so we define them here to avoid needing two separate builds.                   */
#define TIM_IT_CC2 ((uint16_t)0x0004)
#define DMA1_IT_TC3 ((uint32_t)0x00000200)
#define RCC_AHBPeriph_DMA1 ((uint32_t)0x00000001)
#define RCC_APB1Periph_PWR ((uint32_t)0x10000000)
#define RCC_APB2Periph_SPI1 ((uint32_t)0x00001000)
#define RCC_APB2Periph_TIM1 ((uint32_t)0x00000800)
#define RCC_APB2Periph_GPIOA ((uint32_t)0x00000004)
#define RCC_APB2Periph_GPIOB ((uint32_t)0x00000008)
#define SPI_I2S_DMAReq_Tx ((uint16_t)0x0002)

/* TIM_IRQHandler runs _very_ frequently and is time critical. Give it the maximum
 * chance of working well by optimising as much as possible.  Note that it's placed
 * into RAM to avoid wait states for flash access.  Wait states are _not_ good.
 * To get this routine to actually appear in RAM, modify your linker script DATA
 * section to include the line "*(.ramprog .ramprog.*). This will allow the code
 * to be copied along with initialized data.
 */

/* ============================================================================================ */

__attribute__((__section__(".ramprog"))) void TIM_IRQHandler(void)
{
    /* Called at the end of each scanline to schedule the next element of the protocol
     * Assumes that the next scanline will have been prepared for output already,
     */

    AM_BUSY;

    /* Clear the interrupt and move to the next scanline */
    TIM->SR &= ~TIM_IT_CC2;

    /* Remove TCIE and Enable */
    DMA_CHANNEL->CCR &= ~(DMA_CCR3_EN | DMA_CCR3_TCIE);

    /* Now, depending on what element of the frame we're on, do the magic to output it */
    switch (_v.scanLine++) {
        /* ------------------------------------------------------------------------ */
    case FRAME_START ... FRAME_BACKPORCH - 1:
        /* Start of frame - create sync pulse */
        VSYNC_HIGH;
        break;

        /* ------------------------------------------------------------------------ */
    case FRAME_BACKPORCH ... FRAME_OUTPUT_START - 1:
        /* Sync pulse done - top blanking */
        VSYNC_LOW;
        _v.stretchLine = _v.opLine = _v.readLine = 0;

        /* Send out a zeroed line... this is in the first lineBuff at the moment */
        DMA_CHANNEL->CMAR  = (uint32_t)_v.lineBuff[1];
        DMA_CHANNEL->CNDTR = XSIZE;
        DMA_CHANNEL->CCR |= DMA_CCR3_EN; /* Enable, No TCIE */
        break;

        /* ------------------------------------------------------------------------ */
    case FRAME_OUTPUT_START ... FRAME_OUTPUT_END:
        /* Set the previusly prepared scanLine ready to be output */
        DMA_CHANNEL->CMAR  = (uint32_t)_v.lineBuff[_v.readLine];
        DMA_CHANNEL->CNDTR = XSIZE;
        DMA->IFCR          = DMA1_IT_TC3;

        /* See if it's time for the next line to be generated */
        if (_v.stretchLine++ == YSTRETCH) {
            /* Next time we'll swap sides, so we can write into the current buffer */
            /* once it's been transmitted */
            DMA_CHANNEL->CCR |= DMA_CCR3_EN | DMA_CCR3_TCIE; /* TCIE and Enable */

            /* Read from the (hopefully) already prepared line, and reset the stretch */
            _v.readLine    = !_v.readLine;
            _v.stretchLine = 0;
        } else {
            DMA_CHANNEL->CCR |= DMA_CCR3_EN; /*  Enable */
        }
        break;

        /* ------------------------------------------------------------------------ */
    case FRAME_OUTPUT_END + 1 ... FRAME_END - 1:
        /* Send out a zeroed line... */
        DMA_CHANNEL->CMAR  = (uint32_t)_v.lineBuff[1];
        DMA_CHANNEL->CNDTR = XSIZE;
        DMA_CHANNEL->CCR |= DMA_CCR3_EN; /* Enable */
        break;

        /* ------------------------------------------------------------------------ */
    case FRAME_END:
        /* End of frame */
        _v.scanLine = 0;
        break;
    }
}
/* ============================================================================================ */

__attribute__((__section__(".ramprog"))) void DMA_CHANNEL_IRQHandler(void)

{
    /* This routine is called immediately there is room to calculate the next line for output */
    /* It is called as a low priority interrupt so it can do it's work when there's time.     */
    AM_BUSY;

    DMA->IFCR = DMA1_IT_TC3;

    if (_v.opLine >= YSIZE * FONTHEIGHT) {
        /* No more valid scan lines in this frame, so don't output more video */
        /* ...and make sure the first line is set up to go out */
        rasterLine(_v.d, _v.f, (uint32_t *)_v.lineBuff[0], 0);
        _v.readLine = 0;

#ifdef MONITOR_OUTPUT
	if (_v.opLine == YSIZE * FONTHEIGHT)
	  {
	    /* This is sent at the start of every frame in case the other end wasn't awake */
	    ITM_Send32(LCD_COMMAND_CHANNEL,ORBLCD_OPEN_SCREEN(XSIZE*8,YSIZE*16,ORBLCD_DEPTH_1));
	  }
#endif

        /* Zero out line 1 as this will be used for blanking */
        for (uint32_t t = 0; t < XSIZE; t++)
            _v.lineBuff[1][t] = 0;
    } else {
        /* Prepare next line for output */
        rasterLine(_v.d, _v.f, (uint32_t *)_v.lineBuff[!_v.readLine], _v.opLine++);
    }
}

/* ============================================================================================ */
/* ============================================================================================ */
/* ============================================================================================ */
/* Public routines                                                                              */
/* ============================================================================================ */
/* ============================================================================================ */
/* ============================================================================================ */

uint32_t vidxSizeG(void) { return YSIZE * FONTHEIGHT; }

/* ============================================================================================ */

uint32_t vidySizeG(void) { return XSIZE * FONTWIDTH; }

/* ============================================================================================ */

struct displayFile *vidInit(void)

{
    /* Switch on power/clocks to required perhiperals */
    RCC->AHBENR |= RCC_AHBPeriph_DMA1;
    RCC->APB1ENR |= RCC_APB1Periph_PWR;
    RCC->APB2ENR |= RCC_APB2Periph_SPI1 | RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB;

    SETUP_BUSY;
    SETUP_VSYNC;
    SETUP_HSYNC;
    SETUP_VOUT;

    /* Create the video handler object */
    _v.d = DF_create(YSIZE, XSIZE, storage, ' ');

    /* Setup the DMA transfer details */
    DMA_CHANNEL->CCR  = DMA_CCR1_MINC | DMA_CCR1_DIR;
    DMA_CHANNEL->CPAR = (uint32_t)&SPI->DR;

    /* Setup the SPI transfer details */
#ifndef HIRES
    SPI->CR1 = SPI_CR1_MSTR | SPI_CR1_SPE | SPI_CR1_BR_0;
#else
    SPI->CR1 = SPI_CR1_MSTR | SPI_CR1_SPE;
#endif
    SPI->CR2 |= SPI_I2S_DMAReq_Tx;

    /* Set up timings for line period and horizontal pulses */
    TIM->ARR  = LINEPERIOD;
    TIM->CCR1 = HORIZSYNCPULSEWIDTH; /* Set pulse to end */
    TIM->CCR2 = SYNCPLUSPORCH;       /* Set secondary pulse, which generates interrupt, further into line */

    TIM->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; /* PWM mode 1 (Ch1 active until triggered) */
    TIM->CCER  = TIM_CCER_CC1E;                       /* CH1 Output Enable, active High */
    TIM->BDTR  = TIM_BDTR_MOE;                        /* Master output enable */
    TIM->SMCR  = TIM_SMCR_MSM;                        /* Delay trigger for perfect sync */
    TIM->DIER  = TIM_DIER_CC2IE;                      /* Interrupt on channel 2 match only */

    /* Now setup the interrupts... */
    NVIC_SetPriorityGrouping(0U);

    /* Interrupt TIM1 */
    NVIC_SetPriority(TIM1_CC_IRQn, HIGHPRI_IRQ);
    NVIC_EnableIRQ(TIM1_CC_IRQn);

    /* ...and prepare the DMA interrupt for refreshing the line buffer */
    NVIC_SetPriority(DMA_CHANNEL_IRQn, LOWPRI_IRQ);
    NVIC_EnableIRQ(DMA_CHANNEL_IRQn);

    TIM->CR1 = TIM_CR1_CEN; /* timer1 (Line timer) run */

    return _v.d;
}

/* ============================================================================================ */
