#include "bq78350.h"
#include "SMBus.h"
#include "Time_Delay.h"
/*************************************************
�������ܣ�BQ78350�ֽ�д�豸����
�����βΣ�addr:������ַ   vddr���洢���ĵ�ַ	data������	size�����ݴ�С
��������ֵ��0��������1������
������ע��
���ò��裺
������ʼ�ź�
���ʹ�������ַ + ����Ӧ��
�����ֽڵ�ַ + ����Ӧ��
�������� + ����Ӧ��			��λ��ǰ
��ַƫ��
���ͽ����ź�
************************************************/
u8 BQ78350_Write_Byte(u8 addr,u16 vddr,u16 type)
{
	u8 data_LSB = 0;	//�������ݵ��ֽ�
	u8 data_MSB = 0;	//�������ݸ��ֽ�
	
	u8 vddr_LSB = 0;	//���ͼĴ�����ַ���ֽ�
	u8 vddr_MSB = 0;	//���ͼĴ�����ַ���ֽ�
	
	/* ���ͼĴ�����ַ��Ϣ */
	vddr_LSB = (vddr & 0x00FF);
	vddr_MSB = (vddr & 0xFF00) >> 8;
	
	/* ���Ͷ�ȡ��Ϣ */
	data_LSB = (type & 0x00FF);
	data_MSB = (type & 0xFF00) >> 8;
	
	SMBus_Start();									//������ʼ�ź�
	if(SMBus_Send_data(addr))				//���ʹ�������ַ + ����Ӧ��
	{	
		SMBus_Stop();	
		return 1;
	}			
	if(SMBus_Send_data(vddr_LSB))				//�����ֽڵ�ַ + ����Ӧ��
	{
		SMBus_Stop();	
		return 2;
	}
	if(SMBus_Send_data(vddr_MSB))				//�����ֽڵ�ַ + ����Ӧ��
	{
		SMBus_Stop();	
		return 3;
	}
	if(SMBus_Send_data(data_LSB))	//�������� + ����Ӧ��													
	{			
		SMBus_Stop();			
		return 4;
	}		
	if(SMBus_Send_data(data_MSB))	//�������� + ����Ӧ��													
	{			
		SMBus_Stop();			
		return 5;
	}		
	SMBus_Stop();										//���ͽ����ź�
	return 0;
}

/*************************************************
�������ܣ�BQ78350��ȡ - 0X00��������
�����βΣ�addr:������ַ   vddr���洢���ĵ�ַ	data������	size�����ݴ�С
��������ֵ��0��������1������
������ע�� 
8���ֽ�Ϊһҳ��addr = 0xa0��
���ò��裺
������ʼ�ź�
���ʹ�������ַ + ����Ӧ��
�����ֽڵ�ַ + ����Ӧ��
�������� + ����Ӧ��			��λ��ǰ
��ַƫ��
���ͽ����ź�
************************************************/
u8 BQ78350_SBS_CMD(u8 addr,u16 vddr,u8* data,u8 size)
{
	u8 error = 0;
	error = BQ78350_SBS_Write(addr, vddr);
	error = BQ78350_SBS_Read(addr, data, size);
	return error;
}

/*************************************************
�������ܣ�BQ78350��ȡ�豸����
�����βΣ�addr:������ַ   vddr���洢���ĵ�ַ	data������	size�����ݴ�С
��������ֵ��0��������1������
������ע��
8���ֽ�Ϊһҳ��addr = 0xa0��
���ò��裺
������ʼ�ź�
���ʹ�������ַ + ����Ӧ��
�����ֽڵ�ַ + ����Ӧ��
�������� + ����Ӧ��			��λ��ǰ
��ַƫ��
���ͽ����ź�
************************************************/
u8 BQ78350_Block_CMD(u8 addr,u16 vddr,u8* data,u8 size)
{
	u8 error = 0;
	error = BQ78350_Block_Write(addr, vddr);
	error = BQ78350_Block_Read(addr, data, size);
	return error;
}

