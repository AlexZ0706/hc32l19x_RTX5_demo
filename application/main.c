#include "gpio.h"
#include "cmsis_os2.h"
#include "flash.h"
#include "shell_port.h"
#include "lfs_port.h"

static void App_LedInit(void);
void AppTaskLED(void *argument);

const osThreadAttr_t ThreadLED_Attr =
{
    .name = "osRtxLEDThread",
    .attr_bits = osThreadDetached,
    .priority = osPriorityHigh2,
    .stack_size = 2048,
};

osThreadId_t ThreadIdTaskLED = NULL;

//外置晶振配置(8M)
static void system_clock_init(void)
{
    ///< 外部高速初始化时钟配置
    ///< 切换时钟前（根据外部高速晶振）设置XTH频率范围,配置晶振参数，使能目标时钟
    Sysctrl_SetXTHFreq(SysctrlXthFreq8_16MHz);
    Sysctrl_XTHDriverCfg(SysctrlXtalDriver3);
    Sysctrl_SetXTHStableTime(SysctrlXthStableCycle16384);
    Sysctrl_ClkSourceEnable(SysctrlClkXTH, TRUE);
    
    ///< 时钟切换
    Sysctrl_SysClkSwitch(SysctrlClkXTH);
}

//外置晶振(8M) PLL 到 48M
static void App_SystemClkInit_PLL48M_byXTH(void)
{
    stc_sysctrl_pll_cfg_t stcPLLCfg;    
    
    ///< 切换时钟前（根据外部高速晶振）设置XTH频率范围,配置晶振参数，使能目标时钟
    Sysctrl_SetXTHFreq(SysctrlXthFreq4_8MHz);
    Sysctrl_XTHDriverCfg(SysctrlXtalDriver3);
    Sysctrl_SetXTHStableTime(SysctrlXthStableCycle16384);
    Sysctrl_ClkSourceEnable(SysctrlClkXTH, TRUE);
    delay1ms(10);
    
    stcPLLCfg.enInFreq    = SysctrlPllInFreq6_12MHz;    //XTH 8MHz
    stcPLLCfg.enOutFreq   = SysctrlPllOutFreq36_48MHz;  //PLL 输出
    stcPLLCfg.enPllClkSrc = SysctrlPllXthXtal;          //输入时钟源选择XTH
    stcPLLCfg.enPllMul    = SysctrlPllMul6;             //8MHz x 6 = 48MHz
    Sysctrl_SetPLLFreq(&stcPLLCfg); 
    
    ///< 当使用的时钟源HCLK大于24M：设置FLASH 读等待周期为1 cycle(默认值也为1 cycle)
    Flash_WaitCycle(FlashWaitCycle1);    

    ///< 使能PLL
    Sysctrl_ClkSourceEnable(SysctrlClkPLL, TRUE);    
    ///< 时钟切换到PLL
    Sysctrl_SysClkSwitch(SysctrlClkPLL);
}

int32_t main(void)
{ 
    //切换时钟
    //system_clock_init();
    App_SystemClkInit_PLL48M_byXTH();
    
    /* 内核初始化 */
	osKernelInitialize();

	/* 创建启动任务 */
	ThreadIdTaskLED = osThreadNew(AppTaskLED, NULL, &ThreadLED_Attr);
    
    //初始化 letter-shell
    shell_init();
    
    //初始化lfs
    lfs_init();

	/* 开启多任务 */
	osKernelStart();

    while(1)
    {
        osDelay(500);
    }
}

void AppTaskLED(void *argument)
{
	const uint16_t usFrequency = 500; /* 延迟周期 */
	uint32_t tick;

	/* 获取当前时间 */
	tick = osKernelGetTickCount();
    
    ///< LED端口初始化
    App_LedInit();
	
    while(1)
    {
        ///< LED点亮
        Gpio_SetIO(STK_LED_PORT, STK_LED_PIN);
        /* 相对延迟 */
		tick += usFrequency;                          
		osDelayUntil(tick);

        ///< LED关闭
        Gpio_ClrIO(STK_LED_PORT, STK_LED_PIN);
        
		/* 相对延迟 */
		tick += usFrequency;                          
		osDelayUntil(tick);
    }
}

static void App_LedInit(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    
    ///< 打开GPIO外设时钟门控
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE); 
    
    ///< 端口方向配置->输出(其它参数与以上（输入）配置参数一致)
    stcGpioCfg.enDir = GpioDirOut;
    ///< 端口上下拉配置->下拉
    stcGpioCfg.enPu = GpioPuDisable;
    stcGpioCfg.enPd = GpioPdEnable;
    
    ///< LED关闭
    Gpio_ClrIO(STK_LED_PORT, STK_LED_PIN);
    
    ///< GPIO IO LED端口初始化
    Gpio_Init(STK_LED_PORT, STK_LED_PIN, &stcGpioCfg);
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


