#include "STC8A4K60S20A12.h"
#include "intrins.h"
#include "string.h"
#include "stdio.h"
/*********************************************************
*程序功能:  实现ADC高速采集，过压过流关断电路
*MCU：STC8A
*内部晶振频率：24MHZ
*实现要求：根据ADC采集数据控制数字电路完成过压过流关断功能，
*          从采样开始到引脚电平发生变化控制在5个us。
*著作人：李孟龙
*版本：v1.0
*时间：2019.2.15 15 ：22
*********************************************************/
#define ADC_CHANNEL0         0x00            //转换通道P1.0
#define ADC_CHANNEL1         0x01            //转换通道P1.1

#define WT_24M    0x81

#define FOSC  24000000UL
#define BRT  (65536-FOSC/115200/4)

unsigned int Time_Count = 0;  //单次1ms计数
unsigned int  temp = 0;       //用来转换串口输入数值

#define RxLength		32		//接收缓冲长度
unsigned char	Rx_Cnt;				//接收到的字节数, 用户处理数据后必须清0
unsigned char RxBuf[RxLength] = 0;	//接收缓冲
bit	B_Rx_OK = 0;	 		//接收完的标志位, 收到数据块系统设置1, 用户处理数据后必须清0

/* EEPROM 读取/写入变量*/
unsigned int  VEDI = 0x0A7F;    //电流放电保护阈值 2687
unsigned int  VPTH = 3;      //串口输入ADC采集次数
unsigned int  DELY = 800;    //过流之后，恢复采集，延时时间

bit KEY0_P32_Down = 0;  //电流放电保护阈值事件标志位
bit KEY1_P33_Down = 0;  //电流放电保护阈值事件标志位
bit KEY2_P34_Down = 0;  //ADC采集次数事件标志位
bit KEY3_P35_Down = 0;  //ADC采集次数事件标志位
bit KEY4_P36_Down = 0;  //延迟保护事件标志位
bit KEY5_P37_Down = 0;  //延迟保护事件标志位


unsigned int EEPROM_Read_value_VEDI = 0; 
unsigned int EEPROM_Read_value_VEDI_H = 0; 
unsigned int EEPROM_Read_value_VEDI_L = 0; 
unsigned int EEPROM_Read_value_VPTH = 0;
unsigned int EEPROM_Read_value_DELY = 0;
unsigned int EEPROM_Read_value_DELY_H = 0; 
unsigned int EEPROM_Read_value_DELY_L = 0; 


/*********************************************************
*程序功能:  外部时钟源选择
*外部晶振频率：24MHZ
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.15 20 ：35
*********************************************************/
void Clock_Source_Selection()
{   
  P_SW2 = 0x80;             //下面特殊功能寄存器为扩展sfr，逻辑地址位于XDATA区域，访问前须将P_SW2（BAH）寄存器最高位（EAXFR）置1
#if 0
  XOSCCR = 0xc0;            //启动外部晶振振荡器  信号源连接P1^7 P1^6  
  while(!(XOSCCR &1));      //等待时钟稳定
  CLKDIV = 0x00;            //时钟1分频即不分频
  CKSEL = 0x01;             //选择外部晶振32.768MHZ
#endif
 
  CKSEL = 0x00;             //选择内部晶振24MHZ
  P_SW2 = 0x00;
}
/*************************************************************************
函数名：UART串口发送函数
调  用：UART_T (?);
参  数：需要UART串口发送的数据（8位/1字节）
返回值：无 
结  果：将参数中的数据发送给UART串口，确认发送完成后退出
备  注：
/****************************************************************************/
void UART_T (unsigned char UART_Data)
{ 
  ES = 0;
	SBUF = UART_Data;	//定义串口发送数据变量,将接收的数据发送回去
	while(TI == 0);		//检查发送中断标志位
	TI = 0;			//令发送中断标志位为0（软件清零）
  ES = 1; 
}
/*************************************************************************
函数名：char putchar (char c)   
调  用：由Printf（）调用
参  数：
返回值：
结  果：
备  注：unsigned char mm = 0xAA;printf("----:0x%02X\n",mm);后得出的结却是：----:0xAA05；
        unsigned int 所以 我们打印的时候回多出一个字节所以以后就这样定义unsigned int mm = 0xAA;
        printf("----:0x%02X\n",mm & 0x00FF);打印出的结果就是后得出的结却是：----:0x00AA；
/****************************************************************************/
char putchar (char c)   
{        
    ES=0;        
    SBUF = c;        
    while(TI==0);        
    TI=0;        
    ES=1;        
    return c;
}
/*****************************************************************************
函数名：UART串口发送字符串函数
调  用：UART_TC (?);
参  数：需要UART串口发送的数据（8位/1字节）
返回值：无 
结  果：向串口发送一个字符串,长度不限。
备  注：例：UART_TC("d9887321$"); 此函数需要#include <string.h>头文件支持。
/****************************************************************************/
void Uart1_SendStr (unsigned char *str)
{
	while(*str != '\0')
  {
		UART_T(*str++);
	}
	*str = 0;
}

