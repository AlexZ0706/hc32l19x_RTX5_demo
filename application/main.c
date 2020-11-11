#include "cmsis_os2.h"
#include "flash.h"
#include "auto_init.h"

// 外置晶振配置(8M)
static void system_clock_init(void)
{
    // 外部高速初始化时钟配置
    // 切换时钟前（根据外部高速晶振）设置XTH频率范围,配置晶振参数，使能目标时钟
    Sysctrl_SetXTHFreq(SysctrlXthFreq8_16MHz);
    Sysctrl_XTHDriverCfg(SysctrlXtalDriver3);
    Sysctrl_SetXTHStableTime(SysctrlXthStableCycle16384);
    Sysctrl_ClkSourceEnable(SysctrlClkXTH, TRUE);
    
    // 时钟切换
    Sysctrl_SysClkSwitch(SysctrlClkXTH);
}

// 外置晶振(8M) PLL 到 48M
static void App_SystemClkInit_PLL48M_byXTH(void)
{
    stc_sysctrl_pll_cfg_t stcPLLCfg;    
    
    // 切换时钟前（根据外部高速晶振）设置XTH频率范围,配置晶振参数，使能目标时钟
    Sysctrl_SetXTHFreq(SysctrlXthFreq4_8MHz);
    Sysctrl_XTHDriverCfg(SysctrlXtalDriver3);
    Sysctrl_SetXTHStableTime(SysctrlXthStableCycle16384);
    Sysctrl_ClkSourceEnable(SysctrlClkXTH, TRUE);
    delay1ms(10);
    
    stcPLLCfg.enInFreq    = SysctrlPllInFreq6_12MHz;    // XTH 8MHz
    stcPLLCfg.enOutFreq   = SysctrlPllOutFreq36_48MHz;  // PLL 输出
    stcPLLCfg.enPllClkSrc = SysctrlPllXthXtal;          // 输入时钟源选择XTH
    stcPLLCfg.enPllMul    = SysctrlPllMul6;             // 8MHz x 6 = 48MHz
    Sysctrl_SetPLLFreq(&stcPLLCfg); 
    
    // 当使用的时钟源HCLK大于24M：设置FLASH 读等待周期为1 cycle(默认值也为1 cycle)
    Flash_WaitCycle(FlashWaitCycle1);    

    // 使能PLL
    Sysctrl_ClkSourceEnable(SysctrlClkPLL, TRUE);    
    // 时钟切换到PLL
    Sysctrl_SysClkSwitch(SysctrlClkPLL);
}

int32_t main(void)
{
    // 重置中断向量表地址
    *((volatile uint32_t*) 0xE000ED08) = 0x4000;
    
    // 切换时钟
    // system_clock_init();
    App_SystemClkInit_PLL48M_byXTH();
    
    // level 1 函数列表初始化
    level1_init();
    
    // 内核初始化
	osKernelInitialize();
    
    // level 2-6 函数列表初始化
    level2_6_init();

	// 开启多任务
	osKernelStart();

    while(1)
    {
        osDelay(500);
    }
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
