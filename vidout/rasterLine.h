#ifndef _RASTERLINE_H_
#define _RASTERLINE_H_

#include <stdint.h>
#include "displayFile.h"
#include "vidout.h"

struct rasterFont

{
  const uint8_t height;
  const uint8_t width;

  const uint8_t firstChr;
  const uint8_t lastChr;  
  const uint8_t *d;
};

/* ============================================================================================ */

void rasterLine(struct displayFile *d, const struct rasterFont *f, uint32_t *w, uint32_t rl);

/* ============================================================================================ */

#endif /*  _RASTERLINE_H_ */
