#ifndef _BQ78350_H_
#define _BQ78350_H_
#include "stm32f2xx.h"

#define 	BQ_ADDR						0x16		//器件地址

#define 	RemainingCapacityAlarm()	0x01		//容量警告值设置	R/W
#define 	RemainingTimeAlarm()			0x02		//剩余运行时间警告值设置	R/W
#define 	BatteryMode()							0x03		//选择电池运行模式	R/W
#define 	Voltage()									0x09		//总电压	R
#define 	Current()									0x0A		//充放电电流	R +充电 -放电
#define   AverageCurrent()          0x0B    //平均充放电电流	R +充电 -放电
#define 	MaxError()								0x0C		//充电计算错误率
#define 	RelativeStateOfCharge()		0x0D		//剩余电量百分比	R
#define 	RemainingCapacity()				0x0F		//剩余电量	R
#define 	FullChargeCapacity()			0x10		//满充预测容量
#define 	RunTimeToEmpty()					0x11		//剩余运行时间 65535 = 电池没有放电	R
#define 	AverageTimeToFull()				0x13		//剩余充电时间 65535 = 电池没有充电	R
#define 	ChargingCurrent()					0x14		//最大充电电流 65535 = 电池没有充电	R
#define 	ChargingVoltage()					0x15		//最大充电电压 65535 = 电池没有充电	R
#define 	BatteryStatus()						0x16		//电池状态 R
#define 	CycleCount()							0x17		//循环次数 R r/w
#define 	DesignCapacity()					0x18		//设计容量 R r/w
#define 	DesignVoltage()						0x19 		//设计单体/整体电压 R r/w
#define 	SpecificationInfo()				0x1A		//数据缩放 R
#define 	HostFETControl()					0x2B		//主机控制MOS管 R/W
#define 	VAUXVoltage()							0x2E		//VAUX电压	R
#define 	DynamicPower()						0x4C		//运行功率 R
#define 	ExtAveCellVoltage()				0x4D		//返回EDV参数	R
#define 	StateOfHealth()						0x4F		//SOH - 容量和能量	R

#define 	BQ_CellNum				0x06		//电池数量
#define 	BQ_CellVoltage1		0x3F		//电池1电压地址 R
#define 	BQ_CellVoltage2		0x3E		//电池2电压地址	R
#define 	BQ_CellVoltage3		0x3D		//电池3电压地址	R
#define 	BQ_CellVoltage4		0x3C		//电池4电压地址	R
#define 	BQ_CellVoltage5		0x3B		//电池5电压地址	R
#define 	BQ_CellVoltage6		0x3A		//电池6电压地址	R

#define 	BQ_TempNum				0x02		//温度传感器数量
#define 	BQ_Temp						0x08		//温度最大值或平均值


/***    以下命令需要写入 ManufacturerAccess() 中后在 ManufacturerBlockAccess() 中读取返回值    ***/
#define 	ManufacturerAccess()				0x00			//写入指定信息
#define 	ManufacturerBlockAccess()		0x44			//以块格式写入/读取指定信息
																								//格式：Block_Write + Block_Read
																								//Block_Write ： 起始 + 器件地址 + 0x44 + 写入的字节数 + 地址低 + 地址高 + 结束
																								//Block_Read 	： 起始 + 器件地址 + 0x44 + 起始 + 器件地址（读） + 返回数据 + 结束
																								//返回数据格式： 字节数 + 地址低 + 地址高 + 数据Byte * N 