/**************************************************************************
函数名：UART串口初始化函数
调  用：UART_init();
*外部晶振频率：24MHZ
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.16 10 ：00
*备注：串口1是用定时器1（模式0）做波特率发生器
/****************************************************************************/
void Uart1_Init (void){
	EA = 1; //允许总中断（如不使用中断，可用//屏蔽）
	ES = 1; //允许UART串口的中断

	TMOD = 0x00;	//定时器T1/C 工作方式0
	SCON = 0x50;	//串口工作方式1:可变波特率8位数据方式，允许串口接收（SCON = 0x40 时禁止串口接收）
	TH1 = BRT>>8;	//定时器初值高8位设置
	TL1 = BRT;	//定时器初值低8位设置
	TR1 = 1;	//定时器启动    
  AUXR=0x40;
}
/*********************************************************
*程序功能: 定时器0中断初始化
*外部晶振频率：24MHZ
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.16 10 ：00
备注：定时器0定时1ms  优先级设置为较高级，低于外部中断0 1 
*********************************************************/
void TIM0_Init(void)
{
  AUXR |= 0x80;		            //定时器时钟1T模式
  TMOD &= 0xF0;               //模式0 16位自动重载
  TL0=0x40;                  //定时1ms  65536-24000000/12/1000
  TH0=0xA2;
  TR0=0;                    //暂不开启定时器0
  ET0=1;                    //使能定时器
  EA =1;
}

/*********************************************************
*程序功能: 外部中断0
*外部晶振频率：24MHZ
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.19 10 ：00
备注：中断引脚P3^2  下降沿触发  设置优先级为最高
*********************************************************/
void INT0_Init(void)
{
  IT0 = 1;     //使能INT0下降沿中断
  EX0 = 1;      //使能INT0中断
  EA  = 1;     //开总中断
}

/*********************************************************
*程序功能: 外部中断1
*外部晶振频率：24MHZ
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.19 10 ：00
备注：中断引脚P3^3  下降沿触发  设置优先级为最高  次序低于INT0
*********************************************************/
void INT1_Init(void)
{
  IT1 = 1;     //使能INT1下降沿中断
  EX1 = 1;      //使能INT1中断
  EA  = 1;     //开总中断 
}

/*********************************************************
*程序功能: 端口初始化
*外部晶振频率：24MHZ
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.16 10 ：00
*********************************************************/
void GPIO_Init(void)
{
    P3M0 = 0x02;  //设置P3^1 TXD 强推挽输出,设置P3^2 P3^3为准双向口
    P3M1 = 0x00;
    P0M0 = 0x83;  //设置P0^0 P0^1 P0^7 强推挽输出
    P0M1 = 0x00;
    P00 = 1;                  //过压控制通断引脚  初始为高
    P01 = 1;                  //过流控制通断引脚  初始为高
    P07 = 1;                  //LED指示灯 低亮
}


