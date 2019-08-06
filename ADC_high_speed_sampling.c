#include "STC8A4K60S20A12.h"
#include "intrins.h"
#include "string.h"
#include "stdio.h"
/*********************************************************
*������:  ʵ��ADC���ٲɼ�����ѹ�����ضϵ�·
*MCU��STC8A
*�ڲ�����Ƶ�ʣ�24MHZ
*ʵ��Ҫ�󣺸���ADC�ɼ����ݿ������ֵ�·��ɹ�ѹ�����ضϹ��ܣ�
*          �Ӳ�����ʼ�����ŵ�ƽ�����仯������5��us��
*�����ˣ�������
*�汾��v1.0
*ʱ�䣺2019.2.15 15 ��22
*********************************************************/
#define ADC_CHANNEL0         0x00            //ת��ͨ��P1.0
#define ADC_CHANNEL1         0x01            //ת��ͨ��P1.1

#define WT_24M    0x81

#define FOSC  24000000UL
#define BRT  (65536-FOSC/115200/4)

unsigned int Time_Count = 0;  //����1ms����
unsigned int  temp = 0;       //����ת������������ֵ

#define RxLength		32		//���ջ��峤��
unsigned char	Rx_Cnt;				//���յ����ֽ���, �û��������ݺ������0
unsigned char RxBuf[RxLength] = 0;	//���ջ���
bit	B_Rx_OK = 0;	 		//������ı�־λ, �յ����ݿ�ϵͳ����1, �û��������ݺ������0

/* EEPROM ��ȡ/д�����*/
unsigned int  VEDI = 0x0A7F;    //�����ŵ籣����ֵ 2687
unsigned int  VPTH = 3;      //��������ADC�ɼ�����
unsigned int  DELY = 800;    //����֮�󣬻ָ��ɼ�����ʱʱ��

bit KEY0_P32_Down = 0;  //�����ŵ籣����ֵ�¼���־λ
bit KEY1_P33_Down = 0;  //�����ŵ籣����ֵ�¼���־λ
bit KEY2_P34_Down = 0;  //ADC�ɼ������¼���־λ
bit KEY3_P35_Down = 0;  //ADC�ɼ������¼���־λ
bit KEY4_P36_Down = 0;  //�ӳٱ����¼���־λ
bit KEY5_P37_Down = 0;  //�ӳٱ����¼���־λ


unsigned int EEPROM_Read_value_VEDI = 0; 
unsigned int EEPROM_Read_value_VEDI_H = 0; 
unsigned int EEPROM_Read_value_VEDI_L = 0; 
unsigned int EEPROM_Read_value_VPTH = 0;
unsigned int EEPROM_Read_value_DELY = 0;
unsigned int EEPROM_Read_value_DELY_H = 0; 
unsigned int EEPROM_Read_value_DELY_L = 0; 


