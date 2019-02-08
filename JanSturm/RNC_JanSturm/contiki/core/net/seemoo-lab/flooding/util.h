/*
SEEMOO - Secure Mobile Networking Lab and Project, Winter 2017/18
author: Jan Sturm
*/

#ifndef FLOODING_UTIL_H_
#define FLOODING_UTIL_H_

#include "./params.h"
#include "random.h"
#include <stdint.h>

#ifdef DEBUG
#define PRINT_DEBUG(...) printf(__VA_ARGS__)
#else
#define PRINT_DEBUG(...)
#endif

#define GET_BIT(X,N) (((X)>>(N))&1)
#define SET_BIT(X,N) ((X)|(1<<(N)))

uint8_t array_contains(uint8_t val, uint8_t *arr, uint8_t len);
uint16_t get_random_between(uint16_t a, uint16_t b);
#endif //UTIL