/*********************************************************
*程序功能:  ADC初始化
*外部晶振频率：32.768MHZ
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.15 20 ：35
*********************************************************/
void ADC_Init(void)
{
  P1M0 = 0x00;              //设置P1^0 为ADC 电流检测口
  P1M1 = 0x03;              //设置P1^1 为ADC 电压检测口 
  ADCCFG = 0x20;            //设置ADC时钟为系统时钟/2/1   转换结果右对齐 ADC_RES保存高4位
  ADC_CONTR |= 0x80;         //使能ADC模块 
}
/*********************************************************
*程序功能:  获取ADC数据
*参数说明： ADC_CHANNEL ADC通道（0-15）
*外部晶振频率：24MHZ
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.15 20 ：35
*********************************************************/
unsigned int ADC_GetResult(unsigned int ADC_CHANNEL)
{
  unsigned int Result = 0;
  ADC_RES = 0;
  ADC_RESL = 0;
  ADC_CONTR|= ADC_CHANNEL;
  ADC_CONTR |= 0x40;        //启动AD转换 
  _nop_();
  while(!(ADC_CONTR & 0X20));  //查询ADC完成标志
  ADC_CONTR &= ~0x20;          //清完成标志
  Result = ADC_RES;
  Result = (Result<<8|ADC_RESL);                //读取ADC高4位结果 
  ADC_CONTR &= 0xF0;          //清除上一个通道 
  return Result;
}

/*********************************************************
*程序功能:  中位值平均滤波法
*著作人：李孟龙
*版本：v1.0
*时间：2019.7.27 17 ：35
*********************************************************/
unsigned int ADC_Result_Filter(unsigned int ADC_CHANNEL)
{
  char count,i,j;
  char temp,sum=0;
  unsigned int VPTH_Value = VPTH;
  unsigned int value_buf[20];
  for(count=0;count<VPTH_Value;count++)
  {
     value_buf[count] = ADC_GetResult(ADC_CHANNEL); 
  }
  for(j=0;j<VPTH_Value-1;j++)
  {
    for(i=0;i<VPTH_Value-j;i++)
    {
      if(value_buf[i] > value_buf[i+1])  //冒泡算法
      {
         temp = value_buf[i];
         value_buf[i] = value_buf[i+1];
         value_buf[i+1] = temp;
      }
    }
  }
  for(count=1;count<VPTH_Value-1;count++)  //去掉最大值与最小值
      sum += value_buf[count];
  return (sum/(VPTH_Value-2));
}

/*********************************************************
*程序功能:  过流控制函数
*参数说明： 
*外部晶振频率：24MHZ
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.24 18 ：02
*********************************************************/
void Overcurrent_Control(void)
{
    unsigned int Voltage0=0;
    unsigned char i = 0;
    Voltage0 = ADC_GetResult(ADC_CHANNEL0);  
    printf("---- Voltage0 = :0x%04X\r\n",Voltage0);
    if(Voltage0 > VEDI)       //过流  1.64V   可能毛刺
    //if(Voltage0 > 0x0A)       //过流  1.64V
    {   
        Voltage0 = ADC_Result_Filter(ADC_CHANNEL0);   //进行滤波分析
        if(Voltage0 > VEDI)      //真实过流
        {       
            P00 = 0;
            P01 = 0; 
            TR0 = 1; //开启定时器
            printf("Beyond the standard here\r\n");        
        }
        Voltage0 = 0;
    }    
}
/*********************************************************
*程序功能:  过压控制函数
*参数说明： 
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.24 18 ：02
*********************************************************/
void Overvoltage_Control() 
{
    unsigned int Voltage1=0;
    unsigned char i = 0;       
    Voltage1 = ADC_GetResult(ADC_CHANNEL1);              
   //printf("---- Voltage1 = :0x%04X\r\n",Voltage1);
   if(Voltage1 > 0x0E14)      //过压 2.2V   可能是毛刺
   //if(Voltage1 > 0x0E)      //过压 2.2V
    {
      Voltage1 = ADC_Result_Filter(ADC_CHANNEL1); //进行滤波分析
      if(Voltage1 > 0x0E14)   //真实过压
      {
         P00 = 0;
         P01 = 0;      
         TR0 = 1; //开启定时器  
         printf("Beyond the standard here\r\n");
      }
      Voltage1 = 0;
    }   
}

/**************************************************************************
函数名：关闭IAP功能
调  用：IAP_Read();
*内部晶振频率：24MHZ
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.16 10 ：00
*备注：
/****************************************************************************/
void IAP_Idle()
{
  IAP_CONTR = 0;     //关闭IAP功能
  IAP_CMD = 0;       //清除命令寄存器
  IAP_TRIG = 0;      //清除触发寄存器
  IAP_ADDRH = 0X80;   //将地址设置到非IAP区域
  IAP_ADDRL = 0;
}

