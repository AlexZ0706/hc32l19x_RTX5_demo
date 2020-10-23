#include "auto_init.h"
#include "cmsis_os2.h"
#include "gpio.h"

static const osThreadAttr_t threadAttr =
{
    .name = "osRtxLEDThread",
    .attr_bits = osThreadDetached,
    .priority = osPriorityLow,
    .stack_size = 256,
};

static osThreadId_t threadId = NULL;

static void ledInit(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    
    // 打开GPIO外设时钟门控
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE); 
    
    // 端口方向配置->输出(其它参数与以上（输入）配置参数一致)
    stcGpioCfg.enDir = GpioDirOut;
    // 端口上下拉配置->下拉
    stcGpioCfg.enPu = GpioPuDisable;
    stcGpioCfg.enPd = GpioPdEnable;
    
    // LED关闭
    Gpio_ClrIO(STK_LED_PORT, STK_LED_PIN);
    
    // GPIO IO LED端口初始化
    Gpio_Init(STK_LED_PORT, STK_LED_PIN, &stcGpioCfg);
}

void ledTask(void *argument)
{
    while(1)
    {
        // LED点亮
        Gpio_SetIO(STK_LED_PORT, STK_LED_PIN);
        osDelay(500);

        // LED关闭
        Gpio_ClrIO(STK_LED_PORT, STK_LED_PIN);
        osDelay(500);
    }
}

int appLedInit(void)
{
    // LED端口初始化
    ledInit();
    
    // 创建启动任务
	threadId = osThreadNew(ledTask, NULL, &threadAttr);
    return 0;
}
INIT_APP_EXPORT(appLedInit);