/*********************************************************
*������:  �ⲿʱ��Դѡ��
*�ⲿ����Ƶ�ʣ�24MHZ
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.15 20 ��35
*********************************************************/
void Clock_Source_Selection()
{   
  P_SW2 = 0x80;             //�������⹦�ܼĴ���Ϊ��չsfr���߼���ַλ��XDATA���򣬷���ǰ�뽫P_SW2��BAH���Ĵ������λ��EAXFR����1
#if 0
  XOSCCR = 0xc0;            //�����ⲿ��������  �ź�Դ����P1^7 P1^6  
  while(!(XOSCCR &1));      //�ȴ�ʱ���ȶ�
  CLKDIV = 0x00;            //ʱ��1��Ƶ������Ƶ
  CKSEL = 0x01;             //ѡ���ⲿ����32.768MHZ
#endif
 
  CKSEL = 0x00;             //ѡ���ڲ�����24MHZ
  P_SW2 = 0x00;
}
/*************************************************************************
��������UART���ڷ��ͺ���
��  �ã�UART_T (?);
��  ������ҪUART���ڷ��͵����ݣ�8λ/1�ֽڣ�
����ֵ���� 
��  �����������е����ݷ��͸�UART���ڣ�ȷ�Ϸ�����ɺ��˳�
��  ע��
/****************************************************************************/
void UART_T (unsigned char UART_Data)
{ 
  ES = 0;
	SBUF = UART_Data;	//���崮�ڷ������ݱ���,�����յ����ݷ��ͻ�ȥ
	while(TI == 0);		//��鷢���жϱ�־λ
	TI = 0;			//����жϱ�־λΪ0��������㣩
  ES = 1; 
}
/*************************************************************************
��������char putchar (char c)   
��  �ã���Printf��������
��  ����
����ֵ��
��  ����
��  ע��unsigned char mm = 0xAA;printf("----:0x%02X\n",mm);��ó��Ľ�ȴ�ǣ�----:0xAA05��
        unsigned int ���� ���Ǵ�ӡ��ʱ��ض��һ���ֽ������Ժ����������unsigned int mm = 0xAA;
        printf("----:0x%02X\n",mm & 0x00FF);��ӡ���Ľ�����Ǻ�ó��Ľ�ȴ�ǣ�----:0x00AA��
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
��������UART���ڷ����ַ�������
��  �ã�UART_TC (?);
��  ������ҪUART���ڷ��͵����ݣ�8λ/1�ֽڣ�
����ֵ���� 
��  �����򴮿ڷ���һ���ַ���,���Ȳ��ޡ�
��  ע������UART_TC("d9887321$"); �˺�����Ҫ#include <string.h>ͷ�ļ�֧�֡�
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
��������UART���ڳ�ʼ������
��  �ã�UART_init();
*�ⲿ����Ƶ�ʣ�24MHZ
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.16 10 ��00
*��ע������1���ö�ʱ��1��ģʽ0���������ʷ�����
/****************************************************************************/
void Uart1_Init (void){
	EA = 1; //�������жϣ��粻ʹ���жϣ�����//���Σ�
	ES = 1; //����UART���ڵ��ж�

	TMOD = 0x00;	//��ʱ��T1/C ������ʽ0
	SCON = 0x50;	//���ڹ�����ʽ1:�ɱ䲨����8λ���ݷ�ʽ�������ڽ��գ�SCON = 0x40 ʱ��ֹ���ڽ��գ�
	TH1 = BRT>>8;	//��ʱ����ֵ��8λ����
	TL1 = BRT;	//��ʱ����ֵ��8λ����
	TR1 = 1;	//��ʱ������    
  AUXR=0x40;
}
/*********************************************************
*������: ��ʱ��0�жϳ�ʼ��
*�ⲿ����Ƶ�ʣ�24MHZ
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.16 10 ��00
��ע����ʱ��0��ʱ1ms  ���ȼ�����Ϊ�ϸ߼��������ⲿ�ж�0 1 
*********************************************************/
void TIM0_Init(void)
{
  AUXR |= 0x80;		            //��ʱ��ʱ��1Tģʽ
  TMOD &= 0xF0;               //ģʽ0 16λ�Զ�����
  TL0=0x40;                  //��ʱ1ms  65536-24000000/12/1000
  TH0=0xA2;
  TR0=0;                    //�ݲ�������ʱ��0
  ET0=1;                    //ʹ�ܶ�ʱ��
  EA =1;
}

/*********************************************************
*������: �ⲿ�ж�0
*�ⲿ����Ƶ�ʣ�24MHZ
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.19 10 ��00
��ע���ж�����P3^2  �½��ش���  �������ȼ�Ϊ���
*********************************************************/
void INT0_Init(void)
{
  IT0 = 1;     //ʹ��INT0�½����ж�
  EX0 = 1;      //ʹ��INT0�ж�
  EA  = 1;     //�����ж�
}

/*********************************************************
*������: �ⲿ�ж�1
*�ⲿ����Ƶ�ʣ�24MHZ
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.19 10 ��00
��ע���ж�����P3^3  �½��ش���  �������ȼ�Ϊ���  �������INT0
*********************************************************/
void INT1_Init(void)
{
  IT1 = 1;     //ʹ��INT1�½����ж�
  EX1 = 1;      //ʹ��INT1�ж�
  EA  = 1;     //�����ж� 
}

