/*
SEEMOO - Secure Mobile Networking Lab and Project, Winter 2017/18
author: Jan Sturm
*/

#include "contiki.h"
#include "net/netstack.h"
#include "flooding.h"
#include "random.h"
#include "sys/ctimer.h"
#include <stdlib.h>
#include "net/ipv6/tcpip.h"
#include "net/linkaddr.h"
#include "net/packetbuf.h"
#include "net/mac/tsch/tsch.h"
#ifdef LEDS
#include "leds.h"
#endif

#define UDP_CLIENT	8765
#define UDP_SERVER	5678

/* mode can be set in params.h */
static uint8_t mode;

/* batch that is currently operated on */
static uint8_t local_batch_id = 0;

/* various callback timers for timeouts */
static struct ctimer timer_batch;
static struct ctimer timer_packet;
static struct ctimer timer_send;
static struct ctimer timer_nack;
static struct ctimer timer_nack_reply;

// count how much the source already sent in one batch
static uint8_t counter_send_pkt = 0;

static uint8_t **packet_queue; // KxM

/* indicates which packets are already received */
static uint16_t rec = 0;

/* for lazy NACK */
static uint8_t nack_multiplier = 1;

/* give nodes priority in broadcasting if they didn't sent a long time */
static uint8_t priority = 0;

/* indicates if a NACK_REPLY message was already received */
static uint8_t nack_reply_received = 0;

struct flooding_pkt pkt_nack_reply;

/*static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;*/

void receiver(struct uip_udp_conn *c, const linkaddr_t *from) {

  if (mode != MODE_SINK && !check_recovered()) {
    ctimer_set(&timer_nack, DELAY_NACK(nack_multiplier), flooding_send_nack,
               NULL);
  }

  struct flooding_pkt *p_recv = (struct flooding_pkt *)packetbuf_dataptr();

  switch (p_recv->msg_type) {
  case MESSAGE_TYPE_PAYLOAD:

    if (mode != MODE_SINK) {

      // only care for batch_ids greater/equal then oneself's
      if (local_batch_id <= p_recv->batch_id) {

        PRINT_DEBUG("\nreceived broadcast packet  (source: %d.%d)\n",
                    from->u8[0], from->u8[1]);
        PRINT_DEBUG("   --> batch-id: %d , packet_id: %u\n", p_recv->batch_id,
                    p_recv->pkt_id);
        gf_vec_print("   --> data", p_recv->payload, M);

        /* if we didn't receive the packet yet */
        if (!GET_BIT(rec, p_recv->pkt_id)) {
          process_data(p_recv);
        }
      }
    }
    break;
  case MESSAGE_TYPE_NACK:
    if (mode != MODE_SOURCE) {

      PRINT_DEBUG("received NACK message from %u (packet_id: %u)\n",
                  from->u8[0], p_recv->pkt_id);
      if (GET_BIT(rec, p_recv->pkt_id)) {
        // node can provide data
        pkt_nack_reply.batch_id = p_recv->batch_id;
        memcpy(pkt_nack_reply.payload, packet_queue[p_recv->pkt_id], M);
        pkt_nack_reply.pkt_id = p_recv->pkt_id;
        PRINT_DEBUG("@@@ waiting to send NACK reply\n");
        ctimer_set(&timer_nack_reply, DELAY_NACK_REPLY,
                   flooding_send_nack_reply, NULL);
      }
    }
    break;
  case MESSAGE_TYPE_NACK_REPLY:
    PRINT_DEBUG("\nreceived NACK_REPLY message  (source: %d.%d)\n", from->u8[0],
                from->u8[1]);
    PRINT_DEBUG("   --> batch-id: %d\n", p_recv->batch_id);
    gf_vec_print("   --> data", p_recv->payload, M);
    nack_reply_received = 1;
    if (mode != MODE_SINK && !check_recovered()) {
      process_data(p_recv);
    }

    break;
  }
}

