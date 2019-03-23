/*
SEEMOO - Secure Mobile Networking Lab and Project, Winter 2017/18
author: Jan Sturm
*/

#include "contiki.h"
#include "os/net/netstack.h"
#include "./gauss.h"
#include "os/lib/random.h"
#include "./rnc.h"
#include "sys/ctimer.h"
#include <stdlib.h>
#include "net/linkaddr.h"
#include "net/packetbuf.h"
#include "stdio.h"
#include "os/sys/node-id.h"
#include "net/ipv6/tcpip.h"
#include "net/queuebuf.h"

#ifdef LEDS
#include "leds.h"
#endif

/* mode can be set in params.h */
static uint8_t mode;

/* batch that is currently operated on */
static uint8_t local_batch_id = 0;

/* various callback timers for timeouts */
static struct ctimer timer_batch;
static struct ctimer timer_packet;
static struct ctimer timer_send;
//static struct ctimer timer_nack;
//static struct ctimer timer_nack_reply;

/* for lazy NACK */
//static uint8_t nack_multiplier = 1;

// count how much the source already sent in one batch
static uint8_t counter_send_pkt = 0;

static uint8_t **coeff_matrix; // KxK
static uint8_t rank_coeff_matrix = 0;
static uint8_t rank_old = 0;
static uint8_t **packet_queue; // KxM
static uint8_t **recovered;    // KxM

#define UDP_CLIENT	8765
#define UDP_SERVER	5678

/*
  indicates which packets are already received (encoded), one bit for each
  packet.
  we assume K<=8 so it fits in to an uint8_t
*/
static uint8_t seen_packets = 0;

/* give nodes priority in broadcasting if they didn't sent a long time */
static uint8_t priority = 0;

/* indicates if a NACK_REPLY message was already received */
//static uint8_t nack_reply_received = 0;

//static const struct broadcast_callbacks broadcast_call = {broadcast_recv};

//static struct uip_udp_conn udp_conn;
//static struct broadcast_callback current_callback = {receiver};


void receiver(const void *data, uint16_t len,
  const linkaddr_t *from, const linkaddr_t *dest)
 { 
	
  /*if (mode != MODE_SINK && rank_coeff_matrix != FULL_RANK) { //
    nack_multiplier = 1;
    ctimer_set(&timer_nack, DELAY_NACK(nack_multiplier), rnc_send_nack, NULL);
  }*/
  struct tsch_packet *p_recv = (struct tsch_packet *)packetbuf_dataptr();

 // switch (p_recv->msg_type) {
  //case MESSAGE_TYPE_PAYLOAD:
    if (node_id != 1) { //////////
	

      // only care for batch_ids greater/equal then oneself's
      if (local_batch_id <= p_recv->batch_id) {

        PRINT_DEMO("\nreceived RNC message  (source: %d.%d)\n", from->u8[0],
                   from->u8[1]);
        PRINT_DEMO("   --> batch-id: %d\n", p_recv->batch_id);
        gf_vec_print("   --> data", p_recv->payload, M);
        gf_vec_print("   --> coeffs", p_recv->coeff, K);

        process_data(p_recv);
		//printf("aaaaaaa \n");
      }
    }
    //break;
/*
  case MESSAGE_TYPE_NACK:
    if (mode != MODE_SINK) {  ////////////
      PRINT_DEBUG("\nreceived NACK  (source: %d.%d)\n", from->u8[0],
                  from->u8[1]);
      if (seen_packets & p_recv->coeff[0]) {
        // node can provide data
        PRINT_DEBUG("@@@ waiting to send NACK_REPLY\n");
        ctimer_set(&timer_nack_reply, DELAY_NACK_REPLY, rnc_send_nack_reply,
                   NULL);
      }
    }

    break;

  case MESSAGE_TYPE_NACK_REPLY:   //////////
    PRINT_DEMO("\nreceived NACK_REPLY message  (source: %d.%d)\n", from->u8[0],
               from->u8[1]);
    PRINT_DEMO("   --> batch-id: %d\n", p_recv->batch_id);
    gf_vec_print("   --> data", p_recv->payload, M);
    gf_vec_print("   --> coeffs", p_recv->coeff, K);
    nack_reply_received = 1;
    if (mode != MODE_SINK && rank_coeff_matrix != FULL_RANK) { //////
      process_data(p_recv);
    }

    break;
  }*/
}

