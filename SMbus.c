/*********************************************************
*程序功能: SMbus总线驱动程序
*SMbus版本：SMbus1.1
*MCU：STM32F207VE
*实现要求：STM32F207配套bq78350-R1、bq76930完成对电池智能管理
*         
*版本：v1.0
*时间：2019.2.18 16 ：57
*********************************************************/
#include "SMbus.h"
#include "Time_Delay.h"

void SMBus_GPIO_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/*************************************************
函数功能：SMBus起始信号
函数形参：无
函数返回值：错误值
函数备注：
PB6:SCL	开漏输出、外部上拉
PB7:SDA	开漏输出、外部上拉
************************************************/
u8 SMBus_Start(void)
{
	u16 temp = 0;
	SDA_H();
	rt_hw_us_delay(12);	//SCL上升时间
	SCL_H();
	rt_hw_us_delay(12);	//起始信号建立时间 加长一些
	while(SCL() == 0)		//等待从设备准备好数据并释放时钟总线
	{
		temp++;
		if(temp == 20000)			//TLOW：MEXT = 10ms 是允许主器件在从start-to-ack，ack-to-ack或ack-to-stop定义的消息的每个字节内延长其时钟周期的累积时间。
		{
			temp = 0;
			return 11;		
		}				
	}
	rt_hw_us_delay(12);	//结束信号建立时间
	SDA_L();
	rt_hw_us_delay(12);	//起始信号保持时间
	SCL_L();				//时钟默认拉低
	return 0;
}

/*************************************************
函数功能：SMBus停止信号
函数形参：无
函数返回值：错误值
函数备注：
PB6:SCL	开漏输出、外部上拉
PB7:SDA	开漏输出、外部上拉
************************************************/
u8 SMBus_Stop(void)
{
	u16 temp = 0;
	SCL_L();				//防止误停止BUG
	SDA_L();
	rt_hw_us_delay(12);	//SCL下降时间
	SCL_H();	
	rt_hw_us_delay(12);	//结束信号建立时间
	while(SCL() == 0)		//等待从设备准备好数据并释放时钟总线
	{
		temp++;
		if(temp == 20000)			//TLOW：MEXT = 10ms 是允许主器件在从start-to-ack，ack-to-ack或ack-to-stop定义的消息的每个字节内延长其时钟周期的累积时间。
		{
			temp = 0;
			return 99;		
		}				
	}
	rt_hw_us_delay(12);	//结束信号建立时间
	SDA_H();
	rt_hw_us_delay(12);	//结束信号保持时间
	return 0;
}

/*************************************************
函数功能：SMBus发送应答
函数形参：ack:0应答 1非应答
函数返回值：错误值
函数备注：
PB6:SCL	开漏输出、外部上拉
PB7:SDA	开漏输出、外部上拉
************************************************/
u8 SMBus_Send_ACK(u8 ack)
{
	u16 temp = 0;
	SCL_L();
	if(ack == 1)
	{
		SDA_H();		//发送非应答
	}
	else
	{
		SDA_L();		//发送应答
	}
	rt_hw_us_delay(12);	//数据建立时间
	SCL_H();
	rt_hw_us_delay(12);	//数据保持时间
	while(SCL() == 0)		//等待从设备准备好数据并释放时钟总线
	{
		temp++;
		if(temp == 20000)			//TLOW：MEXT = 10ms 是允许主器件在从start-to-ack，ack-to-ack或ack-to-stop定义的消息的每个字节内延长其时钟周期的累积时间。
		{
			temp = 0;
			return 66;		
		}				
	}
	rt_hw_us_delay(12);	//数据保持时间
	SCL_L();			
	return 0;
}

/*************************************************
函数功能：SMBus接收应答
函数形参：无
函数返回值：错误值
函数备注：
PB6:SCL	开漏输出、外部上拉
PB7:SDA	开漏输出、外部上拉
************************************************/
u8 SMBus_Read_ACK(void)
{
	u16 temp1 = 0;
	SCL_L();						//时钟线保证是低电平,低电平从机准备数据				
	SDA_H();						//释放总线
	rt_hw_us_delay(12);	//数据建立时间
	SCL_H();
	rt_hw_us_delay(12);	//数据保持时间
	while(SCL() == 0)		//等待从设备准备好数据并释放时钟总线
	{
		temp1++;
		if(temp1 == 20000)			//TLOW：MEXT = 10ms 是允许主器件在从start-to-ack，ack-to-ack或ack-to-stop定义的消息的每个字节内延长其时钟周期的累积时间。
		{
			temp1 = 0;
			return 33;		
		}				
	}
	rt_hw_us_delay(12);	//数据保持时间
	SCL_L();
	return 0;
}

/*************************************************
函数功能：SMBus发送数据+接收应答
函数形参：data:要发送的数据
函数返回值：0：应答 1：非应答
函数备注：
PB6:SCL	开漏输出、外部上拉
PB7:SDA	开漏输出、外部上拉
************************************************/
u8 SMBus_Send_data(u8 data)
{
	u8 i;
	u16 temp = 0;
	SCL_L();									//时钟线保证是低电平,低电平从机准备数据
	for(i = 0; i < 8; i++)
	{
		if(data & (0x80 >> i))	//主机准备数据
		{
			SDA_H();
		}
		else
		{
			SDA_L();
		}
		rt_hw_us_delay(12);	//数据建立时间
		SCL_H();						//主机发送数据
		rt_hw_us_delay(12);	//数据保持时间
		while(SCL() == 0)		//等待从设备准备好数据并释放时钟总线
		{
			temp++;
			if(temp == 20000)	//TLOW：MEXT = 10ms 是允许主器件在从start-to-ack，ack-to-ack或ack-to-stop定义的消息的每个字节内延长其时钟周期的累积时间。
			{
				temp = 0;
				return 55;		
			}				
		}
		rt_hw_us_delay(12);	//结束信号建立时间
		SCL_L();						//时钟线低电平，准备下一次发送
	}
	return SMBus_Read_ACK();		//接收应答	
}

/*************************************************
函数功能：SMBus读取数据+发送应答
函数形参：应答
函数返回值：接收到的数据
函数备注：
PB6:SCL	开漏输出、外部上拉
PB7:SDA	开漏输出、外部上拉
************************************************/
u8 SMBus_Read_data(u8 ack)
{
	u8 i;
	u8 data = 0;
	u16 temp = 0;
	SDA_H();								//释放总线
	rt_hw_us_delay(12);
	SCL_L();								//时钟线保证是低电平,低电平从机准备数据
	for(i = 0; i < 8; i++)	
	{
		rt_hw_us_delay(12);		//从机数据建立时间
		SCL_H();							//从机发送数据
		rt_hw_us_delay(12);		//从机数据保持时间
		while(SCL() == 0)			//等待从设备准备好数据并释放时钟总线
		{
			temp++;
			if(temp == 20000)		//TLOW：MEXT = 10ms 是允许主器件在从start-to-ack，ack-to-ack或ack-to-stop定义的消息的每个字节内延长其时钟周期的累积时间。
			{
				temp = 0;
				return 77;		
			}				
		}
		rt_hw_us_delay(12);		//结束信号建立时间
		data <<= 1;
		if(SDA() == 1)
		{
			data |= 1;
		}
		SCL_L();	
	}
	SMBus_Send_ACK(ack);
	return data;		
}







