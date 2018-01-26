#include "user_config.h"
#define CIRCLE_TIME 12

const size_t xStackSize = 25U;
TimerHandle_t MY_Timer = NULL;

void vWDTTask( void *pvParameters );
void vTCPTask( void *pvParameters );
void vAPPTask( void *pvParameters );
void HardWare_Init()
{
	RUN_LED_INIT;
	UART0Init(9);
}
int main(void)
{
	SystemInit();
	HardWare_Init();
	xTaskCreate(vWDTTask,"WDT",xStackSize,NULL,2,NULL);
	xTaskCreate(vTCPTask,"TCP",xStackSize,NULL,3,NULL);
	xTaskCreate(vAPPTask,"APP",xStackSize,NULL,4,NULL);
	vTaskStartScheduler();
	for(;;);
	return 0;
}
void vWDTTask( void *pvParameters )
{
	uint8_t f = 0;
	LPC_WDT->TC  = 0X1E848;  ; //设置WDT定时值为1秒.
    LPC_WDT->MOD = 0x03;       //设置WDT工作模式,启动WDT
	while(1)
	{
		portENTER_CRITICAL();
		LPC_WDT->FEED = 0xAA;
        LPC_WDT->FEED = 0x55;
		portEXIT_CRITICAL();
		if(f)
		{
			RUN_LED_ON;
			f =0;
		}
		else
		{
			RUN_LED_OFF;
			f =1;
		}
		vTaskDelay(500);
	}
}
void vTCPTask( void *pvParameters )
{
	while(1)
	{
		UART0Write_Str((uint8_t *)"aaaaaaaa\r\n");
		vTaskDelay(1000);
	}
}
void vAPPTask( void *pvParameters )
{

	while(1)
	{
		UART0Write_Str((uint8_t *)"bbbbbbbbb\r\n");
		vTaskDelay(1000);
	}
}

void vConfigureTimerForRunTimeStats( void )
{
    const unsigned long TCR_COUNT_RESET = 2, CTCR_CTM_TIMER = 0x00, TCR_COUNT_ENABLE = 0x01;

	/* This function configures a timer that is used as the time base when
	collecting run time statistical information - basically the percentage
	of CPU time that each task is utilising.  It is called automatically when
	the scheduler is started (assuming configGENERATE_RUN_TIME_STATS is set
	to 1). */

	/* Power up and feed the timer. */
//	LPC_SC->PCONP |= 0x02UL;
//	LPC_SC->PCLKSEL = (LPC_SC->PCLKSEL & (~(0x3<<2))) | (0x01 << 2);

//	/* Reset Timer 0 */
//	LPC_TIM0->TCR = TCR_COUNT_RESET;

//	/* Just count up. */
//	LPC_TIM0->CTCR = CTCR_CTM_TIMER;

//	/* Prescale to a frequency that is good enough to get a decent resolution,
//	but not too fast so as to overflow all the time. */
//	LPC_TIM0->PR =  ( configCPU_CLOCK_HZ / 10000UL ) - 1UL;

//	/* Start the counter. */
//	LPC_TIM0->TCR = TCR_COUNT_ENABLE;
}


