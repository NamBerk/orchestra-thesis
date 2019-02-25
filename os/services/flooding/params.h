/*
SEEMOO - Secure Mobile Networking Lab and Project, Winter 2017/18
author: Jan Sturm
*/

#ifndef FLOODING_PARAMS_H_
#define FLOODING_PARAMS_H_

/* uncomment for additional debug/demo messages */
//#define DEBUG
//#define DEMO

/* turn on LED if a batch is fully recovered*/
#define LEDS

/* uncomment to measure power*/
//#define POWERTRACE

/* The node id of the initiator */
#define ID_SINK	1

/* how many batches the source should send out */
#define NUMBER_OF_BATCHES 1

/* The node ids of the sinks */
#define ID_SOURCES {7,6}
/* if sink is relay aswell, add here too */
#define ID_RELAY_SINKS {}

/* choose between GF(256), GF(16) and GF(2) */
#define GF 2

/* batch size */
#define K 8

/* payload size in bytes */
#define M 16


/* ######################## TIMEOUT values #########################*/

/* high enough to let nodes all recover current batch */
#define INTERVAL_BATCH (CLOCK_SECOND * K)
#define INTERVAL_PKT (CLOCK_SECOND >> 2)

#define DELAY_FLOODING_PKT(prio) (get_random_between(CLOCK_SECOND >> (3+prio), CLOCK_SECOND >> (2+prio)))
#define DELAY_NACK(mul) (get_random_between(MIN(2,mul)*(CLOCK_SECOND >> 1),MIN(2,mul)*(CLOCK_SECOND >> 0)))
#define DELAY_NACK_REPLY (get_random_between(CLOCK_SECOND >> 3, CLOCK_SECOND >> 2))


/* ######################## don't change after here ################*/
#define MODE_SOURCE 0
#define MODE_RELAY 1
#define MODE_SINK 2
#define MODE_RELAY_PLUS_SINK 3

#define MESSAGE_TYPE_PAYLOAD 0
#define MESSAGE_TYPE_NACK 1
#define MESSAGE_TYPE_NACK_REPLY 2

#endif