void init_flooding(void) {

  /* init memory */
  packet_queue = malloc(K * sizeof(uint8_t *));
  uint8_t i;
  for (i = 0; i < K; i++) {
    packet_queue[i] = malloc(M);
  }

  clear_memory();

  /* each node looks up its mode */
  //uint8_t id_sink = ID_SINK;
  uint8_t id_sources[] = ID_SOURCES;
 // uint8_t id_relay_sources[] = ID_RELAY_SOURCES;
  if (node_id == ID_SINK) {
    mode = MODE_SINK;
    local_batch_id = 0xff;
  } else if (array_contains(node_id, id_sources, sizeof(id_sources))) {
    mode = MODE_SOURCE;
    /*if (array_contains(node_id, id_relay_sources, sizeof(id_relay_sources))) {
      mode = MODE_RELAY_PLUS_SOURCE;
    }*/
  } else {
    mode = MODE_RELAY;
  }

  PRINT_DEBUG("id: %u, mode: %u\n", node_id, mode);
  udp_broadcast_new(UDP_SERVER,NULL);

  //broadcast_open(&broadcast, 111, &broadcast_call);
}

void start_flooding(void) {

  if (mode == MODE_SINK) {
    send_new_batch(NULL);
  }
}

void send_new_batch(void *ptr) {

  ++local_batch_id;
  if (local_batch_id < NUMBER_OF_BATCHES - 1) {
    /* send new batch every INTERVAL_BATCH seconds */
    ctimer_set(&timer_batch, INTERVAL_BATCH, send_new_batch, NULL);
  }

  /* send new packet every INTERVAL_PKT seconds in current batch */
  ctimer_set(&timer_packet, INTERVAL_PKT, send_new_packet, &local_batch_id);
  counter_send_pkt = 0;
}

static uint8_t local_packet = 0x00;
void send_new_packet(void *bid) {

  uint8_t i;
  struct flooding_pkt p;
  p.msg_type = MESSAGE_TYPE_PAYLOAD;
  p.batch_id = *(uint8_t *)bid;
  for (i = 0; i < M; i++) {
#if GF == 2
    local_packet = gf_choose_random();
#else
    local_packet = GF_MOD(++local_packet);
#endif
    p.payload[i] = local_packet;
  }
  memcpy(packet_queue[counter_send_pkt], p.payload, M);
  p.pkt_id = counter_send_pkt++;
  rec = SET_BIT(rec, p.pkt_id);

  print_flooding_packet("SINK INITIAL BROADCAST", &p);
  gf_vec_print("   data", p.payload, M);

  packetbuf_copyfrom(&p, sizeof(p));
  //broadcast_send(&broadcast);
NETSTACK_NETWORK.output(&tsch_broadcast_address);
  /* K packets in one batch */
  if (counter_send_pkt < K) {
    ctimer_reset(&timer_packet);
  }
}

void process_data(struct flooding_pkt *p_recv) {

  if (local_batch_id < p_recv->batch_id) {
    if (!check_recovered() &&
        (mode == MODE_SOURCE )) { //|| mode == MODE_RELAY_PLUS_SOURCE
      printf("ERROR  --> batch %u not recovered\n", local_batch_id);
    }
    local_batch_id = p_recv->batch_id;
    clear_memory();
  }

  memcpy(packet_queue[p_recv->pkt_id], p_recv->payload, M);
  rec = SET_BIT(rec, p_recv->pkt_id);

  if (mode == MODE_RELAY ) { //|| mode == MODE_RELAY_PLUS_SOURCE
    flood_packet_delay(p_recv);
  }

  if (check_recovered()) {
    if (mode == MODE_SOURCE ) { // || mode == MODE_RELAY_PLUS_SOURCE

#if GF != 2
      if (check_data(packet_queue)) {
        printf("--> REC SUCCESSFULL (batch-id= %d)\n", local_batch_id);
#ifdef LEDS
        leds_on(LEDS_GREEN);
#endif
      } else {
        printf("--> REC NOT SUCCESSFULL (batch-id= %d)\n", local_batch_id);
#ifdef LEDS
        leds_on(LEDS_RED);
#endif
      }
#endif // GF != 2
      print_batch("recovered", packet_queue, local_batch_id);
    }
    ctimer_stop(&timer_nack);
  }
}
struct flooding_pkt packet;
void flood_packet_delay(struct flooding_pkt *p_recv) {

  packet.msg_type = MESSAGE_TYPE_PAYLOAD;
  packet.batch_id = p_recv->batch_id;
  packet.pkt_id = p_recv->pkt_id;
  memcpy(packet.payload, p_recv->payload, M);
  ctimer_set(&timer_send, DELAY_FLOODING_PKT(priority), flooding_broadcast,
             NULL);
  ++priority;
}

