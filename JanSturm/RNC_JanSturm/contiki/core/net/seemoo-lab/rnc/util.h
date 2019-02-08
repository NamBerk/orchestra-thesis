/*
SEEMOO - Secure Mobile Networking Lab and Project, Winter 2017/18
author: Jan Sturm
*/

#ifndef RNC_UTIL_H_
#define RNC_UTIL_H_

#include <stdint.h>
#include "random.h"
#include "./params.h"

#ifdef DEBUG
#define PRINT_DEBUG(...) printf(__VA_ARGS__)
#define PRINT_DEMO(...) printf(__VA_ARGS__)
#else
#ifdef DEMO
#define PRINT_DEMO(...) printf(__VA_ARGS__)
#else
#define PRINT_DEMO(...)
#endif
#define PRINT_DEBUG(...)
#endif

#define GET_BIT(X,N) (((X)>>(N))&1)
#define SET_BIT(X,N) ((X)|(1<<(N)))

uint8_t get_zeros(uint8_t *v, uint8_t n);
void init_bytevec(uint8_t *v);
void bytevec_copy(uint8_t*r,uint8_t *v);
uint8_t array_contains(uint8_t val, uint8_t *arr, uint8_t len);
uint16_t get_random_between(uint16_t a, uint16_t b);

#endif
