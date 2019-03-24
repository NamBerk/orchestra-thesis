
#ifndef BROAD_H_
#define BROAD_H_

#include "contiki.h"
#include "net/linkaddr.h"
#include "net/ipv6/simple-udp.h"

struct simple_udp_connection *c;

typedef void (* broadcast_input_callback)(struct simple_udp_connection *conn,
										  const linkaddr_t *src,uint16_t sender_port,
										  const linkaddr_t *dest,uint16_t receiver_port,
											const void *data, uint16_t len );
  


void broadcast_set_input_callback(broadcast_input_callback callback);

#endif 