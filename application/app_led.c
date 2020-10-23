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
    
    // ��GPIO����ʱ���ſ�
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE); 
    
    // �˿ڷ�������->���(�������������ϣ����룩���ò���һ��)
    stcGpioCfg.enDir = GpioDirOut;
    // �˿�����������->����
    stcGpioCfg.enPu = GpioPuDisable;
    stcGpioCfg.enPd = GpioPdEnable;
    
    // LED�ر�
    Gpio_ClrIO(STK_LED_PORT, STK_LED_PIN);
    
    // GPIO IO LED�˿ڳ�ʼ��
    Gpio_Init(STK_LED_PORT, STK_LED_PIN, &stcGpioCfg);
}

void ledTask(void *argument)
{
    while(1)
    {
        // LED����
        Gpio_SetIO(STK_LED_PORT, STK_LED_PIN);
        osDelay(500);

        // LED�ر�
        Gpio_ClrIO(STK_LED_PORT, STK_LED_PIN);
        osDelay(500);
    }
}

int appLedInit(void)
{
    // LED�˿ڳ�ʼ��
    ledInit();
    
    // ������������
	threadId = osThreadNew(ledTask, NULL, &threadAttr);
    return 0;
}
INIT_APP_EXPORT(appLedInit);
