#ifndef _BQ78350_H_
#define _BQ78350_H_
#include "stm32f2xx.h"

#define 	BQ_ADDR						0x16		//������ַ

#define 	RemainingCapacityAlarm()	0x01		//��������ֵ����	R/W
#define 	RemainingTimeAlarm()			0x02		//ʣ������ʱ�侯��ֵ����	R/W
#define 	BatteryMode()							0x03		//ѡ��������ģʽ	R/W
#define 	Voltage()									0x09		//�ܵ�ѹ	R
#define 	Current()									0x0A		//��ŵ����	R +��� -�ŵ�
#define   AverageCurrent()          0x0B    //ƽ����ŵ����	R +��� -�ŵ�
#define 	MaxError()								0x0C		//�����������
#define 	RelativeStateOfCharge()		0x0D		//ʣ������ٷֱ�	R
#define 	RemainingCapacity()				0x0F		//ʣ�����	R
#define 	FullChargeCapacity()			0x10		//����Ԥ������
#define 	RunTimeToEmpty()					0x11		//ʣ������ʱ�� 65535 = ���û�зŵ�	R
#define 	AverageTimeToFull()				0x13		//ʣ����ʱ�� 65535 = ���û�г��	R
#define 	ChargingCurrent()					0x14		//�������� 65535 = ���û�г��	R
#define 	ChargingVoltage()					0x15		//������ѹ 65535 = ���û�г��	R
#define 	BatteryStatus()						0x16		//���״̬ R
#define 	CycleCount()							0x17		//ѭ������ R r/w
#define 	DesignCapacity()					0x18		//������� R r/w
#define 	DesignVoltage()						0x19 		//��Ƶ���/�����ѹ R r/w
#define 	SpecificationInfo()				0x1A		//�������� R
#define 	HostFETControl()					0x2B		//��������MOS�� R/W
#define 	VAUXVoltage()							0x2E		//VAUX��ѹ	R
#define 	DynamicPower()						0x4C		//���й��� R
#define 	ExtAveCellVoltage()				0x4D		//����EDV����	R
#define 	StateOfHealth()						0x4F		//SOH - ����������	R

#define 	BQ_CellNum				0x06		//�������
#define 	BQ_CellVoltage1		0x3F		//���1��ѹ��ַ R
#define 	BQ_CellVoltage2		0x3E		//���2��ѹ��ַ	R
#define 	BQ_CellVoltage3		0x3D		//���3��ѹ��ַ	R
#define 	BQ_CellVoltage4		0x3C		//���4��ѹ��ַ	R
#define 	BQ_CellVoltage5		0x3B		//���5��ѹ��ַ	R
#define 	BQ_CellVoltage6		0x3A		//���6��ѹ��ַ	R

#define 	BQ_TempNum				0x02		//�¶ȴ���������
#define 	BQ_Temp						0x08		//�¶����ֵ��ƽ��ֵ


/***    ����������Ҫд�� ManufacturerAccess() �к��� ManufacturerBlockAccess() �ж�ȡ����ֵ    ***/
#define 	ManufacturerAccess()				0x00			//д��ָ����Ϣ
#define 	ManufacturerBlockAccess()		0x44			//�Կ��ʽд��/��ȡָ����Ϣ
																								//��ʽ��Block_Write + Block_Read
																								//Block_Write �� ��ʼ + ������ַ + 0x44 + д����ֽ��� + ��ַ�� + ��ַ�� + ����
																								//Block_Read 	�� ��ʼ + ������ַ + 0x44 + ��ʼ + ������ַ������ + �������� + ����
																								//�������ݸ�ʽ�� �ֽ��� + ��ַ�� + ��ַ�� + ����Byte * N 
