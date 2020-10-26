#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"

#include "uart.h"
#include "gpio.h"

#include "auto_init.h"
#include "log.h"
#include "shell.h"
#include "ringbuff.h"

const osThreadAttr_t ThreadShell_Attr = 
{
    .name = "osRtxShellThread",
    .attr_bits = osThreadDetached,
    .priority = osPriorityLow,
    .stack_size = 1024,
};
osThreadId_t ThreadIdTaskShell = NULL;

Shell       shell;
char        shell_buf[1024] = {0};
char        ringBuf[64] = {0};
sRingbuff   g_ring_buff = {0};

void uartLogWrite(char *buffer, short len);
Log uartLog = {
    .write = uartLogWrite,
    .active = true,
    .level = LOG_DEBUG
};

static void uart_init(void)
{
    stc_gpio_cfg_t       stcGpioCfg;
    stc_uart_cfg_t       stcCfg;
    stc_uart_baud_t      stcBaud;
    
    DDL_ZERO_STRUCT(stcGpioCfg);
    DDL_ZERO_STRUCT(stcCfg);
    DDL_ZERO_STRUCT(stcBaud);
    
    ring_buff_init(&g_ring_buff, ringBuf,  sizeof(ringBuf));
    
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio,TRUE);  // GPIO外设模块时钟使能
    Sysctrl_SetPeripheralGate(SysctrlPeripheralUart0,TRUE); // UART0外设模块时钟使能
    
    stcGpioCfg.enDir = GpioDirOut;
    Gpio_Init(GpioPortB, GpioPin8, &stcGpioCfg);
    Gpio_SetAfMode(GpioPortB, GpioPin8, GpioAf7);           // 配置PB08 为UART0 TX
    stcGpioCfg.enDir = GpioDirIn;
    Gpio_Init(GpioPortB, GpioPin9, &stcGpioCfg);
    Gpio_SetAfMode(GpioPortB, GpioPin9, GpioAf7);           // 配置PB09 为UART0 RX 
    
    stcCfg.enRunMode = UartMskMode1;                        // 模式1
    stcCfg.enStopBit = UartMsk1bit;                         // 1位停止位
    //stcCfg.enMmdorCk = UartMskEven;                       // 偶校验
    stcCfg.stcBaud.u32Baud  = 115200;                       // 波特率115200
    stcCfg.stcBaud.enClkDiv = UartMsk8Or16Div;              // 通道采样分频配置
    stcCfg.stcBaud.u32Pclk  = Sysctrl_GetPClkFreq();        // 获得外设时钟（PCLK）频率值
    Uart_Init(M0P_UART0, &stcCfg);                          // 串口初始化
    
    Uart_ClrStatus(M0P_UART0,UartRC);                       // 清接收请求
    Uart_ClrStatus(M0P_UART0,UartTC);                       // 清发送请求
    Uart_EnableIrq(M0P_UART0,UartRxIrq);                    // 使能串口接收中断
    //Uart_EnableIrq(M0P_UART0,UartTxIrq);                    // 使能串口发送中断
    EnableNvic(UART0_2_IRQn, IrqLevel3, TRUE);              // 系统中断使能
}

void Uart0_IRQHandler(void)
{
    if(Uart_GetStatus(M0P_UART0, UartRC))                   // UART0数据接收
    {
        Uart_ClrStatus(M0P_UART0, UartRC);                  // 清中断状态位
        uint8_t c = Uart_ReceiveData(M0P_UART0);            // 接收数据字节
        if(RINGBUFF_OK != ring_buff_push_data(&g_ring_buff, (uint8_t *)&c, 1))
        {
            // do not printf in isr at normal situation
        }
    }
}

static void shell_write(char data)
{
    Uart_SendDataPoll(M0P_UART0,data);                      // 查询方式发送数据
}

static int8_t shell_read(char *data)
{
    if(1 == ring_buff_pop_data(&g_ring_buff, (uint8_t *)data, 1))
    {
        return 0;
    }
    osDelay(2);
    return -1;
}

int shell_init(void)
{
    uart_init();
    
    shell.write = shell_write;
    shell.read = shell_read;
    shellInit(&shell, shell_buf, sizeof(shell_buf));
    logRegister(&uartLog, &shell);
    
    ThreadIdTaskShell = osThreadNew(shellTask, &shell, &ThreadShell_Attr);
    return 0;
}
INIT_PREV_EXPORT(shell_init);

void uartLogWrite(char *buffer, short len)
{
    if (uartLog.shell)
    {
        shellWriteEndLine(uartLog.shell, buffer, len);
    }
}

int func(int argc, char *argv[])
{
    logPrintln("%dparameter(s)", argc);
    for (char i = 1; i < argc; i++)
    {
        logPrintln("%s", argv[i]);
    }
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), func, func, test);

int reboot(int argc, char *argv[])
{
    NVIC_SystemReset();
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), reboot, reboot, reboot mcu);