/**************************************************************************
函数名：EEPROM 读取函数
调  用：
*外部晶振频率：24MHZ
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.16 10 ：00
*备注：
/****************************************************************************/
char IAP_Read(int addr)
{
  char dat;
  
  IAP_CONTR = WT_24M;    //使能IAP
  IAP_CMD = 1;           //设置IAP读命令
  IAP_ADDRL = addr;      //设置IAP低地址
  IAP_ADDRH = addr>>8;   //设置IAP高地址
  IAP_TRIG = 0x5a;       //写触发命令(0x5a)
  IAP_TRIG = 0xa5;       //写触发命令(0xa5)
  _nop_();
  dat = IAP_DATA;        //读IAP数据
  IAP_Idle();            //关闭IAP功能
  
  return dat;
}

/**************************************************************************
函数名：EEPROM 写入函数
*外部晶振频率：24MHZ  不分频
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.16 10 ：00
*备注：
/****************************************************************************/
void IAP_Program(int addr, char dat)
{
  IAP_CONTR = WT_24M;    //使能IAP
  IAP_CMD = 2;           //设置IAP写命令
  IAP_ADDRL = addr;      //设置IAP低地址
  IAP_ADDRH = addr>>8;   //设置IAP高地址
  IAP_DATA = dat;        //写IAP数据    
  IAP_TRIG = 0x5a;       //写触发命令(0x5a)
  IAP_TRIG = 0xa5;       //写触发命令(0xa5)
  _nop_(); 
  IAP_Idle();            //关闭IAP功能
}

/**************************************************************************
函数名：EEPROM 擦除函数
*外部晶振频率：24MHZ
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.16 10 ：00
*备注：串口1是用定时器1（模式0）做波特率发生器
/****************************************************************************/
void IAP_Erase(int addr)
{
  IAP_CONTR = WT_24M;    //使能IAP
  IAP_CMD = 3;           //设置IAP写命令
  IAP_ADDRL = addr;      //设置IAP低地址
  IAP_ADDRH = addr>>8;   //设置IAP高地址
  IAP_TRIG = 0x5a;       //写触发命令(0x5a)
  IAP_TRIG = 0xa5;       //写触发命令(0xa5)
  _nop_(); 
  IAP_Idle();            //关闭IAP功能
}
/**************************************************************************
*函数名：EEPROM_Init()
*参数：
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.16 10 ：00
*备注：EEPROM 首先判断数据是否为空，判断是不是第一次写入
/**************************************************************************/
void EEPROM_Init()
{
//    IAP_Erase(0x0000);
//    IAP_Erase(0x0200);
//    IAP_Erase(0x0400);
//    IAP_Erase(0x0600);
    EEPROM_Read_value_VEDI_H = IAP_Read(0x0000);
    EEPROM_Read_value_VEDI_L = IAP_Read(0x0200);  
    EEPROM_Read_value_VPTH   = IAP_Read(0x0400);
    EEPROM_Read_value_DELY_H = IAP_Read(0x0600);
    EEPROM_Read_value_DELY_L = IAP_Read(0x0800);

   if((EEPROM_Read_value_VEDI_H == 0xFFFF) || (EEPROM_Read_value_VPTH == 0xFFFF)|| (EEPROM_Read_value_DELY_H == 0xFFFF))
    {   
       IAP_Erase(0x0000);
       IAP_Program(0x0000,0x0A);
       EEPROM_Read_value_VEDI_H = IAP_Read(0x0000);
      
       IAP_Erase(0x0200);
       IAP_Program(0x0200,0x7F);
       EEPROM_Read_value_VEDI_L = IAP_Read(0x0200);

       IAP_Erase(0x0400);
       IAP_Program(0x0400,3);
       EEPROM_Read_value_VPTH = IAP_Read(0x0400);

       IAP_Erase(0x0600);        
       IAP_Program(0x0600,0x03);  
       EEPROM_Read_value_DELY_H = IAP_Read(0x0600);
      
       IAP_Erase(0x0800);        
       IAP_Program(0x0800,0x20);  
       EEPROM_Read_value_DELY_L = IAP_Read(0x0800);
      
       printf("Wirted VEDI = %d %d %d %d %d\r\n",EEPROM_Read_value_VEDI_H,EEPROM_Read_value_VEDI_L,EEPROM_Read_value_VPTH,EEPROM_Read_value_DELY_H,EEPROM_Read_value_DELY_L);     
       printf("first write here\r\n");
       EEPROM_Read_value_VEDI_H=0;
       EEPROM_Read_value_VEDI_L=0;
       EEPROM_Read_value_VPTH = 0;
       EEPROM_Read_value_DELY_H=0;
       EEPROM_Read_value_DELY_L=0;      
    }
    else
    {
      EEPROM_Read_value_VEDI = EEPROM_Read_value_VEDI | EEPROM_Read_value_VEDI_H;
      VEDI = (EEPROM_Read_value_VEDI<<8) | (EEPROM_Read_value_VEDI_L &0x00FF);
      
      VPTH = EEPROM_Read_value_VPTH;
      
      EEPROM_Read_value_DELY = EEPROM_Read_value_DELY | EEPROM_Read_value_DELY_H;
      DELY = (EEPROM_Read_value_DELY<<8) | (EEPROM_Read_value_DELY_L &0x00FF); 
      
      printf("courrt VEDI = 0x%04X\r\n",VEDI); 
      printf("courrt VPTH = %d\r\n",VPTH);      
      printf("courrt Delay_count = %d\r\n",DELY);
      EEPROM_Read_value_VEDI_H=0;
      EEPROM_Read_value_VEDI_L=0;
      EEPROM_Read_value_VPTH = 0;
      EEPROM_Read_value_DELY_H=0;
      EEPROM_Read_value_DELY_L=0;
    }
}

