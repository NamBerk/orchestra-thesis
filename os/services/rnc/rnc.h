/*
SEEMOO - Secure Mobile Networking Lab and Project, Winter 2017/18
author: Jan Sturm
*/

#ifndef RNC_H_
#define RNC_H_

#include <stdlib.h>
#include <stdio.h>

#include "sys/node-id.h"
#include "os/net/rime/rime.h"

#include "./gf.h"
#include "./util.h"
#include "./params.h"

struct rnc_pkt {
	uint8_t payload[M];
	uint8_t batch_id;
	uint8_t coeff[K];
	uint8_t msg_type;
};

void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from);
void init_rnc(void);
void start_rnc(void);
void generate_init_coeffs(uint8_t dia);
void rnc_generate_payload(struct rnc_pkt *p, uint8_t batch_id);
void rnc_send_pkt_delay(uint8_t batch_id);
void rnc_send_nack(void *p);
void rnc_send_nack_reply(void *p);
void rnc_broadcast(void *p);
void send_new_batch(void *ptr);
void send_new_packet(void *bid);
void process_data(struct rnc_pkt* p_recv);
uint8_t store_coeffs_and_payload(struct rnc_pkt *p_recv);
uint8_t check_data(uint8_t **v);
void clear_memory(void);

void print_batch(const char *s, uint8_t **v, uint8_t id);
void print_rnc_packet(const char *s, struct rnc_pkt *p);
void print_lse(uint8_t **mat, uint8_t **rec, uint8_t **queue);

#endif /* RNC_H_ */
