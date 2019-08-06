#include "bq78350.h"
#include "SMBus.h"
#include "Time_Delay.h"
/*************************************************
函数功能：BQ78350字节写设备类型
函数形参：addr:器件地址   vddr：存储到的地址	data：数据	size：数据大小
函数返回值：0：正常；1：错误
函数备注：
配置步骤：
发送起始信号
发送从器件地址 + 接收应答
发送字节地址 + 接收应答
发送数据 + 接收应答			低位在前
地址偏移
发送结束信号
************************************************/
u8 BQ78350_Write_Byte(u8 addr,u16 vddr,u16 type)
{
	u8 data_LSB = 0;	//发送数据低字节
	u8 data_MSB = 0;	//发送数据高字节
	
	u8 vddr_LSB = 0;	//发送寄存器地址低字节
	u8 vddr_MSB = 0;	//发送寄存器地址高字节
	
	/* 发送寄存器地址信息 */
	vddr_LSB = (vddr & 0x00FF);
	vddr_MSB = (vddr & 0xFF00) >> 8;
	
	/* 发送读取信息 */
	data_LSB = (type & 0x00FF);
	data_MSB = (type & 0xFF00) >> 8;
	
	SMBus_Start();									//发送起始信号
	if(SMBus_Send_data(addr))				//发送从器件地址 + 接收应答
	{	
		SMBus_Stop();	
		return 1;
	}			
	if(SMBus_Send_data(vddr_LSB))				//发送字节地址 + 接收应答
	{
		SMBus_Stop();	
		return 2;
	}
	if(SMBus_Send_data(vddr_MSB))				//发送字节地址 + 接收应答
	{
		SMBus_Stop();	
		return 3;
	}
	if(SMBus_Send_data(data_LSB))	//发送数据 + 接收应答													
	{			
		SMBus_Stop();			
		return 4;
	}		
	if(SMBus_Send_data(data_MSB))	//发送数据 + 接收应答													
	{			
		SMBus_Stop();			
		return 5;
	}		
	SMBus_Stop();										//发送结束信号
	return 0;
}

/*************************************************
函数功能：BQ78350读取 - 0X00发送命令
函数形参：addr:器件地址   vddr：存储到的地址	data：数据	size：数据大小
函数返回值：0：正常；1：错误
函数备注： 
8个字节为一页；addr = 0xa0；
配置步骤：
发送起始信号
发送从器件地址 + 接收应答
发送字节地址 + 接收应答
发送数据 + 接收应答			低位在前
地址偏移
发送结束信号
************************************************/
u8 BQ78350_SBS_CMD(u8 addr,u16 vddr,u8* data,u8 size)
{
	u8 error = 0;
	error = BQ78350_SBS_Write(addr, vddr);
	error = BQ78350_SBS_Read(addr, data, size);
	return error;
}

/*************************************************
函数功能：BQ78350读取设备类型
函数形参：addr:器件地址   vddr：存储到的地址	data：数据	size：数据大小
函数返回值：0：正常；1：错误
函数备注：
8个字节为一页；addr = 0xa0；
配置步骤：
发送起始信号
发送从器件地址 + 接收应答
发送字节地址 + 接收应答
发送数据 + 接收应答			低位在前
地址偏移
发送结束信号
************************************************/
u8 BQ78350_Block_CMD(u8 addr,u16 vddr,u8* data,u8 size)
{
	u8 error = 0;
	error = BQ78350_Block_Write(addr, vddr);
	error = BQ78350_Block_Read(addr, data, size);
	return error;
}