/**************************************************************************
*函数名：EEPROM_Program()
*参数：
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.24 10 ：00
*备注：EEPROM写入之前 要先擦除
/****************************************************************************/
void EEPROM_Program(void)
{
   if(KEY0_P32_Down || KEY1_P33_Down || KEY2_P34_Down || KEY3_P35_Down|| KEY4_P36_Down || KEY5_P37_Down)
   {
     if(KEY0_P32_Down)
     {
        KEY0_P32_Down = 0;
        VEDI = VEDI + temp;   
        IAP_Erase(0x0000);
        IAP_Program(0x0000,VEDI>>8); 
        EEPROM_Read_value_VEDI_H = IAP_Read(0x0000);
         
        IAP_Erase(0x0200);       
        IAP_Program(0x0200,VEDI);  
        EEPROM_Read_value_VEDI_L = IAP_Read(0x0200);
       
        EEPROM_Read_value_VEDI =  EEPROM_Read_value_VEDI_H;
        EEPROM_Read_value_VEDI = (EEPROM_Read_value_VEDI<<8) | (EEPROM_Read_value_VEDI_L & 0x00FF);    
        printf("courrt VEDI = 0x%04X\r\n",VEDI);              
        printf("courrt EEPROM_Read_value_VEDI = 0x%04X\r\n",EEPROM_Read_value_VEDI); 
        EEPROM_Read_value_VEDI_H=0;
        EEPROM_Read_value_VEDI_L=0;
        EEPROM_Read_value_VEDI = 0;
        temp = 0;
     }
     else if (KEY1_P33_Down)
     {
        KEY1_P33_Down = 0;
        VEDI = VEDI - temp;
        IAP_Erase(0x0000);
        IAP_Program(0x0000,VEDI>>8); 
        EEPROM_Read_value_VEDI_H = IAP_Read(0x0000);
         
        IAP_Erase(0x0200);       
        IAP_Program(0x0200,VEDI);  
        EEPROM_Read_value_VEDI_L = IAP_Read(0x0200);
       
        EEPROM_Read_value_VEDI = EEPROM_Read_value_VEDI_H;
        EEPROM_Read_value_VEDI = (EEPROM_Read_value_VEDI<<8) | (EEPROM_Read_value_VEDI_L & 0x00FF);
        printf("courrt VEDI = 0x%04X\r\n",VEDI);              
        printf("courrt EEPROM_Read_value_VEDI = 0x%04X\r\n",EEPROM_Read_value_VEDI);
        EEPROM_Read_value_VEDI_H=0;
        EEPROM_Read_value_VEDI_L=0;
        EEPROM_Read_value_VEDI = 0;
        temp = 0;       
     }
     else if(KEY2_P34_Down)
     {
        KEY2_P34_Down = 0;
        VPTH = VPTH + temp;
        IAP_Erase(0x0400);
        IAP_Program(0x0400,VPTH);  
        EEPROM_Read_value_VPTH = IAP_Read(0x0400);
        printf("courrt VPTH = %d\r\n",VPTH);
        printf("courrt EEPROM_Read_value_VPTH = %d\r\n",EEPROM_Read_value_VPTH);
        EEPROM_Read_value_VPTH =0;
        temp = 0;
     }
     else if(KEY3_P35_Down)
     {
        KEY3_P35_Down = 0;
        VPTH = VPTH - temp;
        IAP_Erase(0x0400);         
        IAP_Program(0x0400,VPTH);  
        EEPROM_Read_value_VPTH = IAP_Read(0x0400);
        printf("VPTH = %d\r\n",VPTH);
        printf("EEPROM_Read_value_VPTH = %d\r\n",EEPROM_Read_value_VPTH);
        EEPROM_Read_value_VPTH =0;
        temp = 0;
     }
     else if(KEY4_P36_Down)    
     {
        KEY4_P36_Down = 0;
        DELY = DELY + temp;  
        IAP_Erase(0x0600);
        IAP_Program(0x0600,DELY>>8); 
        EEPROM_Read_value_DELY_H = IAP_Read(0x0600);
         
        IAP_Erase(0x0800);       
        IAP_Program(0x0800,DELY);  
        EEPROM_Read_value_DELY_L = IAP_Read(0x0800);
       
        EEPROM_Read_value_DELY =  EEPROM_Read_value_DELY_H;
        EEPROM_Read_value_DELY = (EEPROM_Read_value_DELY<<8) | (EEPROM_Read_value_DELY_L & 0x00FF);    
        printf("courrt VEDI = %d\r\n",DELY);              
        printf("courrt EEPROM_Read_value_VEDI = %d\r\n",EEPROM_Read_value_DELY); 
        EEPROM_Read_value_DELY_H=0;
        EEPROM_Read_value_DELY_L=0;
        EEPROM_Read_value_DELY = 0;
        temp = 0;
     }
     else if(KEY5_P37_Down)
     {
        KEY5_P37_Down = 0;
        DELY = DELY - temp;  
        IAP_Erase(0x0600);
        IAP_Program(0x0600,DELY>>8); 
        EEPROM_Read_value_DELY_H = IAP_Read(0x0600);
         
        IAP_Erase(0x0800);       
        IAP_Program(0x0800,DELY);  
        EEPROM_Read_value_DELY_L = IAP_Read(0x0800);
       
        EEPROM_Read_value_DELY =  EEPROM_Read_value_DELY_H;
        EEPROM_Read_value_DELY = (EEPROM_Read_value_DELY<<8) | (EEPROM_Read_value_DELY_L & 0x00FF);    
        printf("courrt VEDI = %d\r\n",DELY);              
        printf("courrt EEPROM_Read_value_VEDI = %d\r\n",EEPROM_Read_value_DELY); 
        EEPROM_Read_value_DELY_H=0;
        EEPROM_Read_value_DELY_L=0;
        EEPROM_Read_value_DELY = 0;
        temp = 0;
     }
   }
}

