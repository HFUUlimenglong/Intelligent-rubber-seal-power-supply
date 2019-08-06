#include "stm32f2xx.h"
#include "stm32f2xx_can.h"
#include "stm32f2xx_rcc.h"
#include "stm32f2xx_gpio.h"
#include "can.h"
//#include "protocol.h"


#include <rtthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "stm32f2xx.h"
#include "usart.h"

#include "nwk_protocol.h"
#include "cmd_type.h"
#include "3g_protocol.h"
#include "crc.h"
#include "devrange.h"

#include "bootcfg.h"
#include "ptl_nwk.h"
#include "bsmac_parser.h"
#include "net_app.h"
#include "led_indicator.h"
#include "3g_thread.h"
#include "3g_watchdog.h"
#include "../../../../../version.h"

//#define LOG_DEBUG
#include "3g_log.h"

uint8_t CAN_Ping_flag = 0;   //定时CAN口开始心跳包测试标志

struct QUEUE can_cache_t;
uint16_t can_tx_success_flag = RT_TRUE;
extern uint8_t is_net_ok;
extern uint8_t is_can_ok;

/*
该机制作用是将一条很长的数据拆分成若干次分别发送，最后组合成一条完整的数据包
*/
CAN_DATA_SEND can_send;
uint8_t can_send_buf[128];
uint8_t can_send_len = 0;        
uint8_t can_send_all_len = 0;

//队列初始化
void QUEUE_init(struct QUEUE *hp)
{
	hp->front = hp->real = NULL;
}

//入队
void QUEUE_input(struct QUEUE *hp,uint8_t * data, uint8_t data_len)
{
	struct CACHE_DATA *p =(struct CACHE_DATA *)rt_malloc(sizeof(struct CACHE_DATA));
	if(p == NULL)
	{
		return ;
	}
	rt_memcpy(p->data,data,data_len);

	p->next=NULL;
	if(hp->front == NULL)
	{
		hp->front=p;
		hp->real=p;	
	}
	else
	{
		hp->real->next=p;
		hp->real=p;
	}	
}


//出队
void QUEUE_output(struct QUEUE *hp,uint8_t *data,uint8_t len)
{
	struct CACHE_DATA *p=hp->front;
	if(hp->front == NULL)
	{
		return ;
	}
  
	rt_memcpy(data,p->data,len);
	
	hp->front = p->next;
	rt_free(p);
	if(hp->front == NULL)
	{
		hp->real=NULL;
	}
}


//清空队列
void QUEUE_clear(struct QUEUE *hp)
{
	struct CACHE_DATA *p = hp->front;
	if(hp->front == NULL)
	{
		return;
	}
	while(p != NULL)
	{
		hp->front=p->next;
		rt_free(p);
		p=hp->front;
	}
	hp->front=hp->real=NULL;
}

//检查队列是否有成员
char QUEUE_check(struct QUEUE *hp) 
{
	char queue_state = RT_FALSE;
	if(hp->front == NULL)
		queue_state = RT_FALSE ;
	else if(hp->front == hp->real)
		queue_state = RT_FALSE ;
	else
		queue_state = RT_TRUE;
	
	return queue_state;
}


/*
 ------------can发送配置参数------------
 Id:can的id，标准帧11位，扩展帧29位
 IDE:   CAN_Id_Standard 标准帧     CAN_Id_Extended 扩展帧   
 RTR:  CAN_RTR_Data 数据帧        CAN_RTR_Remote 远程帧
 can_data: 数据帧才需要，最多8个数据
 len:报文长度，最大为8
*/
uint8_t can_data_send(uint32_t Id,uint8_t IDE,uint8_t RTR,uint8_t *can_data,uint8_t len)
{
	u8 transmit_mailbox = 0;
	CanTxMsg TxMessage;
	
	if(IDE == CAN_Id_Extended)
		TxMessage.ExtId = Id;
	else if(IDE == CAN_Id_Standard)
		TxMessage.StdId=Id;			
		
	TxMessage.IDE=IDE; 
	TxMessage.RTR=RTR;	
	TxMessage.DLC = len;
	
	rt_memcpy(&TxMessage.Data[0], can_data, len);
	transmit_mailbox = CAN_Transmit(CAN1, &TxMessage);
	
	return transmit_mailbox;        	
}