/*************************************************
�������ܣ��ж�����ֵ
�����βΣ�data
��������ֵ��value
������ע��
*********************************/
signed int Complement_u16data(u16 data)
{
	u16 temp = 0;
	if(data & (1<<15))
	{
		temp = 65536 - data;
		return -temp;
	}
	else
		return data;
}

signed char Complement_u8data(u8 data)
{
	u8 temp = 0;
	if(data & (1<<7))
	{
		temp = 256 - data;
		return -temp;
	}
	else
		return data;
}
/*************************************************
�������ܣ�BQ78350��ȡ�豸����
�����βΣ�addr:������ַ   vddr���洢���ĵ�ַ	data������	size�����ݴ�С
��������ֵ��0��������1������
������ע��
8���ֽ�Ϊһҳ��addr = 0xa0��
���ò��裺
������ʼ�ź�
���ʹ�������ַ + ����Ӧ��
�����ֽڵ�ַ + ����Ӧ��
�������� + ����Ӧ��			��λ��ǰ
��ַƫ��
���ͽ����ź�
************************************************/
u32 BQ78350_Read_DevType2(u8* R_data,u8 vddr)
{
	u8 i;
	
	/* д��Ϣ  -  ���������ݺ�����ȴ�����Ϊ����Ӧ�������Ѿ��ȴ�Ӧ�𣬴�����Ӧ��˵���Ѿ�׼����дһ��д���� */
	SMBus_Start();									//������ʼ�ź�

	if(SMBus_Send_data(0x16))				//���ʹ�������ַ + ����Ӧ��
	{	
		SMBus_Stop();	
		return 11;
	}		
	if(SMBus_Send_data(vddr))				//���Ϳ�R/W���� + ����Ӧ��
	{
		SMBus_Stop();	
		return 22;
	}
	SMBus_Start();									//������ʼ�ź�
	if(SMBus_Send_data(0x17))				//���ʹ�������ַ + ����Ӧ��
	{	
		SMBus_Stop();	
		return 99;
	}		
	for(i = 0; i < 3; i++)										//�������� + ����Ӧ��
	{
		if(i == 2)
		{	
			*R_data++ = SMBus_Read_data(NOACK);	//���ͷ�Ӧ��	
		}
		else 
		{
			*R_data++ = SMBus_Read_data(ACK);	
		}			
	}				
	SMBus_Stop();										//���ͽ����ź�
	return 0;
}

/*************************************************
�������ܣ�BQ78350��ȡ�豸����
�����βΣ�addr:������ַ   vddr���洢���ĵ�ַ	data������	size�����ݴ�С
��������ֵ��0��������1������
������ע��
8���ֽ�Ϊһҳ��addr = 0xa0��
���ò��裺
������ʼ�ź�
���ʹ�������ַ + ����Ӧ��
�����ֽڵ�ַ + ����Ӧ��
�������� + ����Ӧ��			��λ��ǰ
��ַƫ��
���ͽ����ź�
************************************************/
u32 BQ78350_Read_Byte(u8 addr,u16 vddr)
{
	u8 i;
	u32 data = 0;
	u16 temp = 0;//����һ���ֽ�
	
	/* ���ͼĴ�����ַ��Ϣ */
	u8 vddr_LSB = (vddr & 0x00FF);				//���ͼĴ�����ַ���ֽ�
	
	/* ��ȡ��Ϣ */
	SMBus_Start();									//������ʼ�ź�
	if(SMBus_Send_data(addr))				//���ʹ�������ַ + ����Ӧ��
	{	
		SMBus_Stop();	
		return 1;
	}		
	if(SMBus_Send_data(vddr_LSB))				//�����ֽڵ�ַ + ����Ӧ��
	{
		SMBus_Stop();	
		return 2;
	}
	SMBus_Start();		//������ʼ�ź�
	if(SMBus_Send_data(addr | 0x01))				//���ʹ�������ַ + ����Ӧ��
	{	
		SMBus_Stop();	
		return 4;
	}		
	for(i = 0; i < 2; i++)										//�������� + ����Ӧ��
	{
		if(i == 1)
		{	
			temp = SMBus_Read_data(NOACK);	//���ͷ�Ӧ��
			temp <<= 8;	
		}
		else 
		{
			temp = SMBus_Read_data(ACK);	
		}			
		data |= temp;
	}								
	SMBus_Stop();										//���ͽ����ź�
	return data;
}