/*************************************************
函数功能：判断正负值
函数形参：data
函数返回值：value
函数备注：
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
函数功能：BQ78350读取设备类型
函数形参：addr:器件地址   vddr：存储到的地址	data：数据	size：数据大小
函数返回值：0：正常；1：错误
函数备注：
8个字节为一页；addr = 0xa0；
配置步骤：
发送起始信号
发送从器件地址 + 接收应答
发送字节地址 + 接收应答
发送数据 + 接收应答			低位在前
地址偏移
发送结束信号
************************************************/
u32 BQ78350_Read_DevType2(u8* R_data,u8 vddr)
{
	u8 i;
	
	/* 写信息  -  发送完数据后无需等待，因为接受应答处理函数已经等待应答，从器件应答说明已经准备好写一次写数据 */
	SMBus_Start();									//发送起始信号

	if(SMBus_Send_data(0x16))				//发送从器件地址 + 接收应答
	{	
		SMBus_Stop();	
		return 11;
	}		
	if(SMBus_Send_data(vddr))				//发送块R/W命令 + 接收应答
	{
		SMBus_Stop();	
		return 22;
	}
	SMBus_Start();									//发送起始信号
	if(SMBus_Send_data(0x17))				//发送从器件地址 + 接收应答
	{	
		SMBus_Stop();	
		return 99;
	}		
	for(i = 0; i < 3; i++)										//接收数据 + 发送应答
	{
		if(i == 2)
		{	
			*R_data++ = SMBus_Read_data(NOACK);	//发送非应答	
		}
		else 
		{
			*R_data++ = SMBus_Read_data(ACK);	
		}			
	}				
	SMBus_Stop();										//发送结束信号
	return 0;
}

/*************************************************
函数功能：BQ78350读取设备类型
函数形参：addr:器件地址   vddr：存储到的地址	data：数据	size：数据大小
函数返回值：0：正常；1：错误
函数备注：
8个字节为一页；addr = 0xa0；
配置步骤：
发送起始信号
发送从器件地址 + 接收应答
发送字节地址 + 接收应答
发送数据 + 接收应答			低位在前
地址偏移
发送结束信号
************************************************/
u32 BQ78350_Read_Byte(u8 addr,u16 vddr)
{
	u8 i;
	u32 data = 0;
	u16 temp = 0;//接收一个字节
	
	/* 发送寄存器地址信息 */
	u8 vddr_LSB = (vddr & 0x00FF);				//发送寄存器地址低字节
	
	/* 读取信息 */
	SMBus_Start();									//发送起始信号
	if(SMBus_Send_data(addr))				//发送从器件地址 + 接收应答
	{	
		SMBus_Stop();	
		return 1;
	}		
	if(SMBus_Send_data(vddr_LSB))				//发送字节地址 + 接收应答
	{
		SMBus_Stop();	
		return 2;
	}
	SMBus_Start();		//发送起始信号
	if(SMBus_Send_data(addr | 0x01))				//发送从器件地址 + 接收应答
	{	
		SMBus_Stop();	
		return 4;
	}		
	for(i = 0; i < 2; i++)										//接收数据 + 发送应答
	{
		if(i == 1)
		{	
			temp = SMBus_Read_data(NOACK);	//发送非应答
			temp <<= 8;	
		}
		else 
		{
			temp = SMBus_Read_data(ACK);	
		}			
		data |= temp;
	}								
	SMBus_Stop();										//发送结束信号
	return data;
}

/*************************************************
函数功能：BQ78350读取8位寄存器地址内数据
函数形参：addr:器件地址   vddr：存储到的地址	data：数据	size：数据大小
函数返回值：0：正常；1：错误
函数备注：
8个字节为一页；addr = 0xa0；
配置步骤：
发送起始信号
发送从器件地址 + 接收应答
发送字节地址 + 接收应答
发送数据 + 接收应答			低位在前
地址偏移
发送结束信号
************************************************/
u8 BQ78350_Read_buf(u8 addr,u16 vddr,u8* data,u8 size)
{
	u8 i;
	
	/* 发送寄存器地址信息 */
	u8 vddr_LSB = (vddr & 0xFF);				//发送寄存器地址低字节
	
	/* 读取信息 */
	SMBus_Start();									//发送起始信号
	if(SMBus_Send_data(addr))				//发送从器件地址 + 接收应答
	{	
		SMBus_Stop();	
		return 1;
	}		
	if(SMBus_Send_data(vddr_LSB))				//发送字节地址 + 接收应答
	{
		SMBus_Stop();	
		return 2;
	}
	SMBus_Start();		//发送起始信号
	if(SMBus_Send_data(addr | 0x01))				//发送从器件地址 + 接收应答
	{	
		SMBus_Stop();	
		return 4;
	}		
	for(i = 0; i < size; i++)										//接收数据 + 发送应答
	{
		if(i == size-1)
		{	
			*data++ = SMBus_Read_data(NOACK);	//发送非应答
		}
		else 
		{
			*data++ = SMBus_Read_data(ACK);	
		}			
	}								
	SMBus_Stop();										//发送结束信号
	return 0;
}