void flooding_send_nack(void *p) {

  struct flooding_pkt pkt_nack;
  pkt_nack.msg_type = MESSAGE_TYPE_NACK;
  pkt_nack.batch_id = local_batch_id;
  pkt_nack.pkt_id = get_empty_packet_id(); // TODO get ids
  PRINT_DEBUG("FLOODING BROADCAST - NACK\n");
  print_flooding_packet("FLOODING BROADCAST - NACK", &pkt_nack);
  packetbuf_copyfrom(&pkt_nack, sizeof(pkt_nack));
  //broadcast_send(&broadcast);
  NETSTACK_NETWORK.output(&tsch_broadcast_address);
  ctimer_set(&timer_nack, DELAY_NACK(nack_multiplier), flooding_send_nack,
             NULL);
  nack_multiplier *= 2;
}

void flooding_send_nack_reply(void *p) {
  if (!nack_reply_received) {

    pkt_nack_reply.msg_type = MESSAGE_TYPE_NACK_REPLY;

    PRINT_DEBUG("FLOODING BROADCAST - NACK REPLY\n");
    print_flooding_packet("FLOODING BROADCAST - NACK REPLY", &pkt_nack_reply);
    packetbuf_copyfrom(&pkt_nack_reply, sizeof(pkt_nack_reply));
    //broadcast_send(&broadcast);
	NETSTACK_NETWORK.output(&tsch_broadcast_address);
  } else {
    PRINT_DEBUG("discarding NACK REPLY\n");
    nack_reply_received = 0;
  }
}

void flooding_broadcast(void *p) {
  print_flooding_packet("FLOODING BROADCAST", &packet);
  packetbuf_copyfrom(&packet, sizeof(packet));
  NETSTACK_NETWORK.output(&tsch_broadcast_address);
  //broadcast_send(&broadcast);
  priority = 0;
}

int check_recovered() { return rec == ((1 << K) - 1); }

/* returns id of an not received packet */
int get_empty_packet_id() {

  uint8_t id = 0;
  while (GET_BIT(rec, id) != 0)
    ++id;

  return id;
}

void clear_memory(void) {

  uint8_t i;
  for (i = 0; i < K; i++) {
    memset(packet_queue[i], 0x00, M);
  }

  rec = 0;
#ifdef LEDS
  leds_off(LEDS_RED);
  leds_off(LEDS_GREEN);
#endif
}

void print_batch(const char *s, uint8_t **v, uint8_t id) {

  uint8_t i, j;
  PRINT_DEBUG("%s (batch-id= %d):\n", s, id);

  for (i = 0; i < K; i++) {
    PRINT_DEBUG("   --> packet_%d: [", i);
    for (j = 0; j < M; j++) {
      if (j != M - 1)
        PRINT_DEBUG("0x%x , ", v[i][j]);
      else
        PRINT_DEBUG("0x%x", v[i][j]);
    }
    PRINT_DEBUG("]\n");
  }

  PRINT_DEBUG("\n");
}

/* checks if the recovered data is correct;
  incrementing data for debugging purposes */
uint8_t check_data(uint8_t **v) {

  uint8_t ref = v[0][0];
  uint8_t i, j;

  for (i = 0; i < K; i++) {
    for (j = 0; j < M; j++) {
      if (v[i][j] != GF_MOD(ref++)) {
        return 0;
      }
    }
  }

  return 1;
}

void print_flooding_packet(const char *s, struct flooding_pkt *p) {

  PRINT_DEBUG("--> %s (batch-id= %u, packet-id= %u)\n", s, p->batch_id,
              p->pkt_id);
  gf_vec_print("   data", p->payload, M);
}
