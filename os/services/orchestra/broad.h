
#ifndef BROAD_H_
#define BROAD_H_

#include "contiki.h"
#include "net/linkaddr.h"


typedef void (* broadcast_input_callback)(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest);


void broadcast_set_input_callback(broadcast_input_callback callback);

#endif 