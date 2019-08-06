#ifndef _SMBus_H_
#define _SMBus_H_
#include "stm32f2xx.h"

#define  SMBC_GPIO_Port  RCC_AHB1Periph_GPIOB
#define  SMBD_GPIO_Port  RCC_AHB1Periph_GPIOB
#define  SMBC_Pin        GPIO_Pin_6
#define  SMBD_Pin        GPIO_Pin_7


#define SCL_L()		GPIO_WriteBit(GPIOB,SMBC_Pin,Bit_RESET)
#define SCL_H()		GPIO_WriteBit(GPIOB,SMBC_Pin,Bit_SET)

#define SDA_L()		GPIO_WriteBit(GPIOB,SMBD_Pin,Bit_RESET)
#define SDA_H()		GPIO_WriteBit(GPIOB,SMBD_Pin,Bit_SET)

#define SCL()			GPIO_ReadInputDataBit(GPIOB,SMBC_Pin)
#define SDA()			GPIO_ReadInputDataBit(GPIOB,SMBD_Pin)

#define 	ACK 			0
#define 	NOACK 		1

void SMBus_GPIO_init(void);
u8 SMBus_Start(void);
u8 SMBus_Stop(void);
u8 SMBus_Send_ACK(u8 ack);
u8 SMBus_Read_ACK(void);
u8 SMBus_Send_data(u8 data);
u8 SMBus_Read_data(u8 ack);
 

#endif