/*--------------------------------------------------------------------------------------------*/

void init_rnc(void) { // should be altered according to my implementation

  /* init memory */
  coeff_matrix = malloc(K * sizeof(uint8_t *));
  packet_queue = malloc(K * sizeof(uint8_t *));
  recovered = malloc(K * sizeof(uint8_t *));
  uint8_t i;
  for (i = 0; i < K; i++) {
    coeff_matrix[i] = malloc(K);
    packet_queue[i] = malloc(M);
    recovered[i] = malloc(M);
  }

  clear_memory();

  /* each node looks up its mode */
  uint8_t id_sources[] = ID_SOURCES; /////
  /*uint8_t id_relay_sinks[] = ID_RELAY_SINKS;*/ //////
  if (node_id == ID_SINK) {     /////
    mode = MODE_SINK;
    local_batch_id = 0xff;
  } else if (array_contains(node_id, id_sources, sizeof(id_sources))) { //////
    mode = MODE_SOURCE;
    /*if (array_contains(node_id, id_relay_sinks, sizeof(id_relay_sinks))) {
      mode = MODE_RELAY_PLUS_SINK;
    }*/  ///////
  } else {
    mode = MODE_RELAY;
  }
  
  PRINT_DEBUG("id: %u, mode: %u\n", node_id, mode);
	struct uip_udp_conn *udp_conn=udp_broadcast_new(UDP_SERVER, NULL);
	udp_bind(udp_conn,UDP_SERVER);
	//simple_udp_register(udp_conn,UDP_SERVER,NULL,UDP_CLIENT,receiver);
  //broadcast_open(&broadcast, 111, &broadcast_call);
}

void start_rnc(void) {

  if (node_id == 1) { /////
    send_new_batch(NULL);
  }
}

void send_new_batch(void *ptr) {
  seen_packets = 0;
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
  struct tsch_packet p;
 // p.msg_type = MESSAGE_TYPE_PAYLOAD;
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

  /* source sends uncoded packets */
  generate_init_coeffs(counter_send_pkt);
  seen_packets = SET_BIT(seen_packets, counter_send_pkt);
  memcpy(p.coeff, coeff_matrix[counter_send_pkt++], K);

  print_rnc_packet("SINK INITIAL BROADCAST", &p); /////
  packetbuf_copyfrom(&p, sizeof(p));
  queuebuf_new_from_packetbuf();
  //broadcast_send(&broadcast);
  //simple_udp_sendto(&broadcast_connection, str, strlen(str), &linkaddr_null);
  NETSTACK_NETWORK.output(&linkaddr_null);
  //NETSTACK_MAC.send(NULL,NULL);

  /* K packets in one batch */
  if (counter_send_pkt < K) {
    ctimer_reset(&timer_packet);
  }
}

void generate_init_coeffs(uint8_t dia) {

  memset(coeff_matrix[dia], 0x00, K);
  coeff_matrix[dia][dia] = 0x01;
}