/*************************************************
�������ܣ�BQ78350��ȡ8λ�Ĵ�����ַ������
�����βΣ�addr:������ַ   vddr���洢���ĵ�ַ	data������	size�����ݴ�С
��������ֵ��0��������1������
������ע��
8���ֽ�Ϊһҳ��addr = 0xa0��
���ò��裺
������ʼ�ź�
���ʹ�������ַ + ����Ӧ��
�����ֽڵ�ַ + ����Ӧ��
�������� + ����Ӧ��			��λ��ǰ
��ַƫ��
���ͽ����ź�
************************************************/
u8 BQ78350_Read_buf(u8 addr,u16 vddr,u8* data,u8 size)
{
	u8 i;
	
	/* ���ͼĴ�����ַ��Ϣ */
	u8 vddr_LSB = (vddr & 0xFF);				//���ͼĴ�����ַ���ֽ�
	
	/* ��ȡ��Ϣ */
	SMBus_Start();									//������ʼ�ź�
	if(SMBus_Send_data(addr))				//���ʹ�������ַ + ����Ӧ��
	{	
		SMBus_Stop();	
		return 1;
	}		
	if(SMBus_Send_data(vddr_LSB))				//�����ֽڵ�ַ + ����Ӧ��
	{
		SMBus_Stop();	
		return 2;
	}
	SMBus_Start();		//������ʼ�ź�
	if(SMBus_Send_data(addr | 0x01))				//���ʹ�������ַ + ����Ӧ��
	{	
		SMBus_Stop();	
		return 4;
	}		
	for(i = 0; i < size; i++)										//�������� + ����Ӧ��
	{
		if(i == size-1)
		{	
			*data++ = SMBus_Read_data(NOACK);	//���ͷ�Ӧ��
		}
		else 
		{
			*data++ = SMBus_Read_data(ACK);	
		}			
	}								
	SMBus_Stop();										//���ͽ����ź�
	return 0;
}

/*************************************************
�������ܣ�BQ78350��д
�����βΣ�addr:������ַ   vddr���洢���ĵ�ַ	data������	size�����ݴ�С(�ֽ�)
��������ֵ��0��������1������
������ע��
8���ֽ�Ϊһҳ��addr = 0xa0��
���ò��裺
������ʼ�ź�
���ʹ�������ַ + ����Ӧ��
�����ֽڵ�ַ + ����Ӧ��
�������� + ����Ӧ��
��ַƫ��
���ͽ����ź�
************************************************/
u8 BQ78350_SBS_Write(u8 addr,u16 vddr)
{
	/* ���ͼĴ�����ַ��Ϣ */
	u8 vddr_LSB = (vddr & 0x00FF);				//���ͼĴ�����ַ���ֽ�
	u8 vddr_MSB = (vddr & 0xFF00) >> 8;		//���ͼĴ�����ַ���ֽ�
	
	SMBus_Start();									//������ʼ�ź�
	if(SMBus_Send_data(addr))				//���ʹ�������ַ + ����Ӧ��
	{	
		return 1;
	}												
	if(SMBus_Send_data(0x00))				//�����ֽڵ�ַ + ����Ӧ��
	{
		return 2;
	}
//	if(SMBus_Send_data(0x02))				//�����ֽڵ�ַ + ����Ӧ��
//	{
//		return 3;
//	}
	if(SMBus_Send_data(vddr_MSB))				//�����ֽڵ�ַ + ����Ӧ��
	{
		return 4;
	}
	if(SMBus_Send_data(vddr_LSB))				//�����ֽڵ�ַ + ����Ӧ��
	{
		return 5;
	}												
	SMBus_Stop();										//���ͽ����ź�
	return 0;
}