/*************************************************
函数功能：BQ78350块写
函数形参：addr:器件地址   vddr：存储到的地址	data：数据	size：数据大小(字节)
函数返回值：0：正常；1：错误
函数备注：
8个字节为一页；addr = 0xa0；
配置步骤：
发送起始信号
发送从器件地址 + 接收应答
发送字节地址 + 接收应答
发送数据 + 接收应答
地址偏移
发送结束信号
************************************************/
u8 BQ78350_SBS_Write(u8 addr,u16 vddr)
{
	/* 发送寄存器地址信息 */
	u8 vddr_LSB = (vddr & 0x00FF);				//发送寄存器地址低字节
	u8 vddr_MSB = (vddr & 0xFF00) >> 8;		//发送寄存器地址低字节
	
	SMBus_Start();									//发送起始信号
	if(SMBus_Send_data(addr))				//发送从器件地址 + 接收应答
	{	
		return 1;
	}												
	if(SMBus_Send_data(0x00))				//发送字节地址 + 接收应答
	{
		return 2;
	}
//	if(SMBus_Send_data(0x02))				//发送字节地址 + 接收应答
//	{
//		return 3;
//	}
	if(SMBus_Send_data(vddr_MSB))				//发送字节地址 + 接收应答
	{
		return 4;
	}
	if(SMBus_Send_data(vddr_LSB))				//发送字节地址 + 接收应答
	{
		return 5;
	}												
	SMBus_Stop();										//发送结束信号
	return 0;
}

/*************************************************
函数功能：BQ78350块写
函数形参：addr:器件地址   vddr：存储到的地址	data：数据	size：数据大小(字节)
函数返回值：0：正常；1：错误
函数备注：
8个字节为一页；addr = 0xa0；
配置步骤：
发送起始信号
发送从器件地址 + 接收应答
发送字节地址 + 接收应答
发送数据 + 接收应答
地址偏移
发送结束信号
************************************************/
u8 BQ78350_Block_Write(u8 addr,u16 vddr)
{
	/* 发送寄存器地址信息 */
	u8 vddr_LSB = (vddr & 0x00FF);				//发送寄存器地址低字节
	u8 vddr_MSB = (vddr & 0xFF00) >> 8;		//发送寄存器地址低字节
	
	SMBus_Start();									//发送起始信号
	if(SMBus_Send_data(addr))				//发送从器件地址 + 接收应答
	{	
		return 1;
	}												
	if(SMBus_Send_data(0x44))				//发送字节地址 + 接收应答
	{
		return 2;
	}
	if(SMBus_Send_data(0x02))				//发送字节地址 + 接收应答
	{
		return 3;
	}
	if(SMBus_Send_data(vddr_LSB))				//发送字节地址 + 接收应答
	{
		return 4;
	}
	if(SMBus_Send_data(vddr_MSB))				//发送字节地址 + 接收应答
	{
		return 5;
	}												
	SMBus_Stop();										//发送结束信号
	return 0;
}