//检测can总线状态，can总线有3个发送邮箱，检测发送邮箱是否可以用作为判定条件
uint8_t CAN_Check(CAN_TypeDef* CANx)
{
	uint8_t mbox_empty = 0;
	if ((CANx->TSR&CAN_TSR_TME0) == CAN_TSR_TME0)      //邮箱0可用
	{
		mbox_empty = 0;
	}
	else if ((CANx->TSR&CAN_TSR_TME1) == CAN_TSR_TME1)      //邮箱1可用
	{
		mbox_empty = 1;
	}
	else if ((CANx->TSR&CAN_TSR_TME2) == CAN_TSR_TME2)     //邮箱2可用
	{
		mbox_empty = 2;
	}
	else
	{
		mbox_empty = CAN_TxStatus_NoMailBox;      //邮箱不可用(邮箱满或者can不通)
	}
	return mbox_empty;
}



//can接收中断
void CAN1_RX0_IRQHandler(void)
{
	CanRxMsg RxMessage;
	//CAN接收
	CAN_Receive(CAN1,CAN_FIFO0,&RxMessage);//相当于将can接收的数据存在FIFO0的RxMessage里
}


//进发送中断的目的是为了设置can发送成功标志位
void CAN1_TX_IRQHandler(void) //CAN TX       CAN1_RX0_IRQn   CAN1_TX_IRQn
{
  if (CAN_GetITStatus(CAN1,CAN_IT_TME)!= RESET)
	{
	   	CAN_ClearITPendingBit(CAN1,CAN_IT_TME);
  }
}

//can心跳包，每隔1秒检测can总线，如果连续3秒检测到can总线不通则视为can断开
static void can_live_poll(void )
{
	static uint8_t check_num = 0;
	uint8_t mbox = 0;

	if(CAN_Ping_flag)
	{   
    CAN_Ping_flag = 0;
		mbox = CAN_Check(CAN1);
		if(mbox != CAN_TxStatus_NoMailBox)
		{
      rt_kprintf("CAN ping Ok\n");
			is_can_ok = RT_TRUE;
			check_num = 0;
		}
		else
		{
			check_num ++;
			if(check_num >= 3)
			{
				is_can_ok = RT_FALSE;
				check_num = 0;
			}
				
		}		
	}
}



//can1初始化
void can_thread_init(void)
{
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;	

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	/* 引脚配置 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;                          //接收引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;                       //上拉输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	  
	/* Configure CAN pin: TX */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);  

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_11;
	GPIO_Init(GPIOA, &GPIO_InitStructure);			

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_CAN1);
       GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_CAN1);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1,ENABLE);
	CAN_DeInit(CAN1);
	CAN_StructInit(&CAN_InitStructure);

	//关闭时间触发模式
	CAN_InitStructure.CAN_TTCM=DISABLE;
	//关闭自动离线管理
	CAN_InitStructure.CAN_ABOM=DISABLE;
	//关闭自动唤醒模式
	CAN_InitStructure.CAN_AWUM=DISABLE;
	//报文自动重传
	CAN_InitStructure.CAN_NART=DISABLE;
	//FIFO溢出时报文覆盖源文件
	CAN_InitStructure.CAN_RFLM=DISABLE;
	//报文发送优先级取决于发送顺序
	CAN_InitStructure.CAN_TXFP=DISABLE;
	//工作模式（正常）
	CAN_InitStructure.CAN_Mode=CAN_Mode_Normal;
  //配置CAN波特率 1M =30/CAN_Prescaler/(1+6+8)
	CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1=CAN_BS1_8tq;
	CAN_InitStructure.CAN_BS2=CAN_BS2_6tq;