/*********************************************************
*������: �˿ڳ�ʼ��
*�ⲿ����Ƶ�ʣ�24MHZ
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.16 10 ��00
*********************************************************/
void GPIO_Init(void)
{
    P3M0 = 0x02;  //����P3^1 TXD ǿ�������,����P3^2 P3^3Ϊ׼˫���
    P3M1 = 0x00;
    P0M0 = 0x83;  //����P0^0 P0^1 P0^7 ǿ�������
    P0M1 = 0x00;
    P00 = 1;                  //��ѹ����ͨ������  ��ʼΪ��
    P01 = 1;                  //��������ͨ������  ��ʼΪ��
    P07 = 1;                  //LEDָʾ�� ����
}


/*********************************************************
*������:  ADC��ʼ��
*�ⲿ����Ƶ�ʣ�32.768MHZ
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.15 20 ��35
*********************************************************/
void ADC_Init(void)
{
  P1M0 = 0x00;              //����P1^0 ΪADC ��������
  P1M1 = 0x03;              //����P1^1 ΪADC ��ѹ���� 
  ADCCFG = 0x20;            //����ADCʱ��Ϊϵͳʱ��/2/1   ת������Ҷ��� ADC_RES�����4λ
  ADC_CONTR |= 0x80;         //ʹ��ADCģ�� 
}
/*********************************************************
*������:  ��ȡADC����
*����˵���� ADC_CHANNEL ADCͨ����0-15��
*�ⲿ����Ƶ�ʣ�24MHZ
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.15 20 ��35
*********************************************************/
unsigned int ADC_GetResult(unsigned int ADC_CHANNEL)
{
  unsigned int Result = 0;
  ADC_RES = 0;
  ADC_RESL = 0;
  ADC_CONTR|= ADC_CHANNEL;
  ADC_CONTR |= 0x40;        //����ADת�� 
  _nop_();
  while(!(ADC_CONTR & 0X20));  //��ѯADC��ɱ�־
  ADC_CONTR &= ~0x20;          //����ɱ�־
  Result = ADC_RES;
  Result = (Result<<8|ADC_RESL);                //��ȡADC��4λ��� 
  ADC_CONTR &= 0xF0;          //�����һ��ͨ�� 
  return Result;
}

/*********************************************************
*������:  ��λֵƽ���˲���
*�����ˣ�������
*�汾��v1.0
*ʱ�䣺2019.7.27 17 ��35
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
      if(value_buf[i] > value_buf[i+1])  //ð���㷨
      {
         temp = value_buf[i];
         value_buf[i] = value_buf[i+1];
         value_buf[i+1] = temp;
      }
    }
  }
  for(count=1;count<VPTH_Value-1;count++)  //ȥ�����ֵ����Сֵ
      sum += value_buf[count];
  return (sum/(VPTH_Value-2));
}

/*********************************************************
*������:  �������ƺ���
*����˵���� 
*�ⲿ����Ƶ�ʣ�24MHZ
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.24 18 ��02
*********************************************************/
void Overcurrent_Control(void)
{
    unsigned int Voltage0=0;
    unsigned char i = 0;
    Voltage0 = ADC_GetResult(ADC_CHANNEL0);  
    printf("---- Voltage0 = :0x%04X\r\n",Voltage0);
    if(Voltage0 > VEDI)       //����  1.64V   ����ë��
    //if(Voltage0 > 0x0A)       //����  1.64V
    {   
        Voltage0 = ADC_Result_Filter(ADC_CHANNEL0);   //�����˲�����
        if(Voltage0 > VEDI)      //��ʵ����
        {       
            P00 = 0;
            P01 = 0; 
            TR0 = 1; //������ʱ��
            printf("Beyond the standard here\r\n");        
        }
        Voltage0 = 0;
    }    
}
/*********************************************************
*������:  ��ѹ���ƺ���
*����˵���� 
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.24 18 ��02
*********************************************************/
void Overvoltage_Control() 
{
    unsigned int Voltage1=0;
    unsigned char i = 0;       
    Voltage1 = ADC_GetResult(ADC_CHANNEL1);              
   //printf("---- Voltage1 = :0x%04X\r\n",Voltage1);
   if(Voltage1 > 0x0E14)      //��ѹ 2.2V   ������ë��
   //if(Voltage1 > 0x0E)      //��ѹ 2.2V
    {
      Voltage1 = ADC_Result_Filter(ADC_CHANNEL1); //�����˲�����
      if(Voltage1 > 0x0E14)   //��ʵ��ѹ
      {
         P00 = 0;
         P01 = 0;      
         TR0 = 1; //������ʱ��  
         printf("Beyond the standard here\r\n");
      }
      Voltage1 = 0;
    }   
}