/**************************************************************************
*函数名：将字符"0~9,A~F或a~f"转成十六进制
*参数：dat: 要检测的字符
*返回: 0x00~0x0F为正确. 0xFF为错误.
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.25 18 ：00
*备注：
/****************************************************************************/
unsigned char	CheckData(unsigned char dat)
{
	if((dat >= '0') && (dat <= '9'))		return (dat-'0');
	if((dat >= 'A') && (dat <= 'F'))		return (dat-'A'+10);
	return 0xff;
}

/**************************************************************************
*函数名：计算各种输入方式的地址.
*参数：无
*返回：16位EEPROM地址.
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.25 19 ：40
*备注：
/****************************************************************************/
unsigned int	GetAddress(void)
{
	unsigned int	address;
	unsigned char	i,j;
	
	address = 0;
	if((RxBuf[2] == '0') && (RxBuf[3] == 'X'))
	{
		for(i=4; i<8; i++)
		{
			j = CheckData(RxBuf[i]);
			if(j >= 0x10)	return 65535;	//error
			address = (address << 4) + j;
		}
		return (address);
	}
	return	65535;	//error
}

/*********************************************************
*程序功能: 串口输入数据处理
*参数说明： 
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.25 15 ：31
*********************************************************/
void Serial_InputData_Processing ()
{
  int i = 0;
  if(B_Rx_OK)        //接收完的标志位, 收到数据块系统设置1, 用户处理数据后必须清0
  {
      if(Rx_Cnt > 0)	//确认有数据
      {
          //Uart1_SendStr(RxBuf);		//把收到的数据原样返回,用于测试 
          printf("%s\r\n",RxBuf);        
          if((Rx_Cnt >= 6) && (RxBuf[1] == ' '))	//最短命令为6个字节
          {
              for(i=0; i<6; i++)
              {
                if((RxBuf[i] >= 'a') && (RxBuf[i] <= 'z'))	
                  RxBuf[i] = RxBuf[i] - 'a' + 'A';	//小写转大写
              }            
              if(RxBuf[0] == 'R'&& RxBuf[1] == ' ')	//PC请求读取某一个阈值
              {
                if(RxBuf[2] == 'V'&& RxBuf[3] == 'E'&& RxBuf[4] == 'D'&& RxBuf[5] == 'I')
                {
                  EEPROM_Read_value_VEDI_H = IAP_Read(0x0000);
                  EEPROM_Read_value_VEDI_L = IAP_Read(0x0200);
                  EEPROM_Read_value_VEDI =EEPROM_Read_value_VEDI | EEPROM_Read_value_VEDI_H;
                  EEPROM_Read_value_VEDI = (EEPROM_Read_value_VEDI<<8) |(EEPROM_Read_value_VEDI_L & 0x00FF);
                  printf("courrt EEPROM_Read_value_VEDI = 0x%04X\r\n",EEPROM_Read_value_VEDI);
                  EEPROM_Read_value_VEDI_H = 0;
                  EEPROM_Read_value_VEDI_L = 0;
                  EEPROM_Read_value_VEDI = 0;
                }
                else if(RxBuf[2] == 'V'&& RxBuf[3] == 'P'&& RxBuf[4] == 'T'&& RxBuf[5] == 'H')
                {
                  EEPROM_Read_value_VPTH = IAP_Read(0x0400);
                  printf("courrt EEPROM_Read_value_VPTH = %d\r\n",EEPROM_Read_value_VPTH);
                  EEPROM_Read_value_VPTH = 0;
                }
                else if(RxBuf[2] == 'D'&& RxBuf[3] == 'E'&& RxBuf[4] == 'L'&& RxBuf[5] == 'Y')
                {  
                  EEPROM_Read_value_DELY_H = IAP_Read(0x0600);
                  EEPROM_Read_value_DELY_L = IAP_Read(0x0800);
                  EEPROM_Read_value_DELY =  EEPROM_Read_value_DELY_H;
                  EEPROM_Read_value_DELY = (EEPROM_Read_value_DELY<<8) | (EEPROM_Read_value_DELY_L & 0x00FF);   
                  printf("courrt EEPROM_Read_value_VEDI = %d\r\n",EEPROM_Read_value_DELY); 
                  EEPROM_Read_value_DELY_H = 0;
                  EEPROM_Read_value_DELY_L = 0;
                  EEPROM_Read_value_DELY = 0;
                }
                else
                {
                  printf("Check if your spelling is wrong\r\n");
                }
              }
              else if((RxBuf[0] == 'W') && (Rx_Cnt >= 8) && (RxBuf[6] == ' '))	//写入1个字节
              {
                if(RxBuf[2] == 'V'&& RxBuf[3] == 'E'&& RxBuf[4] == 'D'&& RxBuf[5] == 'I'&& RxBuf[7] == '+')
                {                   
                    KEY0_P32_Down = 1;      
                    for(i = 8;i<Rx_Cnt;i++)
                    {
                        temp = RxBuf[i]-'0'+temp*10;
                    }
                }
                else if(RxBuf[2] == 'V'&& RxBuf[3] == 'E'&& RxBuf[4] == 'D'&& RxBuf[5] == 'I'&& RxBuf[7] == '-')
                {                 
                    KEY1_P33_Down = 1;    
                    for(i = 8;i<Rx_Cnt;i++)
                    {
                        temp = RxBuf[i]-'0'+temp*10;
                    }
                }
                
                else if(RxBuf[2] == 'V'&& RxBuf[3] == 'P'&& RxBuf[4] == 'T'&& RxBuf[5] == 'H'&& RxBuf[7] == '+')
                {                
                   KEY2_P34_Down = 1;
                   for(i = 8;i<Rx_Cnt;i++)
                    {
                        temp = RxBuf[i]-'0'+temp*10;
                    }
                  
                }
                else if(RxBuf[2] == 'V'&& RxBuf[3] == 'P'&& RxBuf[4] == 'T'&& RxBuf[5] == 'H'&& RxBuf[7] == '-')
                {                 
                   KEY3_P35_Down = 1;
                   for(i = 8;i<Rx_Cnt;i++)
                    {
                        temp = RxBuf[i]-'0'+temp*10;
                    }
                }
                else if(RxBuf[2] == 'D'&& RxBuf[3] == 'E'&& RxBuf[4] == 'L'&& RxBuf[5] == 'Y'&& RxBuf[7] == '+')
                {                
                   KEY4_P36_Down = 1;
                   for(i = 8;i<Rx_Cnt;i++)
                    {
                        temp = RxBuf[i]-'0'+temp*10;
                    }
                }
                else if(RxBuf[2] == 'D'&& RxBuf[3] == 'E'&& RxBuf[4] == 'L'&& RxBuf[5] == 'Y'&& RxBuf[7] == '-')
                {                 
                   KEY5_P37_Down = 1;
                   for(i = 8;i<Rx_Cnt;i++)
                    {
                        temp = RxBuf[i]-'0'+temp*10;
                    }
                }
                else 
                {
                   printf("Check if your spelling is wrong\r\n");
                }
                    
              }       
          }
      } 
      RxBuf[0] = '\0';
      memset(RxBuf,0,RxLength);
      Rx_Cnt  = 0;	//清除字节数
      B_Rx_OK = 0;	//清除接收完成标志
  }
}