/*************************************************
�������ܣ�BQ78350��д
�����βΣ�addr:������ַ   vddr���洢���ĵ�ַ	data������	size�����ݴ�С(�ֽ�)
��������ֵ��0��������1������
������ע��
8���ֽ�Ϊһҳ��addr = 0xa0��
���ò��裺
������ʼ�ź�
���ʹ�������ַ + ����Ӧ��
�����ֽڵ�ַ + ����Ӧ��
�������� + ����Ӧ��
��ַƫ��
���ͽ����ź�
************************************************/
u8 BQ78350_Block_Write(u8 addr,u16 vddr)
{
	/* ���ͼĴ�����ַ��Ϣ */
	u8 vddr_LSB = (vddr & 0x00FF);				//���ͼĴ�����ַ���ֽ�
	u8 vddr_MSB = (vddr & 0xFF00) >> 8;		//���ͼĴ�����ַ���ֽ�
	
	SMBus_Start();									//������ʼ�ź�
	if(SMBus_Send_data(addr))				//���ʹ�������ַ + ����Ӧ��
	{	
		return 1;
	}												
	if(SMBus_Send_data(0x44))				//�����ֽڵ�ַ + ����Ӧ��
	{
		return 2;
	}
	if(SMBus_Send_data(0x02))				//�����ֽڵ�ַ + ����Ӧ��
	{
		return 3;
	}
	if(SMBus_Send_data(vddr_LSB))				//�����ֽڵ�ַ + ����Ӧ��
	{
		return 4;
	}
	if(SMBus_Send_data(vddr_MSB))				//�����ֽڵ�ַ + ����Ӧ��
	{
		return 5;
	}												
	SMBus_Stop();										//���ͽ����ź�
	return 0;
}

/*************************************************
�������ܣ�BQ78350��д
�����βΣ�addr:������ַ   vddr���洢���ĵ�ַ	data������	size�����ݴ�С(�ֽ�)
��������ֵ��0��������1������
������ע��
8���ֽ�Ϊһҳ��addr = 0xa0��
���ò��裺
������ʼ�ź�
���ʹ�������ַ + ����Ӧ��
�����ֽڵ�ַ + ����Ӧ��
�������� + ����Ӧ��
��ַƫ��
���ͽ����ź�
************************************************/
u8 BQ78350_Block_Writedata(u8 addr,u16 vddr,signed int data,u8 size)
{
	/* ���ͼĴ�����ַ��Ϣ */
	u8 vddr_LSB = (vddr & 0x00FF);				//���ͼĴ�����ַ���ֽ�
	u8 vddr_MSB = (vddr & 0xFF00) >> 8;		//���ͼĴ�����ַ���ֽ�
	/* ���Ͷ�ȡ��Ϣ */
	u8 data_LSB = (data & 0x00FF);
	u8 data_MSB = (data & 0xFF00) >> 8;
	
	SMBus_Start();									//������ʼ�ź�
	if(SMBus_Send_data(addr))				//���ʹ�������ַ + ����Ӧ��
	{	
		return 1;
	}												
	if(SMBus_Send_data(0x44))				//�����ֽڵ�ַ + ����Ӧ��
	{
		return 2;
	}
	if(SMBus_Send_data(size + 2))				//�����ֽڵ�ַ + ����Ӧ��
	{
		return 3;
	}
	if(SMBus_Send_data(vddr_LSB))				//�����ֽڵ�ַ + ����Ӧ��
	{
		return 4;
	}
	if(SMBus_Send_data(vddr_MSB))				//�����ֽڵ�ַ + ����Ӧ��
	{
		return 5;
	}		
	if(size == 2)
	{
		if(SMBus_Send_data(data_MSB))				//�����ֽڵ�ַ + ����Ӧ��
		{
			return 6;
		}
	}
	if(SMBus_Send_data(data_LSB))				//�����ֽڵ�ַ + ����Ӧ��
	{
		return 7;
	}		
	SMBus_Stop();										//���ͽ����ź�
	return 0;
}

