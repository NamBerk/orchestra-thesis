

#include "./util.h"

void init_bytevec(uint8_t *v) { memset(v, 0x00, M); }

void bytevec_copy(uint8_t *r, uint8_t *v) { memcpy(r, v, M); }

/* indicates if arr contains val */
uint8_t array_contains(uint8_t val, uint8_t *arr, uint8_t len) {
  uint8_t i;
  for (i = 0; i < len; i++) {
    if (arr[i] == val)
      return 1;
  }
  return 0;
}

/* determine how many leading zeros are in the row vector
  return 0xff if all 0
 */
uint8_t get_zeros(uint8_t *v, uint8_t n) {
  uint8_t i;
  uint8_t zeros = 0x00;
  for (i = 0; i < n; i++) {
    if (v[i] == 0x00)
      ++zeros;
    else
      return zeros;
  }
  return 0xff;
}

/* returns random number in [a,b) */
uint16_t get_random_between(uint16_t a, uint16_t b) {

  return a + (random_rand() % b);
}