/* 																	���� 																		*/
#define 	Device_Type									0x0001 		//�豸����
#define 	CellBalanceToggle						0x001B		//��Ԫƽ���л�
#define 	AFEDelayDisable							0x001C		//AFE�ӳٽ���
#define 	PRE_CHG_FET									0x001E 		//��/�ر�Pre-CHG FET��������
#define 	CHG_FET											0x001F 		//��/�ر�CHG FET��������
#define 	DSG_FET											0x0020 		//��/�ر�DSG FET��������
#define 	FET_Control									0x0022		//����/���ù̼���CHG��DSG��PCHG FET�Ŀ���
#define 	LifetimeDataCollection			0x0023		//����/�����������������ռ�
#define 	PermanentFailure						0x0024		//����/��������ʧ��
#define 	BlackBoxRecorder						0x0025		//����/���úںм�¼������
#define 	LED_DisplayEnable						0x0027 		//���û����DISP���Ŵ�����LED��ʾ����
#define 	LifetimeDataReset						0x0028 		//�������������е�Lifetime����
#define 	PermanentFailDataReset			0x0029		//�������������е�PF(����ʧ��)��������
#define 	BlackBoxRecorderReset				0x002A		//�������������еĺںм�¼������
#define 	LED_TOGGLE 									0x002B 		//LED��ʾ���ӹر��л�Ϊ����
#define 	LED_DisplayPress						0x002C 		//����LED��ʾ��ģ��DISP���ŵĵ� - �� - �ͼ��
#define 	DeviceReset									0x0041		//��λbq78350-R1
/* 																	��ȡ 																		*/
#define 	SafetyAlert									0x0050		//����ManufacturerBlockAccess�����ϵ�SafetyAlert������־��
#define 	SafetyStatus								0x0051		//����ManufacturerBlockAccess�ϵ�SafetyStatus��־��
#define 	PFAlert											0x0052		//����ManufacturerBlockAccess�ϵ�PFAlert��־
#define 	PFStatus										0x0053		//����ManufacturerBlockAccess�ϵ�PFStatus��־
#define 	OperationStatus							0x0054		//����ManufacturerBlockAccess�ϵĲ���״̬OperationStatus��־
#define 	ChargingStatus							0x0055		//����ManufacturerBlockAccess�ϵ�ChargingStatus��־ - Ԥ��ģʽ
#define 	GaugingStatus								0x0056		//����ManufacturerBlockAccess�ϵĲ���״̬GaugingStatus��־
#define 	ManufacturingStatus					0x0057		//����ManufacturerBlockAccess�ϵ�����״��ManufacturingStatus��־
#define 	AFEStatus										0x0058 		//������ӦAFE������(��ַ0x00)
#define 	AFEConfig										0x0059 		//������ӦAFE������(��ַ0x01��0x0B)
#define 	AFEData											0x005B		//AEF���������� ��ѹ���¶ȵ�	R
#define 	LifetimeDataBlock1					0x0060 		//0x0060 - 0x0066����ManufacturerBlockAccess�ϵ�����������
#define 	DAStatus1										0x0071		//����ManufacturerBlockAccess�ϵĵ�ص�ѹ
#define 	DAStatus2										0x0072		//����ManufacturerBlockAccess()�ϵ�TS1��TS2�¶�����


/* ���� Flash ���� ����ManufacturerBlockAccess()ͨ��Ѱַ�����ַ���ʣ����ݶ�������С�˲����� */
//��ȡ����Ϊ���ģʽ
#define 	DesignCapacity_mAh					0x45CC		//��Ƶ������ - 1350mah
#define 	Design_Voltage							0x45D0		//��Ƶ�ص�ѹ - 3200mv
#define 	FET_Options									0x445F		//��ЧӦ����ܵ�ѡ�� - KEYIN
#define 	AFE_Cell_Map								0x44AC		//cell���ѡ��
#define 	Protection_Con							0x44C5		//�������� - VAUX
#define 	EnabledProtectionsA					0x44C6		//���ñ���A
#define 	EnabledProtectionsB					0x44C7		//���ñ���B
#define 	EnabledProtectionsC					0x44C8		//���ñ���B
#define 	CEDV_GaugingCon							0x458B		//CEDV��������
#define 	LED_Configuration						0x448C		//��ʾ��������/����ѡ��
#define 	DA_Configuration						0x44AB 		//���ò�������
#define 	CUV_Threshold								0x44CC		//���Ƿѹ����ֵ - 2500mv
#define 	CUV_Recovery								0x44CF		//���Ƿѹ�ָ�ֵ - 2550mv
#define 	COV_Threshold								0x44D1		//��ع�ѹ����ֵ - 3650mv
#define 	COV_Recovery								0x44D4		//��ع�ѹ�ָ�ֵ - 3600mv
#define 	OCC_Threshold								0x44D6		//����������ֵ - 3000mA
#define 	OCC_Delay										0x44D8		//�����������ȴ�ֵ - 2s			
#define 	OCC_Recovery								0x44D9		//�����������ָ�ֵ - -10mA		
#define 	OCC_RecoveryDelay						0x44DB		//�����������ָ��ȴ�ֵ - 2s			
#define 	OCD_Threshold								0x44DC		//�ŵ��������ֵ - -3000mA
#define 	OCD_Delay										0x44DE		//�ŵ���������ȴ�ֵ - 2s			
#define 	OCD_Recovery								0x44DF		//�ŵ���������ָ�ֵ - 10mA		
#define 	OCD_RecoveryDelay						0x44E1		//�ŵ���������ָ��ȴ�ֵ - 2s	
#define 	OTC_Threshold								0x44ED		//�����±���ֵ - Ĭ��55��
#define 	OTC_Recovery								0x44F0		//�����»ָ�ֵ - Ĭ��50��
#define 	OTD_Threshold								0x44F2		//�ŵ���±���ֵ - Ĭ��60��
#define 	OTD_Recovery								0x44F5		//�ŵ���»ָ�ֵ - Ĭ��55��
#define 	OTF_Threshold								0x44F7		//FET���±���ֵ - Ĭ��80��
#define 	OTF_Recovery								0x44FA		//FET���»ָ�ֵ - Ĭ��65��
#define 	UTC_Threshold								0x44FC		//���Ƿ�±���ֵ - Ĭ��0��
#define 	UTC_Recovery								0x44FF		//���Ƿ�»ָ�ֵ - 10��
#define 	UTD_Threshold								0x4501		//�ŵ�Ƿ�±���ֵ - Ĭ��0��
#define 	UTD_Recovery								0x4504		//�ŵ�Ƿ�»ָ�ֵ - 10��
#define 	Charge_Threshold						0x4510		//����������ֵ - 2500mA
#define 	CTO_Delay										0x4514 		//����ʱ - 3Hour
#define 	CTO_Reset										0x4516 		//��縴λ���� - Ĭ��2mAh
#define 	OC_Threshold								0x4518		//������������ֵ - 1375mAh
#define 	OC_Recovery									0x451A		//���������ָ�ֵ - 1300mAh
#define 	Cell_Balance_Threshold			0x460D		//��ؾ�����ֵ - 3300mv
#define 	Fast_ChargingVoltage				0x45E4		//����ѹ - 3600mv
#define 	Fast_ChargingCurrent				0x45E6		//������ - 1000ma
#define 	Pre_ChargingStartVoltage		0x45EA		//Ԥ�俪ʼ��ѹ - 2600mv
#define 	Pre_ChargingRecVoltage			0x45EC		//Ԥ��ָ���ѹ - 3000mv
#define 	Pre_ChargingCurrent					0x45E8		//Ԥ����� - 1000ma

