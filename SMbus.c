/*********************************************************
*������: SMbus������������
*SMbus�汾��SMbus1.1
*MCU��STM32F207VE
*ʵ��Ҫ��STM32F207����bq78350-R1��bq76930��ɶԵ�����ܹ���
*         
*�汾��v1.0
*ʱ�䣺2019.2.18 16 ��57
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
�������ܣ�SMBus��ʼ�ź�
�����βΣ���
��������ֵ������ֵ
������ע��
PB6:SCL	��©������ⲿ����
PB7:SDA	��©������ⲿ����
************************************************/
u8 SMBus_Start(void)
{
	u16 temp = 0;
	SDA_H();
	rt_hw_us_delay(12);	//SCL����ʱ��
	SCL_H();
	rt_hw_us_delay(12);	//��ʼ�źŽ���ʱ�� �ӳ�һЩ
	while(SCL() == 0)		//�ȴ����豸׼�������ݲ��ͷ�ʱ������
	{
		temp++;
		if(temp == 20000)			//TLOW��MEXT = 10ms �������������ڴ�start-to-ack��ack-to-ack��ack-to-stop�������Ϣ��ÿ���ֽ����ӳ���ʱ�����ڵ��ۻ�ʱ�䡣
		{
			temp = 0;
			return 11;		
		}				
	}
	rt_hw_us_delay(12);	//�����źŽ���ʱ��
	SDA_L();
	rt_hw_us_delay(12);	//��ʼ�źű���ʱ��
	SCL_L();				//ʱ��Ĭ������
	return 0;
}

/*************************************************
�������ܣ�SMBusֹͣ�ź�
�����βΣ���
��������ֵ������ֵ
������ע��
PB6:SCL	��©������ⲿ����
PB7:SDA	��©������ⲿ����
************************************************/
u8 SMBus_Stop(void)
{
	u16 temp = 0;
	SCL_L();				//��ֹ��ֹͣBUG
	SDA_L();
	rt_hw_us_delay(12);	//SCL�½�ʱ��
	SCL_H();	
	rt_hw_us_delay(12);	//�����źŽ���ʱ��
	while(SCL() == 0)		//�ȴ����豸׼�������ݲ��ͷ�ʱ������
	{
		temp++;
		if(temp == 20000)			//TLOW��MEXT = 10ms �������������ڴ�start-to-ack��ack-to-ack��ack-to-stop�������Ϣ��ÿ���ֽ����ӳ���ʱ�����ڵ��ۻ�ʱ�䡣
		{
			temp = 0;
			return 99;		
		}				
	}
	rt_hw_us_delay(12);	//�����źŽ���ʱ��
	SDA_H();
	rt_hw_us_delay(12);	//�����źű���ʱ��
	return 0;
}

/*************************************************
�������ܣ�SMBus����Ӧ��
�����βΣ�ack:0Ӧ�� 1��Ӧ��
��������ֵ������ֵ
������ע��
PB6:SCL	��©������ⲿ����
PB7:SDA	��©������ⲿ����
************************************************/
u8 SMBus_Send_ACK(u8 ack)
{
	u16 temp = 0;
	SCL_L();
	if(ack == 1)
	{
		SDA_H();		//���ͷ�Ӧ��
	}
	else
	{
		SDA_L();		//����Ӧ��
	}
	rt_hw_us_delay(12);	//���ݽ���ʱ��
	SCL_H();
	rt_hw_us_delay(12);	//���ݱ���ʱ��
	while(SCL() == 0)		//�ȴ����豸׼�������ݲ��ͷ�ʱ������
	{
		temp++;
		if(temp == 20000)			//TLOW��MEXT = 10ms �������������ڴ�start-to-ack��ack-to-ack��ack-to-stop�������Ϣ��ÿ���ֽ����ӳ���ʱ�����ڵ��ۻ�ʱ�䡣
		{
			temp = 0;
			return 66;		
		}				
	}
	rt_hw_us_delay(12);	//���ݱ���ʱ��
	SCL_L();			
	return 0;
}

