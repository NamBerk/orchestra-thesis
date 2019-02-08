/*
SEEMOO - Secure Mobile Networking Lab and Project, Winter 2017/18
author: Jan Sturm
*/

#include "gf.h"

void gf_print_byte(char *c, uint8_t b) { PRINT_DEBUG("%s: 0x%x\n", c, b); }

uint8_t gf_choose_random() {
#if GF == 2 //(uint8_t)random_rand() returns alternating odd and even values
  random_init(random_rand() >> 1);
#endif
  return GF_MOD((uint8_t)random_rand());
}


void gf_matrix_print(char *c, uint8_t **mat, uint8_t dim_r, uint8_t dim_c) {
  uint8_t i, j;
  PRINT_DEBUG("%s:\n   ", c);
  for (i = 0; i < dim_r; i++) {
    for (j = 0; j < dim_c; j++) {
      if (j != dim_c - 1)
        PRINT_DEBUG("0x%x , ", mat[i][j]);
      else
        PRINT_DEBUG("0x%x", mat[i][j]);
    }
    PRINT_DEBUG("\n   ");
  }
  PRINT_DEBUG("\n");
}


void gf_vec_print(char *c, uint8_t *vec, uint8_t dim) {

  uint8_t i;
  PRINT_DEBUG("%s: [", c);
  for (i = 0; i < dim; i++) {
    if (i != dim - 1)
      PRINT_DEBUG("0x%x , ", vec[i]);
    else
      PRINT_DEBUG("0x%x", vec[i]);
  }
  PRINT_DEBUG("]\n");
}