/* 																	设置 																		*/
#define 	Device_Type									0x0001 		//设备类型
#define 	CellBalanceToggle						0x001B		//单元平衡切换
#define 	AFEDelayDisable							0x001C		//AFE延迟禁用
#define 	PRE_CHG_FET									0x001E 		//打开/关闭Pre-CHG FET驱动功能
#define 	CHG_FET											0x001F 		//打开/关闭CHG FET驱动功能
#define 	DSG_FET											0x0020 		//打开/关闭DSG FET驱动功能
#define 	FET_Control									0x0022		//禁用/启用固件对CHG，DSG和PCHG FET的控制
#define 	LifetimeDataCollection			0x0023		//禁用/启用生命周期数据收集
#define 	PermanentFailure						0x0024		//禁用/启用永久失败
#define 	BlackBoxRecorder						0x0025		//启用/禁用黑盒记录器功能
#define 	LED_DisplayEnable						0x0027 		//启用或禁用DISP引脚触发的LED显示功能
#define 	LifetimeDataReset						0x0028 		//重置数据闪存中的Lifetime数据
#define 	PermanentFailDataReset			0x0029		//重置数据闪存中的PF(永久失败)数据数据
#define 	BlackBoxRecorderReset				0x002A		//重置数据闪存中的黑盒记录器数据
#define 	LED_TOGGLE 									0x002B 		//LED显示屏从关闭切换为开启
#define 	LED_DisplayPress						0x002C 		//开启LED显示：模拟DISP引脚的低 - 高 - 低检测
#define 	DeviceReset									0x0041		//复位bq78350-R1
/* 																	读取 																		*/
#define 	SafetyAlert									0x0050		//返回ManufacturerBlockAccess（）上的SafetyAlert（）标志。
#define 	SafetyStatus								0x0051		//返回ManufacturerBlockAccess上的SafetyStatus标志。
#define 	PFAlert											0x0052		//返回ManufacturerBlockAccess上的PFAlert标志
#define 	PFStatus										0x0053		//返回ManufacturerBlockAccess上的PFStatus标志
#define 	OperationStatus							0x0054		//返回ManufacturerBlockAccess上的操作状态OperationStatus标志
#define 	ChargingStatus							0x0055		//返回ManufacturerBlockAccess上的ChargingStatus标志 - 预充模式
#define 	GaugingStatus								0x0056		//返回ManufacturerBlockAccess上的测量状态GaugingStatus标志
#define 	ManufacturingStatus					0x0057		//返回ManufacturerBlockAccess上的生产状况ManufacturingStatus标志
#define 	AFEStatus										0x0058 		//返回相应AFE的配置(地址0x00)
#define 	AFEConfig										0x0059 		//返回相应AFE的配置(地址0x01到0x0B)
#define 	AFEData											0x005B		//AEF测量的数据 电压、温度等	R
#define 	LifetimeDataBlock1					0x0060 		//0x0060 - 0x0066返回ManufacturerBlockAccess上的生存期数据
#define 	DAStatus1										0x0071		//返回ManufacturerBlockAccess上的电池电压
#define 	DAStatus2										0x0072		//返回ManufacturerBlockAccess()上的TS1、TS2温度数据


