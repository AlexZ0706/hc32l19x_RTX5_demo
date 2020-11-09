#include "rtc.h"
#include "lua_if.h"

#define RTC_CONVERT_BIN2BCD(__VALUE__) (uint8_t)((((__VALUE__) / 10U) << 4U) | ((__VALUE__) % 10U))
#define RTC_CONVERT_BCD2BIN(__VALUE__) (uint8_t)(((uint8_t)((__VALUE__) & (uint8_t)0xF0U) >> (uint8_t)0x4U) * 10U + ((__VALUE__) & (uint8_t)0x0FU))

int system(const char *cmd)
{
    return 0;
}

time_t time(time_t *_t)
{
    struct tm tmNew;
    time_t  timeNow;
    stc_rtc_time_t rtcTime;
    
    DDL_ZERO_STRUCT(tmNew);
    DDL_ZERO_STRUCT(rtcTime);
    
    Rtc_ReadDateTime(&rtcTime);
    
    tmNew.tm_sec  = RTC_CONVERT_BCD2BIN(rtcTime.u8Second);
    tmNew.tm_min  = RTC_CONVERT_BCD2BIN(rtcTime.u8Minute);
    tmNew.tm_hour = RTC_CONVERT_BCD2BIN(rtcTime.u8Hour);
    tmNew.tm_mday = RTC_CONVERT_BCD2BIN(rtcTime.u8Day);
    tmNew.tm_mon  = RTC_CONVERT_BCD2BIN(rtcTime.u8Month);
    tmNew.tm_year = RTC_CONVERT_BCD2BIN(rtcTime.u8Year);
    
    timeNow = mktime(&tmNew);
    if(_t != NULL)
    {
        *_t = timeNow;
    }
    return timeNow;
}

void exit(int status)
{
    while(1);
}