/*************************************************
�������ܣ�SMBus����Ӧ��
�����βΣ���
��������ֵ������ֵ
������ע��
PB6:SCL	��©������ⲿ����
PB7:SDA	��©������ⲿ����
************************************************/
u8 SMBus_Read_ACK(void)
{
	u16 temp1 = 0;
	SCL_L();						//ʱ���߱�֤�ǵ͵�ƽ,�͵�ƽ�ӻ�׼������				
	SDA_H();						//�ͷ�����
	rt_hw_us_delay(12);	//���ݽ���ʱ��
	SCL_H();
	rt_hw_us_delay(12);	//���ݱ���ʱ��
	while(SCL() == 0)		//�ȴ����豸׼�������ݲ��ͷ�ʱ������
	{
		temp1++;
		if(temp1 == 20000)			//TLOW��MEXT = 10ms �������������ڴ�start-to-ack��ack-to-ack��ack-to-stop�������Ϣ��ÿ���ֽ����ӳ���ʱ�����ڵ��ۻ�ʱ�䡣
		{
			temp1 = 0;
			return 33;		
		}				
	}
	rt_hw_us_delay(12);	//���ݱ���ʱ��
	SCL_L();
	return 0;
}

/*************************************************
�������ܣ�SMBus��������+����Ӧ��
�����βΣ�data:Ҫ���͵�����
��������ֵ��0��Ӧ�� 1����Ӧ��
������ע��
PB6:SCL	��©������ⲿ����
PB7:SDA	��©������ⲿ����
************************************************/
u8 SMBus_Send_data(u8 data)
{
	u8 i;
	u16 temp = 0;
	SCL_L();									//ʱ���߱�֤�ǵ͵�ƽ,�͵�ƽ�ӻ�׼������
	for(i = 0; i < 8; i++)
	{
		if(data & (0x80 >> i))	//����׼������
		{
			SDA_H();
		}
		else
		{
			SDA_L();
		}
		rt_hw_us_delay(12);	//���ݽ���ʱ��
		SCL_H();						//������������
		rt_hw_us_delay(12);	//���ݱ���ʱ��
		while(SCL() == 0)		//�ȴ����豸׼�������ݲ��ͷ�ʱ������
		{
			temp++;
			if(temp == 20000)	//TLOW��MEXT = 10ms �������������ڴ�start-to-ack��ack-to-ack��ack-to-stop�������Ϣ��ÿ���ֽ����ӳ���ʱ�����ڵ��ۻ�ʱ�䡣
			{
				temp = 0;
				return 55;		
			}				
		}
		rt_hw_us_delay(12);	//�����źŽ���ʱ��
		SCL_L();						//ʱ���ߵ͵�ƽ��׼����һ�η���
	}
	return SMBus_Read_ACK();		//����Ӧ��	
}

/*************************************************
�������ܣ�SMBus��ȡ����+����Ӧ��
�����βΣ�Ӧ��
��������ֵ�����յ�������
������ע��
PB6:SCL	��©������ⲿ����
PB7:SDA	��©������ⲿ����
************************************************/
u8 SMBus_Read_data(u8 ack)
{
	u8 i;
	u8 data = 0;
	u16 temp = 0;
	SDA_H();								//�ͷ�����
	rt_hw_us_delay(12);
	SCL_L();								//ʱ���߱�֤�ǵ͵�ƽ,�͵�ƽ�ӻ�׼������
	for(i = 0; i < 8; i++)	
	{
		rt_hw_us_delay(12);		//�ӻ����ݽ���ʱ��
		SCL_H();							//�ӻ���������
		rt_hw_us_delay(12);		//�ӻ����ݱ���ʱ��
		while(SCL() == 0)			//�ȴ����豸׼�������ݲ��ͷ�ʱ������
		{
			temp++;
			if(temp == 20000)		//TLOW��MEXT = 10ms �������������ڴ�start-to-ack��ack-to-ack��ack-to-stop�������Ϣ��ÿ���ֽ����ӳ���ʱ�����ڵ��ۻ�ʱ�䡣
			{
				temp = 0;
				return 77;		
			}				
		}
		rt_hw_us_delay(12);		//�����źŽ���ʱ��
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