/*************************************************
函数功能：BQ78350块写
函数形参：addr:器件地址   vddr：存储到的地址	data：数据	size：数据大小(字节)
函数返回值：0：正常；1：错误
函数备注：
8个字节为一页；addr = 0xa0；
配置步骤：
发送起始信号
发送从器件地址 + 接收应答
发送字节地址 + 接收应答
发送数据 + 接收应答
地址偏移
发送结束信号
************************************************/
u8 BQ78350_Block_Writedata(u8 addr,u16 vddr,signed int data,u8 size)
{
	/* 发送寄存器地址信息 */
	u8 vddr_LSB = (vddr & 0x00FF);				//发送寄存器地址低字节
	u8 vddr_MSB = (vddr & 0xFF00) >> 8;		//发送寄存器地址低字节
	/* 发送读取信息 */
	u8 data_LSB = (data & 0x00FF);
	u8 data_MSB = (data & 0xFF00) >> 8;
	
	SMBus_Start();									//发送起始信号
	if(SMBus_Send_data(addr))				//发送从器件地址 + 接收应答
	{	
		return 1;
	}												
	if(SMBus_Send_data(0x44))				//发送字节地址 + 接收应答
	{
		return 2;
	}
	if(SMBus_Send_data(size + 2))				//发送字节地址 + 接收应答
	{
		return 3;
	}
	if(SMBus_Send_data(vddr_LSB))				//发送字节地址 + 接收应答
	{
		return 4;
	}
	if(SMBus_Send_data(vddr_MSB))				//发送字节地址 + 接收应答
	{
		return 5;
	}		
	if(size == 2)
	{
		if(SMBus_Send_data(data_MSB))				//发送字节地址 + 接收应答
		{
			return 6;
		}
	}
	if(SMBus_Send_data(data_LSB))				//发送字节地址 + 接收应答
	{
		return 7;
	}		
	SMBus_Stop();										//发送结束信号
	return 0;
}

/*************************************************
函数功能：BQ78350块读
函数形参：addr:器件地址   vddr：读取的首地址	data：读到数据	size：数据大小(字节)
函数返回值：无
函数备注：
主要发送应答信号就会连续读，直到发送非应答；	
配置步骤：
发送起始信号
发送从器件地址 + 接收应答	addr = 0xa0；
发送字节地址 + 接收应答
发送起始信号
发送从器件地址 + 接收应答	addr = 0xa1；
接收数据 + 发送应答
地址偏移
发送非应答
发送结束信号
************************************************/
u8 BQ78350_Block_Read(u8 addr,u8* data,u8 size)
{
	u8 i;
	
	SMBus_Start();										//发送起始信号
	if(SMBus_Send_data(addr))					//发送从器件地址 + 接收应答	addr = 0xa0；
	{	
		return 6;
	}												
	if(SMBus_Send_data(0x44))					//发送字节地址 + 接收应答
	{
		return 7;
	}
	SMBus_Start();										//发送起始信号
	if(SMBus_Send_data(addr | 0x01))	//发送从器件地址 + 接收应答	addr = 0xa1；
	{	
		return 8;
	}		
	for(i = 0; i < size; i++)										//接收数据 + 发送应答
	{
		if(i == size - 1)
		{	
			*data++ = SMBus_Read_data(NOACK);	//发送非应答
		}
		else 
		{
			*data++ = SMBus_Read_data(ACK);
		}			
	}								
	SMBus_Stop();											//发送结束信号
	return 0;
}

/*************************************************
函数功能：BQ78350块读
函数形参：addr:器件地址   vddr：读取的首地址	data：读到数据	size：数据大小(字节)
函数返回值：无
函数备注：
主要发送应答信号就会连续读，直到发送非应答；	
配置步骤：
发送起始信号
发送从器件地址 + 接收应答	addr = 0xa0；
发送字节地址 + 接收应答
发送起始信号
发送从器件地址 + 接收应答	addr = 0xa1；
接收数据 + 发送应答
地址偏移
发送非应答
发送结束信号
************************************************/
u8 BQ78350_SBS_Read(u8 addr,u8* data,u8 size)
{
	u8 i;
	
	SMBus_Start();										//发送起始信号
	if(SMBus_Send_data(addr))					//发送从器件地址 + 接收应答	addr = 0xa0；
	{	
		return 6;
	}												
	if(SMBus_Send_data(0x23))					//发送字节地址 + 接收应答
	{
		return 7;
	}
	SMBus_Start();										//发送起始信号
	if(SMBus_Send_data(addr | 0x01))	//发送从器件地址 + 接收应答	addr = 0xa1；
	{	
		return 8;
	}		
	for(i = 0; i < size; i++)										//接收数据 + 发送应答
	{
		if(i == size - 1)
		{	
			*data++ = SMBus_Read_data(NOACK);	//发送非应答
		}
		else 
		{
			*data++ = SMBus_Read_data(ACK);
		}			
	}								
	SMBus_Stop();											//发送结束信号
	return 0;
}