//	CAN_InitStructure.CAN_Prescaler = 200;  //10kbps
//	CAN_InitStructure.CAN_Prescaler = 100;  //20kbps
//	CAN_InitStructure.CAN_Prescaler = 50;  //40kbps
//	CAN_InitStructure.CAN_Prescaler = 40;  //50kbps
	CAN_InitStructure.CAN_Prescaler = 20;  //100kbps

	//初始化CAN
	CAN_Init(CAN1,&CAN_InitStructure);

	//屏蔽滤波（can接收才涉及）
	CAN_FilterInitStructure.CAN_FilterNumber=0;//0号滤波器
	//屏蔽滤波模式
	CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;//标识符屏蔽位模式
	//32位寄存器
	CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
	//标识符寄存器高16位
	CAN_FilterInitStructure.CAN_FilterIdHigh=0x0000;
	//标识符寄存器低16位
	CAN_FilterInitStructure.CAN_FilterIdLow=0x0000;
	//屏蔽寄存器高16位
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x0000;
	//屏蔽寄存器低16位
	CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x0000;
	//过滤器将ID报文关联到FIFO0缓存区中，数据只能从这里导出
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0;
	//过滤器使能
	CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;
	//初始化过滤器
	CAN_FilterInit(&CAN_FilterInitStructure);

	//接收中断使能
	CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);
	//发送中断使能
	CAN_ITConfig(CAN1,CAN_IT_TME,ENABLE);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); 

	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);


	NVIC_InitStructure.NVIC_IRQChannel = CAN1_TX_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority= 1; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);

}

static void timeout5(void *parameter)
{
  CAN_Ping_flag = 1;
}

/*
** can 线程入口
*/
void can_thread_entry(void * param)
{	
	uint8_t len = 0;
	uint8_t mbox = 0;
  static rt_timer_t timer5;
	QUEUE_init(&can_cache_t);
	can_thread_init();
  /*创建ping定时器5，周期定时器*/
  timer5 = rt_timer_create("timer2",timeout5,RT_NULL,10000,RT_TIMER_FLAG_PERIODIC|RT_TIMER_FLAG_HARD_TIMER);
  /*启动定时器5*/
  if(timer5 != RT_NULL) rt_timer_start(timer5); 
	while(1)
	{
		if(can_tx_success_flag == RT_FALSE)
		{
			if(CAN_Check(CAN1) != CAN_TxStatus_NoMailBox)
			{
				if(can_send.can_sended_len < can_send.can_all_len)
				{
					//rt_kprintf(" send_len = %d  all_len = %d\n",can_send.can_sended_len , can_send.can_all_len);
					if(can_send.can_all_len -8 > can_send.can_sended_len)
						len = 8;
					else
						len = can_send.can_all_len - can_send.can_sended_len;
                
					mbox = can_data_send(sys_option.u32BsId+20000,CAN_Id_Extended,CAN_RTR_Data,can_send.can_buf+can_send.can_sended_len,len);
					if(mbox != CAN_TxStatus_NoMailBox)
					{
						can_send.can_sended_len += len;
					}	
								
					if(can_send.can_sended_len >= can_send.can_all_len)
					{
						rt_memset(&can_send, 0, sizeof(CAN_DATA_SEND));
						can_tx_success_flag = RT_TRUE;
					}						
				}			
			}
		}
		else
		{
			if(QUEUE_check(&can_cache_t))
			{	
        PACKET_HEADER_T *packhdr_head = (PACKET_HEADER_T *) can_send.can_buf;    
        
				QUEUE_output(&can_cache_t,can_send.can_buf,128);         //每次将队列内容全部复制，依据协议对队列内容解析，该方式有一定缺陷        
				can_send.can_all_len = packhdr_head->len+sizeof(PACKET_HEADER_T);  
				rt_kprintf(" can_send.can_all_len = %d\n",can_send.can_all_len);
				can_send.can_sended_len = 0;
				if((packhdr_head->len > 0) && (packhdr_head->len < 128))	
          can_tx_success_flag = RT_FALSE;
			}
		}
		rt_thread_delay(2);
		can_live_poll();
	}
}

static rt_thread_t can_thread = RT_NULL;

rt_bool_t start_can_work()
{
    can_thread = rt_thread_create("can", can_thread_entry,
                                    RT_NULL, 2048, 9 , 20);

    if (can_thread == RT_NULL)
    {
        ERROR_LOG("create can work thread failed\n");
        return RT_FALSE;
    }
    rt_thread_startup(can_thread);

    return RT_TRUE;
}

void stop_can_work()
{
    rt_enter_critical();

    if (can_thread != RT_NULL && can_thread->stat != RT_THREAD_CLOSE)
    {
        rt_thread_delete(can_thread);
    }

    rt_exit_critical();

    TIME_LOG(LOG_CRITICAL, "stop can thread\n");
}




