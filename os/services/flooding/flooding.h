/*
SEEMOO - Secure Mobile Networking Lab and Project, Winter 2017/18
author: Jan Sturm
*/

#ifndef FLOODING_H_
#define FLOODING_H_

#include <stdlib.h>
#include <stdio.h>

#include "sys/node-id.h"
#include "os/net/rime/rime.h"
#include "./params.h"
#include "./gf.h"
#include "./util.h"


struct flooding_pkt {
	uint8_t payload[M];
	uint8_t batch_id;
	uint8_t pkt_id;
	uint8_t msg_type;
};

void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from);
void init_flooding(void);
void start_flooding(void);

void flood_packet_delay(struct flooding_pkt *p_recv);
void flooding_send_nack(void *p);
void flooding_send_nack_reply(void *p);

void send_new_batch(void *ptr);
void send_new_packet(void *bid);
void process_data(struct flooding_pkt* p_recv);

void flooding_broadcast(void *p);
int check_recovered();
int get_empty_packet_id();
void clear_memory(void);

uint8_t check_data(uint8_t **v);
void print_batch(const char *s, uint8_t **v, uint8_t id);
void print_flooding_packet(const char *s, struct flooding_pkt *p);

#endif /* FLOODING_H_ */