#define 	Dsg_Current_Threshold				0x44BD		//�ŵ������ֵ - 1000ma
#define 	Chg_Current_Threshold				0x44BF		//��������ֵ - 1000ma
#define 	Quit_Current								0x44C1		//�뿪���� - 2ma
#define 	Learned_FullChargeCapacity	0x42C2		//ѧϰ������� - 1350mah
#define 	Fixed_EDV0									0x459A		//�̶�EDV0ֵ - 0%SOC - 2550mv
#define 	Fixed_EDV1									0x459D		//�̶�EDV1ֵ - 3%SOC - 2900mv
#define 	Fixed_EDV2									0x45A0		//�̶�EDV2ֵ - 7%SOC - 3100mv

#define 	Balancing_Configuration			0x460C		//��ؾ�������

#define 	Cell1_MaxVoltage						0x40C0		//���1��ʷ��ߵ�ѹ		
#define 	Cell2_MaxVoltage						0x40C2		//���2��ʷ��ߵ�ѹ
#define 	Cell3_MaxVoltage						0x40C4		//���3��ʷ��ߵ�ѹ
#define 	Cell4_MaxVoltage						0x40C6		//���4��ʷ��ߵ�ѹ
#define 	Cell5_MaxVoltage						0x40C8		//���5��ʷ��ߵ�ѹ
#define 	Cell6_MaxVoltage						0x40CA		//���6��ʷ��ߵ�ѹ

#define 	Cell1_MinVoltage						0x40E0		//���1��ʷ��͵�ѹ		
#define 	Cell2_MinVoltage						0x40E2		//���2��ʷ��͵�ѹ
#define 	Cell3_MinVoltage						0x40E4		//���3��ʷ��͵�ѹ
#define 	Cell4_MinVoltage						0x40E6		//���4��ʷ��͵�ѹ
#define 	Cell5_MinVoltage						0x40E8		//���5��ʷ��͵�ѹ
#define 	Cell6_MinVoltage						0x40EA		//���6��ʷ��͵�ѹ

#define 	Max_ChargeCurrent						0x4102		//��ʷ��߳�����
#define 	Max_DischargeCurrent				0x4104		//��ʷ��߷ŵ����

#define 	Max_TempCell 								0x410A		//��ʷ��ߵ���¶�
#define 	Min_TempCell								0x410B		//��ʷ��͵���¶�
#define 	Max_TempFet									0x410D		//��ʷ���FET�¶�

signed int Complement_u16data(u16 data); 
signed char Complement_u8data(u8 data);

u8 BQ78350_Block_Writedata(u8 addr,u16 vddr,signed int data,u8 size);

u8 BQ78350_SBS_Write(u8 addr,u16 vddr);
u8 BQ78350_SBS_Read(u8 addr,u8* data,u8 size);
u8 BQ78350_SBS_CMD(u8 addr,u16 vddr,u8* data,u8 size);

u8 BQ78350_Read_buf(u8 addr,u16 vddr,u8* data,u8 size);
u32 BQ78350_Read_DevType2(u8* R_data,u8 vddr);
u8 BQ78350_Write_Byte(u8 addr,u16 vddr,u16 type);
u32 BQ78350_Read_Byte(u8 addr,u16 vddr);
u8 BQ78350_Block_Write(u8 addr,u16 vddr);
u8 BQ78350_Block_Read(u8 addr,u8* data,u8 size);
u8 BQ78350_Block_CMD(u8 addr,u16 vddr,u8* data,u8 size);

#endif
