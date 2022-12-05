#include "itm_messages.h"

/* Define registers locally in case CMSIS isn't being used */
#define DBG_DEMCR  (*(volatile uint32_t *)0xE000EDFC)
#define DBG_TCR    (*(volatile uint32_t *)0xE0000E80)
#define DBG_TER    (*(volatile uint32_t *)0xE0000E00)
#define DBG_PORT   ((volatile uint32_t *)0xE0000000)
#define DBG_LAR    (*(volatile uint32_t *)0xE0000FB0)

#define DBG_DEMCR_TRCENA (1<<24)
#define DBG_TCR_ITMENA   (1<<0)
// ====================================================================================================
__attribute__((__section__(".ramprog"))) static inline uint32_t _sendITM(uint32_t ch, uint32_t d, uint8_t size)
{
    if ((DBG_DEMCR & DBG_DEMCR_TRCENA) && /* Trace enabled */
            (DBG_TCR & DBG_TCR_ITMENA) && /* ITM enabled */
		(ITM_ChannelEnabled(ch) ) /* ITM Port c enabled */
       )
        {
            while (DBG_PORT[ch] == 0) {}; /* Port available? */
            switch(size)
                {
                    case 1:
                        (*((uint8_t *)&(DBG_PORT[ch]))) = (uint8_t) d;
                        return size;
                    case 2:
                        (*((uint16_t *)&(DBG_PORT[ch]))) = (uint16_t) d;
                        return size;
                    case 4:
                        DBG_PORT[ch] = d;
                        return size;
                    default:
                        break;
                }
        }
    return (0);
}
// ====================================================================================================
__attribute__((__section__(".ramprog"))) uint32_t ITM_Send8( uint32_t c, uint8_t d )

{
    return _sendITM( c, d, 1);
}
// ====================================================================================================
__attribute__((__section__(".ramprog"))) uint32_t ITM_Send16( uint32_t c, uint16_t d )

{
    return _sendITM( c, d, 2);
}
// ====================================================================================================
__attribute__((__section__(".ramprog"))) inline uint32_t ITM_Send32( uint32_t c, uint32_t d )

{
    return _sendITM( c, d, 4);
}
// ====================================================================================================
uint32_t ITM_SendString( uint32_t c, char *s )

{
    uint32_t cc=0;

    if ((s) && (ITM_ChannelEnabled(c)))
        {
            while ((s) && (*s))
                {
                    _sendITM( c, *s++, 1);
                    cc++;
                }
        }
    return cc;
}
// ====================================================================================================
void ITM_Enable(void)

{
    DBG_DEMCR|=DBG_DEMCR_TRCENA;
}
// ====================================================================================================
void ITM_Disable(void)

{
    DBG_DEMCR&=~DBG_DEMCR_TRCENA;
}
// ====================================================================================================
void ITM_ChannelEnable(uint32_t ch)

{
    DBG_TER|=(1<<ch);
}
// ====================================================================================================
void ITM_ChannelDisable(uint32_t ch)

{
    DBG_TER&=~(1<<ch);
}
// ====================================================================================================
bool ITM_ChannelEnabled(uint32_t ch)

{
    return ((DBG_TER&(1<<ch)) != 0);
}
// ====================================================================================================
