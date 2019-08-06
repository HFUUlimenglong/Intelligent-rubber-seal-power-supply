#ifndef __CAN_H__
#define __CAN_H__

#include <rtthread.h>
#include <stdint.h>


typedef struct 
{
	uint32_t can_id;
	uint16_t can_sended_len;
	uint16_t can_all_len;
	uint8_t can_buf[128];
}CAN_DATA_SEND;


struct CACHE_DATA
{
	uint8_t data[128];
	struct CACHE_DATA *next;
};

struct QUEUE
{
	struct CACHE_DATA *real;
	struct CACHE_DATA *front;
};



#define CAN_CARD_DATA  0x01
#define CAN_STATION_DATA 0x02
#define CAN_CHANNEL_STATE 0x03

#define CAN_STATION_NET_DISCONNECT 0x00
#define CAN_STATION_NET_CONNECT 0x01
#define CAN_STATION_DATA_TRANSMITION 0x02    //基站数据发送到can总线上
#define CAN_CARD_DATA_TRANSMITION 0x03       //卡数据发送到can总线上

void CAN_Configuration(void);
uint8_t can_data_send(uint32_t Id,uint8_t IDE,uint8_t RTR,uint8_t *can_data,uint8_t len);
rt_bool_t start_can_work(void);


void QUEUE_init(struct QUEUE *hp);
void QUEUE_input(struct QUEUE *hp,uint8_t * data, uint8_t data_len);
void QUEUE_output(struct QUEUE *hp,uint8_t *data,uint8_t len);
void QUEUE_clear(struct QUEUE *hp);
char QUEUE_check(struct QUEUE *hp);

#endif