/*********************************************************
*程序功能:  主函数
*参数说明： 
*内部晶振频率：24MHZ
*著作人：李孟龙
*版本：v2.0
*时间：2019.4.19 15 ：31
*********************************************************/
void main()
{
  Clock_Source_Selection();
  Uart1_Init();
  INT0_Init();
  INT1_Init();
  TIM0_Init();
  GPIO_Init();
  ADC_Init();
  EEPROM_Init();
  //WDT_CONTR = 0x27;         //使能软件看门狗 溢出时间约为3.2s 
  while(1)
  {
      //WDT_CONTR = 0x10;       //清软件看门狗       
      Overcurrent_Control();
      Overvoltage_Control();      
      EEPROM_Program();
      Serial_InputData_Processing();
   }
} 
  
void INT0_Isr() interrupt 0
{
    EX0 = 0;
    KEY0_P32_Down = 1;
    temp = 1;
    EX0 = 1;
}

void TIM0_Isr() interrupt 1   //定时器中断1次 1ms
{
  Time_Count++;
  if( Time_Count == DELY)      //定时DELY
  {
    Time_Count = 0;
    P00 = 1;
    P01 = 1;
    TR0 = 0;   //定时时间到关闭定时器
//    printf("End of delay here\r\n");
  }
}

void INT1_Isr() interrupt 2
{
  EX1 = 0;
  KEY1_P33_Down = 1;
  temp = 1;
  EX1 = 1;
}

/*******************************************************************
函数名：UART串口接收中断处理函数
调  用：[SBUF收到数据后中断处理]
参  数：无
返回值：无
结  果：UART串口接收到数据时产生中断，用户对数据进行处理（并发送回去）
备  注：过长的处理程序会影响后面数据的接收
/*****************************************************************/
void UART1_Tsr() interrupt 4  using 1  //切换寄存器组到1
{ 
    //unsigned char UART_data; //定义串口接收数据变量
   if(RI)
   {
      RI = 0;			//令接收中断标志位为0（软件清零）
      RxBuf[Rx_Cnt] = SBUF;     //存储数据到缓冲区	save the dat to RBUF
      if(++Rx_Cnt >= RxLength)	Rx_Cnt = 0;	//溢出判断
      B_Rx_OK = 1;	//标志已收到一帧数据
   } 
   if(TI)
   {
      TI = 0;
   }       
//    SBUF = RxBuf[Rx_Cnt];	//将接收的数据发送回去（删除//即生效）
//    while(TI == 0);	//检查发送中断标志位
//    TI = 0;		//令发送中断标志位为0（软件清零） 	
}	