/*************************************************
�������ܣ�BQ78350���
�����βΣ�addr:������ַ   vddr����ȡ���׵�ַ	data����������	size�����ݴ�С(�ֽ�)
��������ֵ����
������ע��
��Ҫ����Ӧ���źžͻ���������ֱ�����ͷ�Ӧ��	
���ò��裺
������ʼ�ź�
���ʹ�������ַ + ����Ӧ��	addr = 0xa0��
�����ֽڵ�ַ + ����Ӧ��
������ʼ�ź�
���ʹ�������ַ + ����Ӧ��	addr = 0xa1��
�������� + ����Ӧ��
��ַƫ��
���ͷ�Ӧ��
���ͽ����ź�
************************************************/
u8 BQ78350_Block_Read(u8 addr,u8* data,u8 size)
{
	u8 i;
	
	SMBus_Start();										//������ʼ�ź�
	if(SMBus_Send_data(addr))					//���ʹ�������ַ + ����Ӧ��	addr = 0xa0��
	{	
		return 6;
	}												
	if(SMBus_Send_data(0x44))					//�����ֽڵ�ַ + ����Ӧ��
	{
		return 7;
	}
	SMBus_Start();										//������ʼ�ź�
	if(SMBus_Send_data(addr | 0x01))	//���ʹ�������ַ + ����Ӧ��	addr = 0xa1��
	{	
		return 8;
	}		
	for(i = 0; i < size; i++)										//�������� + ����Ӧ��
	{
		if(i == size - 1)
		{	
			*data++ = SMBus_Read_data(NOACK);	//���ͷ�Ӧ��
		}
		else 
		{
			*data++ = SMBus_Read_data(ACK);
		}			
	}								
	SMBus_Stop();											//���ͽ����ź�
	return 0;
}

/*************************************************
�������ܣ�BQ78350���
�����βΣ�addr:������ַ   vddr����ȡ���׵�ַ	data����������	size�����ݴ�С(�ֽ�)
��������ֵ����
������ע��
��Ҫ����Ӧ���źžͻ���������ֱ�����ͷ�Ӧ��	
���ò��裺
������ʼ�ź�
���ʹ�������ַ + ����Ӧ��	addr = 0xa0��
�����ֽڵ�ַ + ����Ӧ��
������ʼ�ź�
���ʹ�������ַ + ����Ӧ��	addr = 0xa1��
�������� + ����Ӧ��
��ַƫ��
���ͷ�Ӧ��
���ͽ����ź�
************************************************/
u8 BQ78350_SBS_Read(u8 addr,u8* data,u8 size)
{
	u8 i;
	
	SMBus_Start();										//������ʼ�ź�
	if(SMBus_Send_data(addr))					//���ʹ�������ַ + ����Ӧ��	addr = 0xa0��
	{	
		return 6;
	}												
	if(SMBus_Send_data(0x23))					//�����ֽڵ�ַ + ����Ӧ��
	{
		return 7;
	}
	SMBus_Start();										//������ʼ�ź�
	if(SMBus_Send_data(addr | 0x01))	//���ʹ�������ַ + ����Ӧ��	addr = 0xa1��
	{	
		return 8;
	}		
	for(i = 0; i < size; i++)										//�������� + ����Ӧ��
	{
		if(i == size - 1)
		{	
			*data++ = SMBus_Read_data(NOACK);	//���ͷ�Ӧ��
		}
		else 
		{
			*data++ = SMBus_Read_data(ACK);
		}			
	}								
	SMBus_Stop();											//���ͽ����ź�
	return 0;
}