/**************************************************************************
���������ر�IAP����
��  �ã�IAP_Read();
*�ڲ�����Ƶ�ʣ�24MHZ
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.16 10 ��00
*��ע��
/****************************************************************************/
void IAP_Idle()
{
  IAP_CONTR = 0;     //�ر�IAP����
  IAP_CMD = 0;       //�������Ĵ���
  IAP_TRIG = 0;      //��������Ĵ���
  IAP_ADDRH = 0X80;   //����ַ���õ���IAP����
  IAP_ADDRL = 0;
}

/**************************************************************************
��������EEPROM ��ȡ����
��  �ã�
*�ⲿ����Ƶ�ʣ�24MHZ
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.16 10 ��00
*��ע��
/****************************************************************************/
char IAP_Read(int addr)
{
  char dat;
  
  IAP_CONTR = WT_24M;    //ʹ��IAP
  IAP_CMD = 1;           //����IAP������
  IAP_ADDRL = addr;      //����IAP�͵�ַ
  IAP_ADDRH = addr>>8;   //����IAP�ߵ�ַ
  IAP_TRIG = 0x5a;       //д��������(0x5a)
  IAP_TRIG = 0xa5;       //д��������(0xa5)
  _nop_();
  dat = IAP_DATA;        //��IAP����
  IAP_Idle();            //�ر�IAP����
  
  return dat;
}

/**************************************************************************
��������EEPROM д�뺯��
*�ⲿ����Ƶ�ʣ�24MHZ  ����Ƶ
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.16 10 ��00
*��ע��
/****************************************************************************/
void IAP_Program(int addr, char dat)
{
  IAP_CONTR = WT_24M;    //ʹ��IAP
  IAP_CMD = 2;           //����IAPд����
  IAP_ADDRL = addr;      //����IAP�͵�ַ
  IAP_ADDRH = addr>>8;   //����IAP�ߵ�ַ
  IAP_DATA = dat;        //дIAP����    
  IAP_TRIG = 0x5a;       //д��������(0x5a)
  IAP_TRIG = 0xa5;       //д��������(0xa5)
  _nop_(); 
  IAP_Idle();            //�ر�IAP����
}

/**************************************************************************
��������EEPROM ��������
*�ⲿ����Ƶ�ʣ�24MHZ
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.16 10 ��00
*��ע������1���ö�ʱ��1��ģʽ0���������ʷ�����
/****************************************************************************/
void IAP_Erase(int addr)
{
  IAP_CONTR = WT_24M;    //ʹ��IAP
  IAP_CMD = 3;           //����IAPд����
  IAP_ADDRL = addr;      //����IAP�͵�ַ
  IAP_ADDRH = addr>>8;   //����IAP�ߵ�ַ
  IAP_TRIG = 0x5a;       //д��������(0x5a)
  IAP_TRIG = 0xa5;       //д��������(0xa5)
  _nop_(); 
  IAP_Idle();            //�ر�IAP����
}
/**************************************************************************
*��������EEPROM_Init()
*������
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.16 10 ��00
*��ע��EEPROM �����ж������Ƿ�Ϊ�գ��ж��ǲ��ǵ�һ��д��
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
*��������EEPROM_Program()
*������
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.24 10 ��00
*��ע��EEPROMд��֮ǰ Ҫ�Ȳ���
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
*�����������ַ�"0~9,A~F��a~f"ת��ʮ������
*������dat: Ҫ�����ַ�
*����: 0x00~0x0FΪ��ȷ. 0xFFΪ����.
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.25 18 ��00
*��ע��
/****************************************************************************/
unsigned char	CheckData(unsigned char dat)
{
	if((dat >= '0') && (dat <= '9'))		return (dat-'0');
	if((dat >= 'A') && (dat <= 'F'))		return (dat-'A'+10);
	return 0xff;
}

