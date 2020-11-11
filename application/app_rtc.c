#include "rtc.h"
#include "lpm.h"
#include "gpio.h"
#include "auto_init.h"
#include "shell.h"
#include "log.h"

static int rtcInit(void)
{
    stc_rtc_initstruct_t RtcInitStruct;   
    DDL_ZERO_STRUCT(RtcInitStruct);                      // 变量初始值置零
    
    Sysctrl_SetPeripheralGate(SysctrlPeripheralRtc,TRUE);// RTC模块时钟打开    
    
    Sysctrl_ClkSourceEnable(SysctrlClkXTL, TRUE);        // 使能外部XTL时钟作为RTC时钟
    
    RtcInitStruct.rtcAmpm               = RtcPm;        // 24小时制
    RtcInitStruct.rtcClksrc             = RtcClkXtl;    // 外部低速时钟
    RtcInitStruct.rtcPrdsel.rtcPrdsel   = RtcPrds;      // 周期中断类型PRDS
    RtcInitStruct.rtcPrdsel.rtcPrds     = RtcNone;      // 不产生周期中断
    RtcInitStruct.rtcTime.u8Second      = 0x00;         // 配置RTC时间2019年4月17日10:01:55
    RtcInitStruct.rtcTime.u8Minute      = 0x45;
    RtcInitStruct.rtcTime.u8Hour        = 0x18;
    RtcInitStruct.rtcTime.u8Day         = 0x09;
    RtcInitStruct.rtcTime.u8DayOfWeek   = 0x01;
    RtcInitStruct.rtcTime.u8Month       = 0x11;
    RtcInitStruct.rtcTime.u8Year        = 0x20;
    RtcInitStruct.rtcCompen             = RtcCompenEnable;  // 使能时钟误差补偿
    RtcInitStruct.rtcCompValue          = 0;                // 补偿值  根据实际情况进行补偿
    Rtc_Init(&RtcInitStruct);
    
    EnableNvic(RTC_IRQn, IrqLevel3, TRUE);              // 使能RTC中断向量
    Rtc_Cmd(TRUE);                                      // 使能RTC开始计数
    
    return 0;
}

/*
void Rtc_IRQHandler(void)
{
    if (Rtc_GetAlmfItStatus() == TRUE) //闹铃中断
    {
        Rtc_ClearAlmfItStatus();       //清中断标志位
    }
}

void rtcAlarmSet()
{
    stc_rtc_alarmtime_t RtcAlmStruct;
    DDL_ZERO_STRUCT(RtcAlmStruct);
    
    Rtc_AlmIeCmd(FALSE);                                // 关闭闹钟中断
    RtcAlmStruct.RtcAlarmSec    = 0x05;
    RtcAlmStruct.RtcAlarmMinute = 0x02;
    RtcAlmStruct.RtcAlarmHour   = 0x10;
    RtcAlmStruct.RtcAlarmWeek   = 0x7f;                 // 从周一到周日，每天10:02:05启动一次闹铃    
    Rtc_SetAlarmTime(&RtcAlmStruct);                    // 配置闹铃时间    
    Rtc_AlmIeCmd(TRUE);                                 // 使能闹钟中断
}
*/

INIT_APP_EXPORT(rtcInit);

int shell_time(int argc, char *argv[])
{
    stc_rtc_time_t rtcTime;
    DDL_ZERO_STRUCT(rtcTime);
    
    Rtc_ReadDateTime(&rtcTime);
    
    logPrintln("20%X-%X-%X %02X:%02X:%02X", rtcTime.u8Year, 
                                            rtcTime.u8Month, 
                                            rtcTime.u8Day, 
                                            rtcTime.u8Hour,
                                            rtcTime.u8Minute,
                                            rtcTime.u8Second);
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), time, shell_time, getchar time);
