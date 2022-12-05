#ifndef _ITM_MESSAGES_H_
#define _ITM_MESSAGES_H_

#include <stdbool.h>
#include <stdint.h>
// ====================================================================================================
uint32_t ITM_Send8 ( uint32_t c, uint8_t d );
uint32_t ITM_Send16( uint32_t c, uint16_t d );
uint32_t ITM_Send32( uint32_t c, uint32_t d );
uint32_t ITM_SendString( uint32_t c, char *s );
void ITM_Enable(void);
void ITM_Disable(void);
void ITM_ChannelEnable(uint32_t ch);
void ITM_ChannelDisable(uint32_t ch);
bool ITM_ChannelEnabled(uint32_t ch);
// ====================================================================================================
#endif