/**************************************************************************
*������������������뷽ʽ�ĵ�ַ.
*��������
*���أ�16λEEPROM��ַ.
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.25 19 ��40
*��ע��
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
*������: �����������ݴ���
*����˵���� 
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.25 15 ��31
*********************************************************/
void Serial_InputData_Processing ()
{
  int i = 0;
  if(B_Rx_OK)        //������ı�־λ, �յ����ݿ�ϵͳ����1, �û��������ݺ������0
  {
      if(Rx_Cnt > 0)	//ȷ��������
      {
          //Uart1_SendStr(RxBuf);		//���յ�������ԭ������,���ڲ��� 
          printf("%s\r\n",RxBuf);        
          if((Rx_Cnt >= 6) && (RxBuf[1] == ' '))	//�������Ϊ6���ֽ�
          {
              for(i=0; i<6; i++)
              {
                if((RxBuf[i] >= 'a') && (RxBuf[i] <= 'z'))	
                  RxBuf[i] = RxBuf[i] - 'a' + 'A';	//Сдת��д
              }            
              if(RxBuf[0] == 'R'&& RxBuf[1] == ' ')	//PC�����ȡĳһ����ֵ
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
              else if((RxBuf[0] == 'W') && (Rx_Cnt >= 8) && (RxBuf[6] == ' '))	//д��1���ֽ�
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
      Rx_Cnt  = 0;	//����ֽ���
      B_Rx_OK = 0;	//���������ɱ�־
  }
}




/*********************************************************
*������:  ������
*����˵���� 
*�ڲ�����Ƶ�ʣ�24MHZ
*�����ˣ�������
*�汾��v2.0
*ʱ�䣺2019.4.19 15 ��31
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
  //WDT_CONTR = 0x27;         //ʹ��������Ź� ���ʱ��ԼΪ3.2s 
  while(1)
  {
      //WDT_CONTR = 0x10;       //��������Ź�       
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

void TIM0_Isr() interrupt 1   //��ʱ���ж�1�� 1ms
{
  Time_Count++;
  if( Time_Count == DELY)      //��ʱDELY
  {
    Time_Count = 0;
    P00 = 1;
    P01 = 1;
    TR0 = 0;   //��ʱʱ�䵽�رն�ʱ��
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
��������UART���ڽ����жϴ�����
��  �ã�[SBUF�յ����ݺ��жϴ���]
��  ������
����ֵ����
��  ����UART���ڽ��յ�����ʱ�����жϣ��û������ݽ��д��������ͻ�ȥ��
��  ע�������Ĵ�������Ӱ��������ݵĽ���
/*****************************************************************/
void UART1_Tsr() interrupt 4  using 1  //�л��Ĵ����鵽1
{ 
    //unsigned char UART_data; //���崮�ڽ������ݱ���
   if(RI)
   {
      RI = 0;			//������жϱ�־λΪ0��������㣩
      RxBuf[Rx_Cnt] = SBUF;     //�洢���ݵ�������	save the dat to RBUF
      if(++Rx_Cnt >= RxLength)	Rx_Cnt = 0;	//����ж�
      B_Rx_OK = 1;	//��־���յ�һ֡����
   } 
   if(TI)
   {
      TI = 0;
   }       
//    SBUF = RxBuf[Rx_Cnt];	//�����յ����ݷ��ͻ�ȥ��ɾ��//����Ч��
//    while(TI == 0);	//��鷢���жϱ�־λ
//    TI = 0;		//����жϱ�־λΪ0��������㣩 	
}	