void process_data(struct tsch_packet *p_recv) {

  if (local_batch_id < p_recv->batch_id) {
	  
    if (rank_coeff_matrix != FULL_RANK &&
        (node_id == 7 || node_id == 6 )){ // || mode == MODE_RELAY_PLUS_SINK
      printf("ERROR  --> batch %u not recovered\n", local_batch_id);
    }
    local_batch_id = p_recv->batch_id;
    clear_memory();
  }
  /* only process data if not fully received all packets */
  if (rank_coeff_matrix != FULL_RANK) {

    rank_old = rank_coeff_matrix;
    uint8_t index_store = store_coeffs_and_payload(p_recv);

    /*
        A*x = b , solve for x iteratively
        A = coeff_matrix
        x = recovered
        b = packet_queue
    */
    rank_coeff_matrix = gaussian_elimination_iter(
        coeff_matrix, recovered, packet_queue, K, index_store, mode);

    /* update seen packets */
    uint8_t i;
    for (i = 0; i < K; i++) {
      if (coeff_matrix[i][i] != 0) {
        seen_packets = SET_BIT(seen_packets, i);
      }
    }

    PRINT_DEMO("rank changed: %s  (%u --> %u)\n",
               rank_old < rank_coeff_matrix ? "yes" : "no", rank_old,
               rank_coeff_matrix);

    print_lse(coeff_matrix, recovered, packet_queue);

    if (mode == MODE_RELAY /*|| mode == MODE_RELAY_PLUS_SINK*/) {
      if (rank_old < rank_coeff_matrix) {
        /* received new information, share them */
        rnc_send_pkt_delay(p_recv->batch_id);
      }
    }

    if (rank_coeff_matrix == FULL_RANK) {
      if (node_id == 7 || node_id == 6 ) { ///////|| mode == MODE_RELAY_PLUS_SINK
#if GF != 2
        if (check_data(recovered)) {
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
        print_batch("recovered", recovered, local_batch_id);
      }

      //ctimer_stop(&timer_nack);
    }
  }
}

/* generate a random linear combination of all packets in the packet_queue */
void rnc_generate_payload(struct tsch_packet *p, uint8_t batch_id) {

  p->batch_id = batch_id;
  uint8_t b[K];
  gf_vec_choose_random(b, K);
  gf_vec_dot_matrix(p->payload, b, packet_queue, K, M);
  gf_vec_dot_matrix(p->coeff, b, coeff_matrix, K, K);
}

static struct tsch_packet packet;
void rnc_send_pkt_delay(uint8_t batch_id) {

  //packet.msg_type = MESSAGE_TYPE_PAYLOAD;
  rnc_generate_payload(&packet, batch_id);
  ctimer_set(&timer_send, DELAY_RNC_PKT(priority), rnc_broadcast, NULL);
  ++priority;
}

/*void rnc_send_nack(void *p) {

  // TODO use separate struct --> not implemented in this lab
  struct rnc_pkt pkt_nack;
  pkt_nack.msg_type = MESSAGE_TYPE_NACK;
  pkt_nack.batch_id = local_batch_id;

   indicate which packets are missing
     --> store it in first coefficient
  
  pkt_nack.coeff[0] = 0x00;
  uint8_t i;
  for (i = 0; i < K; i++) {
    if (!coeff_matrix[i][i]) {
      pkt_nack.coeff[0] = SET_BIT(pkt_nack.coeff[0], i);
    }
  }

  PRINT_DEBUG("RNC BROADCAST - NACK\n");
  print_rnc_packet("RNC BROADCAST - NACK", &pkt_nack);
  packetbuf_copyfrom(&pkt_nack, sizeof(pkt_nack));
  //broadcast_send(&broadcast);
  NETSTACK_NETWORK.output(&linkaddr_null);
 // NETSTACK_MAC.send(NULL,NULL);
  ctimer_set(&timer_nack, DELAY_NACK(nack_multiplier), rnc_send_nack, NULL);

   lazy NACK 
  nack_multiplier *= 2;
}
 */
/*
void rnc_send_nack_reply(void *p) {
   only send NACK-REPLY if nobody else sent one so far 
  if (!nack_reply_received) {
    struct rnc_pkt pkt_nack_reply;
    pkt_nack_reply.msg_type = MESSAGE_TYPE_NACK_REPLY;
    rnc_generate_payload(&pkt_nack_reply, local_batch_id);

    PRINT_DEBUG("RNC BROADCAST - NACK REPLY\n");
    print_rnc_packet("RNC BROADCAST - NACK REPLY", &pkt_nack_reply);
    packetbuf_copyfrom(&pkt_nack_reply, sizeof(pkt_nack_reply));
	NETSTACK_NETWORK.output(&linkaddr_null);
    //broadcast_send(&broadcast);
	//NETSTACK_MAC.send(NULL,NULL);
  } else {
    PRINT_DEBUG("discarding NACK REPLY\n");
    nack_reply_received = 0;
  }
}*/

void rnc_broadcast(void *p) {
  print_rnc_packet("RNC BROADCAST", &packet);
  packetbuf_copyfrom(&packet, sizeof(packet));
  queuebuf_new_from_packetbuf();
  NETSTACK_NETWORK.output(&linkaddr_null);
 // broadcast_send(&broadcast);
 //NETSTACK_MAC.send(NULL,NULL);
  priority = 0;
}

// returns index where packet was stored
uint8_t store_coeffs_and_payload(struct tsch_packet *p_recv) {

  /* how many leading zeros the coefficient vector contains */
  uint8_t index_zero = get_zeros(p_recv->coeff, K);
  uint8_t zeros[K];
  memset(zeros, 0x00, K);
  uint8_t tmp_coeff[K];
  uint8_t insert_coeff[K];
  memcpy(insert_coeff, p_recv->coeff, K);
  uint8_t *tmp_data = malloc(M);
  uint8_t *insert_data = malloc(M);
  memcpy(insert_data, p_recv->payload, M);

  uint8_t i;

  /* check if coeff_matrix is zero at index_zero. Else, insert data there and
    move others with same strategy down
  */
  for (i = index_zero; i < K; i++) {
    if (memcmp(coeff_matrix[i], zeros, K) == 0) {
      memcpy(coeff_matrix[i], insert_coeff, K);
      memcpy(packet_queue[i], insert_data, M);
      free(tmp_data);
      free(insert_data);
      return index_zero;

    } else {

      memcpy(tmp_coeff, coeff_matrix[i], K);
      memcpy(tmp_data, packet_queue[i], M);
      memcpy(coeff_matrix[i], insert_coeff, K);
      memcpy(packet_queue[i], insert_data, M);
      memcpy(insert_coeff, tmp_coeff, K);
      memcpy(insert_data, tmp_data, M);
    }
  }
  free(tmp_data);
  free(insert_data);
  return 0;
}

void clear_memory(void) {

  uint8_t i;
  for (i = 0; i < K; i++) {
    memset(packet_queue[i], 0x00, M);
    memset(recovered[i], 0x00, M);
    memset(coeff_matrix[i], 0x00, K);
  }
  rank_coeff_matrix = 0;
  seen_packets = 0;

#ifdef LEDS
  leds_off(LEDS_RED);
  leds_off(LEDS_GREEN);
#endif
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

void print_batch(const char *s, uint8_t **v, uint8_t id) {

  uint8_t i, j;
  PRINT_DEMO("%s (batch-id= %d):\n", s, id);

  for (i = 0; i < K; i++) {
    PRINT_DEMO("   --> packet_%d: [", i);
    for (j = 0; j < M; j++) {
      if (j != M - 1)
        PRINT_DEMO("0x%x , ", v[i][j]);
      else
        PRINT_DEMO("0x%x", v[i][j]);
    }
    PRINT_DEMO("]\n");
  }

  PRINT_DEMO("\n");
}

void print_rnc_packet(const char *s, struct tsch_packet *p) {

  PRINT_DEMO("--> %s (batch-id= %u)\n", s, p->batch_id);
  gf_vec_print("   data", p->payload, M);
  gf_vec_print("   coeffs", p->coeff, K);
}

/* prints the whole LSE A*x=b */
void print_lse(uint8_t **mat, uint8_t **rec, uint8_t **queue) {

  uint8_t i, j;
  for (i = 0; i < K; i++) {
    for (j = 0; j < K; j++) {

      if (mat[i][j] < 16) {
        if (j != K - 1)
          PRINT_DEMO("0x0%x, ", mat[i][j]);
        else
          PRINT_DEMO("0x0%x         ", mat[i][j]);
      } else {
        if (j != K - 1)
          PRINT_DEMO("0x%x, ", mat[i][j]);
        else
          PRINT_DEMO("0x%x         ", mat[i][j]);
      }
    }

    for (j = 0; j < M; j++) {

      if (rec[i][j] < 16) {
        if (j != M - 1)
          PRINT_DEMO("0x0%x, ", rec[i][j]);
        else
          PRINT_DEMO("0x0%x         ", rec[i][j]);
      } else {
        if (j != M - 1)
          PRINT_DEMO("0x%x, ", rec[i][j]);
        else
          PRINT_DEMO("0x%x         ", rec[i][j]);
      }
    }

    for (j = 0; j < M; j++) {
      if (queue[i][j] < 16) {
        if (j != M - 1)
          PRINT_DEMO("0x0%x, ", queue[i][j]);
        else
          PRINT_DEMO("0x0%x", queue[i][j]);
      } else {
        if (j != M - 1)
          PRINT_DEMO("0x%x, ", queue[i][j]);
        else
          PRINT_DEMO("0x%x", queue[i][j]);
      }
    }
    PRINT_DEMO("\n");
  }
  PRINT_DEMO("\n");
}