/* 设置 Flash 数据 仅由ManufacturerBlockAccess()通过寻址物理地址访问；数据都必须以小端部发送 */
//读取数据为大端模式
#define 	DesignCapacity_mAh					0x45CC		//设计电池容量 - 1350mah
#define 	Design_Voltage							0x45D0		//设计电池电压 - 3200mv
#define 	FET_Options									0x445F		//场效应晶体管的选择 - KEYIN
#define 	AFE_Cell_Map								0x44AC		//cell检测选择
#define 	Protection_Con							0x44C5		//保护设置 - VAUX
#define 	EnabledProtectionsA					0x44C6		//启用保护A
#define 	EnabledProtectionsB					0x44C7		//启用保护B
#define 	EnabledProtectionsC					0x44C8		//启用保护B
#define 	CEDV_GaugingCon							0x458B		//CEDV测量配置
#define 	LED_Configuration						0x448C		//显示配置启用/禁用选项
#define 	DA_Configuration						0x44AB 		//设置测量特性
#define 	CUV_Threshold								0x44CC		//电池欠压保护值 - 2500mv
#define 	CUV_Recovery								0x44CF		//电池欠压恢复值 - 2550mv
#define 	COV_Threshold								0x44D1		//电池过压保护值 - 3650mv
#define 	COV_Recovery								0x44D4		//电池过压恢复值 - 3600mv
#define 	OCC_Threshold								0x44D6		//充电过流保护值 - 3000mA
#define 	OCC_Delay										0x44D8		//充电过流保护等待值 - 2s			
#define 	OCC_Recovery								0x44D9		//充电过流保护恢复值 - -10mA		
#define 	OCC_RecoveryDelay						0x44DB		//充电过流保护恢复等待值 - 2s			
#define 	OCD_Threshold								0x44DC		//放电过流保护值 - -3000mA
#define 	OCD_Delay										0x44DE		//放电过流保护等待值 - 2s			
#define 	OCD_Recovery								0x44DF		//放电过流保护恢复值 - 10mA		
#define 	OCD_RecoveryDelay						0x44E1		//放电过流保护恢复等待值 - 2s	
#define 	OTC_Threshold								0x44ED		//充电过温保护值 - 默认55℃
#define 	OTC_Recovery								0x44F0		//充电过温恢复值 - 默认50℃
#define 	OTD_Threshold								0x44F2		//放电过温保护值 - 默认60℃
#define 	OTD_Recovery								0x44F5		//放电过温恢复值 - 默认55℃
#define 	OTF_Threshold								0x44F7		//FET过温保护值 - 默认80℃
#define 	OTF_Recovery								0x44FA		//FET过温恢复值 - 默认65℃
#define 	UTC_Threshold								0x44FC		//充电欠温保护值 - 默认0℃
#define 	UTC_Recovery								0x44FF		//充电欠温恢复值 - 10℃
#define 	UTD_Threshold								0x4501		//放电欠温保护值 - 默认0℃
#define 	UTD_Recovery								0x4504		//放电欠温恢复值 - 10℃
#define 	Charge_Threshold						0x4510		//充电电流保护值 - 2500mA
#define 	CTO_Delay										0x4514 		//充电计时 - 3Hour
#define 	CTO_Reset										0x4516 		//充电复位容量 - 默认2mAh
#define 	OC_Threshold								0x4518		//过充容量保护值 - 1375mAh
#define 	OC_Recovery									0x451A		//过充容量恢复值 - 1300mAh
#define 	Cell_Balance_Threshold			0x460D		//电池均衡阈值 - 3300mv
#define 	Fast_ChargingVoltage				0x45E4		//快充电压 - 3600mv
#define 	Fast_ChargingCurrent				0x45E6		//快充电流 - 1000ma
#define 	Pre_ChargingStartVoltage		0x45EA		//预充开始电压 - 2600mv
#define 	Pre_ChargingRecVoltage			0x45EC		//预充恢复电压 - 3000mv
#define 	Pre_ChargingCurrent					0x45E8		//预充电流 - 1000ma

#define 	Dsg_Current_Threshold				0x44BD		//放电电流阈值 - 1000ma
#define 	Chg_Current_Threshold				0x44BF		//充电电流阈值 - 1000ma
#define 	Quit_Current								0x44C1		//离开电流 - 2ma
#define 	Learned_FullChargeCapacity	0x42C2		//学习充电容量 - 1350mah
#define 	Fixed_EDV0									0x459A		//固定EDV0值 - 0%SOC - 2550mv
#define 	Fixed_EDV1									0x459D		//固定EDV1值 - 3%SOC - 2900mv
#define 	Fixed_EDV2									0x45A0		//固定EDV2值 - 7%SOC - 3100mv

#define 	Balancing_Configuration			0x460C		//电池均衡设置

#define 	Cell1_MaxVoltage						0x40C0		//电池1历史最高电压		
#define 	Cell2_MaxVoltage						0x40C2		//电池2历史最高电压
#define 	Cell3_MaxVoltage						0x40C4		//电池3历史最高电压
#define 	Cell4_MaxVoltage						0x40C6		//电池4历史最高电压
#define 	Cell5_MaxVoltage						0x40C8		//电池5历史最高电压
#define 	Cell6_MaxVoltage						0x40CA		//电池6历史最高电压

#define 	Cell1_MinVoltage						0x40E0		//电池1历史最低电压		
#define 	Cell2_MinVoltage						0x40E2		//电池2历史最低电压
#define 	Cell3_MinVoltage						0x40E4		//电池3历史最低电压
#define 	Cell4_MinVoltage						0x40E6		//电池4历史最低电压
#define 	Cell5_MinVoltage						0x40E8		//电池5历史最低电压
#define 	Cell6_MinVoltage						0x40EA		//电池6历史最低电压

#define 	Max_ChargeCurrent						0x4102		//历史最高充电电流
#define 	Max_DischargeCurrent				0x4104		//历史最高放电电流

#define 	Max_TempCell 								0x410A		//历史最高电池温度
#define 	Min_TempCell								0x410B		//历史最低电池温度
#define 	Max_TempFet									0x410D		//历史最高FET温度

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
