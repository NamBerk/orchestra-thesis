/*
SEEMOO - Secure Mobile Networking Lab and Project, Winter 2017/18
author: Jan Sturm
*/

#include "./util.h"

/* indicates if arr contains val */
uint8_t array_contains(uint8_t val, uint8_t *arr, uint8_t len) {
  uint8_t i;
  for (i = 0; i < len; i++) {
    if (arr[i] == val)
      return 1;
  }
  return 0;
}

/* returns random number in [a,b) */
uint16_t get_random_between(uint16_t a, uint16_t b) {

  return a + (random_rand() % b);
}
