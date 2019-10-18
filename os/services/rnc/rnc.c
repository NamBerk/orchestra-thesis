

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
#include "net/ipv6/simple-udp.h"
#include "net/ipv6/uip.h"

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
static struct ctimer timer_nack;


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


static struct simple_udp_connection broadcast_connection;
/*the callback function of created UDP connection. Upon the reception of the packets the "receiver function is called"*/
void receiver(struct simple_udp_connection *c,
			  const uip_ipaddr_t *from,
			  uint16_t sender_port,
			  const uip_ipaddr_t *receiver_addr,
			  uint16_t receiver_port,
			  const uint8_t *data,
			  uint16_t datalen
				)
 { 
	
 
  struct rnc_pkt *p_recv = (struct rnc_pkt *)packetbuf_dataptr();

    if (node_id != 1) { 
	

      // only care for batch_ids greater/equal then oneself's
      if (local_batch_id <= p_recv->batch_id) {

        PRINT_DEMO("\nreceived RNC message  (source: %d.%d)\n", from->u8[0],
                   from->u8[1]);
        PRINT_DEMO("   --> batch-id: %d\n", p_recv->batch_id);
        gf_vec_print("   --> data", p_recv->payload, M);
        gf_vec_print("   --> coeffs", p_recv->coeff, K);

        process_data(p_recv);
		printf("passed process data \n");
      }
    }
 
}

/*--------------------------------------------------------------------------------------------*/

void init_rnc(void) { /*initiliation of the Random Linear Network Coding*/

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
  if (node_id == 1) {     /////
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
  
  PRINT_DEMO("id: %u, mode: %u\n", node_id, mode);
  /*create a udp connection */
	simple_udp_register(&broadcast_connection,UDP_SERVER,NULL,UDP_SERVER,receiver);
  
}

void start_rnc(void) {

  if (node_id == 1) { /*only sink node (where the node id is 1) could start sending the packets*/
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
  printf("send new packet \n");
  counter_send_pkt = 0;
}

static uint8_t local_packet = 0x00;
void send_new_packet(void *bid) {

  uint8_t i;
  struct rnc_pkt p;
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

  /* source sends uncoded packets */
  generate_init_coeffs(counter_send_pkt);
  seen_packets = SET_BIT(seen_packets, counter_send_pkt);
  memcpy(p.coeff, coeff_matrix[counter_send_pkt++], K);

  print_rnc_packet("SINK INITIAL BROADCAST", &p); /////
  packetbuf_copyfrom(&p, sizeof(p));
  queuebuf_new_from_packetbuf();
  NETSTACK_NETWORK.output(NULL); /*broadcast the packets in the packet buffer*/
  

  /* K packets in one batch */
  if (counter_send_pkt < K) {
    ctimer_reset(&timer_packet);
  }
}

void generate_init_coeffs(uint8_t dia) { /*generation of the random coefficients*/

  memset(coeff_matrix[dia], 0x00, K);
  coeff_matrix[dia][dia] = 0x01;
}

void process_data(struct rnc_pkt *p_recv) {
printf("got inside processs data \n");
  if (local_batch_id < p_recv->batch_id) {
	  
    if (rank_coeff_matrix != FULL_RANK &&
        (mode == MODE_SOURCE)){ 
      printf("ERROR  --> batch %u not recovered\n", local_batch_id);
    }
    local_batch_id = p_recv->batch_id;
    clear_memory();
  }
  /* only process data if not fully received all packets */
  if (rank_coeff_matrix != FULL_RANK) {
printf("rank \n");
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

    if (node_id != 1 ) {
      if (rank_old < rank_coeff_matrix) {
        /* received new information, share them */
		printf("ssssss \n");
        rnc_send_pkt_delay(p_recv->batch_id);
      }
    }

    if (rank_coeff_matrix == FULL_RANK) {
      if (mode == MODE_SOURCE ) { 
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

      ctimer_stop(&timer_nack);
    }
  }
}

/* generate a random linear combination of all packets in the packet_queue */
void rnc_generate_payload(struct rnc_pkt *p, uint8_t batch_id) {
	printf("generate payload \n");
  p->batch_id = batch_id;
  uint8_t b[K];
  gf_vec_choose_random(b, K);
  gf_vec_dot_matrix(p->payload, b, packet_queue, K, M);
  gf_vec_dot_matrix(p->coeff, b, coeff_matrix, K, K);
}

static struct rnc_pkt packet;
void rnc_send_pkt_delay(uint8_t batch_id) {

  packet.msg_type = MESSAGE_TYPE_PAYLOAD;
  rnc_generate_payload(&packet, batch_id);
  ctimer_set(&timer_send, DELAY_RNC_PKT(priority), rnc_broadcast, NULL);
  ++priority;
}

void rnc_broadcast(void *p) {
 print_rnc_packet("RNC BROADCAST", &packet);
  packetbuf_copyfrom(&packet, sizeof(packet));
  queuebuf_new_from_packetbuf();
 NETSTACK_NETWORK.output(NULL); /*broadcast*/

  priority = 0;
}

// returns index where packet was stored
uint8_t store_coeffs_and_payload(struct rnc_pkt *p_recv) {

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

void print_rnc_packet(const char *s, struct rnc_pkt *p) {

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
