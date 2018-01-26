#include "user_config.h"
#include  "uart1.h"
#include  "user_queue.h"


static uint8_t UART1SendBuf[256];
static SemaphoreHandle_t  *Uart1Sem;


extern uint32_t SystemCoreClock;


uint8_t Uart1WriteFull(DataQueue *Buf, QUEUE_DATA_TYPE Data, uint8_t Mod)
{
    uint16_t temp;

    Mod = Mod;

    temp = QueueSize((void *)Buf);
    while (temp <= QueueNData((void *)Buf))         /* �ȴ����ݶ��в��� */
    {
        vTaskDelay(2*5);
    }
    return QueueWrite((void *)Buf, Data);           /* ����������� */
}


uint8_t UART1_Init(uint32_t BPS)
{
    uint16_t Fdiv;
	uint32_t bps; 
    //ʹ����UART1��������Դ    
    LPC_SC->PCONP |= (1<<4);
    //Enable UART1 Pins.
	LPC_IOCON->P0_15 = 0x01;     //UART1_TXD
    LPC_IOCON->P0_16 = 0x01;     //UART1_RXD
	switch ( BPS )
    {
        case 1 :
             bps = 2400;
            break;
        case 2 :
             bps = 4800;
            break;
        case 3 :
             bps = 9600;
            break;
        case 4 :
             bps = 14400;
            break;
        case 5 :
             bps = 19200;
            break;
        case 6 :
             bps = 38400;
            break;
        case 7 :
             bps = 56000;
            break;
        case 8 :
             bps = 57600;
            break;
         case 9 :
             bps = 115200;
            break;               
        default:  break;
    }
	

    LPC_UART1->LCR = 0x80;                      // ������ʷ�Ƶ���ӼĴ���     
    Fdiv = (PeripheralClock / 16) / bps;        // ���ò����� 
    LPC_UART1->DLM = Fdiv / 256;
	  LPC_UART1->DLL = Fdiv % 256;
    LPC_UART1->LCR = 0x03;                      // 8N1ģʽ����ֹ���ʷ�Ƶ���ӼĴ��� 
                                                // ������Ϊ8,1,n 
	  LPC_UART1->IER = 0x05;                      // ������պͷ����ж� 
    LPC_UART1->FCR = 0x87;                      // ��ʼ��FIFO 

    NVIC_EnableIRQ(UART1_IRQn);                 // Enable UART1 interrupt 
	  if (QueueCreate((void *)UART1SendBuf,
                     sizeof(UART1SendBuf),
                     NULL,
                     (uint8_t (*)())Uart1WriteFull) 
                     == NOT_OK)
    {
        return FALSE;
    }
    Uart1Sem = xSemaphoreCreateBinary();
    if (Uart1Sem != NULL)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


void UART1Putch(uint8_t Data)
{
	  uint8_t temp;
	
    QueueWrite((void *)UART1SendBuf, Data);         /* ������� */
    portENTER_CRITICAL();
    if ((LPC_UART1->LSR & 0x00000020) != 0)
    {                                               /* UART0���ͱ��ּĴ����� */
        QueueRead(&temp, UART1SendBuf);             /* ���������ӵ����� */
        LPC_UART1->THR = temp;
        LPC_UART1->IER = LPC_UART1->IER | 0x02;     /* �������ж� */
    }
     portEXIT_CRITICAL();
}


void UART1Put_str(uint8_t *Data, uint16_t NByte)
{
    while (NByte-- > 0)
    {
        UART1Putch(*Data++);
    }
}


void UART1Write_Str(uint8_t *Data)
{
    while (*Data != '\0' )
    {
        UART1Putch(*Data++);
    }
}


uint8_t UART1Getch(void)
{
    uint8_t err;
	
    while ((LPC_UART1->LSR & 0x00000001) == 0)
    {                                           /* û���յ����� */
        LPC_UART1->IER = LPC_UART1->IER | 0x01; /* ��������ж� */
		xSemaphoreTake(Uart1Sem,1); 
    }
    err = LPC_UART1->RBR;                       /* ��ȡ�յ������� */
    return err;
}


uint8_t UART1Get(void)
{
    uint8_t err;
	
    if ((LPC_UART1->LSR & 0x00000001) == 0)
    {                                           /* û���յ����� */
        LPC_UART1->IER = LPC_UART1->IER | 0x01; /* ��������ж� */
		xSemaphoreTake(Uart1Sem,1); 
    }
    err = LPC_UART1->RBR;                       /* ��ȡ�յ������� */
    return err;
}


void UART1_IRQHandler (void) 
{
    uint8_t IIR, temp, i;
	static BaseType_t xHigherPriorityTaskWoken;
    portENTER_CRITICAL();

    while(((IIR = LPC_UART1->IIR) & 0x01) == 0)
    {                                                   /* ���ж�δ������ */
        switch (IIR & 0x0e)
        {
            case 0x02:                                  /* THRE�ж�    */
                for (i = 0; i < UART1_FIFO_LENGTH; i++) /* ����FIFO������� */
                {
                    if (QueueRead(&temp, UART1SendBuf) == QUEUE_OK)
                    {
                        LPC_UART1->THR = temp;
                    }
                    else
                    {
                        LPC_UART1->IER = LPC_UART1->IER & (~0x02);        /* ���пգ����ֹ�����ж� */
                    }
                }
                break;
            case 0x04:                                  /* �������ݿ��� */
               xSemaphoreGiveFromISR( Uart1Sem, &xHigherPriorityTaskWoken );                   /* ֪ͨ�������� */
                LPC_UART1->IER = LPC_UART1->IER & (~0x01);                /* ��ֹ���ռ��ַ���ʱ�ж� */
                break;
            case 0x06:                                  /* ������״̬   */
                temp = LPC_UART1->LSR;
                break;
            case 0x0c:                                  /* �ַ���ʱָʾ */
                 xSemaphoreGiveFromISR( Uart1Sem, &xHigherPriorityTaskWoken );                    /* ֪ͨ�������� */
                LPC_UART1->IER = LPC_UART1->IER & (~0x01);                /* ��ֹ���ռ��ַ���ʱ�ж� */
                break;
            default :
            	  temp = LPC_UART1->LSR;
                break;
        }
    }
    //VICVectAddr = 0;            // ֪ͨ�жϿ������жϽ���
    portEXIT_CRITICAL();
}







