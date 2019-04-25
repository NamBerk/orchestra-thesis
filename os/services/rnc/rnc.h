/*
SEEMOO - Secure Mobile Networking Lab and Project, Winter 2017/18
author: Jan Sturm
*/

#ifndef RNC_H_
#define RNC_H_

#include <stdlib.h>
#include <stdio.h>

#include "sys/node-id.h"
#include "net/mac/tsch/tsch.h"
//#include "os/net/rime/rime.h"

#include "gf.h"
#include "util.h"
#include "params.h"
#include "net/ipv6/tcpip.h" 
#include "net/ipv6/simple-udp.h"

struct rnc_pkt{
	uint8_t payload[M];
	uint8_t batch_id;    //bu kismi tsch types in icine aldik
	uint8_t coeff[K];
	uint8_t msg_type;
};

struct simple_udp_connection *c;
void receiver(struct simple_udp_connection *c,
			  const uip_ipaddr_t *from,
			  uint16_t sender_port,
			  const uip_ipaddr_t *receiver_addr,
			  uint16_t receiver_port,
			  const uint8_t *data,
			  uint16_t datalen
				);
//void receiver(const void *data, uint16_t len,
  //const linkaddr_t *from, const linkaddr_t *dest);
void init_rnc(void);
void start_rnc(void);
void generate_init_coeffs(uint8_t dia);
void rnc_generate_payload(struct rnc_pkt *p, uint8_t batch_id);
void rnc_send_pkt_delay(uint8_t batch_id);
//void rnc_send_nack(void *p);
//void rnc_send_nack_reply(void *p);
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
