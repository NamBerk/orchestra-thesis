/*
SEEMOO - Secure Mobile Networking Lab and Project, Winter 2017/18
author: Jan Sturm
*/

#include "gf.h"
#include "sys/node-id.h"


void swap_row(uint8_t **A, uint8_t **b, uint8_t r1, uint8_t r2) {

  if (r1 == r2)
    return;

  uint8_t *p;

  p = A[r1];
  A[r1] = A[r2];
  A[r2] = p;

  p = b[r1];
  b[r1] = b[r2];
  b[r2] = p;

}

uint8_t gaussian_elimination_iter(uint8_t **A, uint8_t **x, uint8_t **b,
                                  uint8_t c, uint8_t index_new, uint8_t mode) {

  uint8_t i, col, row, max_row, dia;
  uint8_t max, tmp, inv;
  uint8_t *tmp_v = malloc(M);

  /* start computing at row index_new since we inserted the new data there */
  for (dia = index_new; dia < c; dia++) {
    max_row = dia, max = A[dia][dia];

    // find biggest element from rows in col <dia>
    for (row = dia + 1; row < c; row++)
      if (A[row][dia] > max)
        max_row = row, max = A[row][dia];

    swap_row(A, b, dia, max_row);

    if (A[dia][dia] != 0) { // avoid divide by zero
      inv = gf_inv(A[dia][dia]);
      // make the rest in col equal to 0
      for (row = dia + 1; row < c; row++) {
        if (A[row][dia] != 0) { // skip zeros
          tmp = gf_mul(A[row][dia], inv);
          for (col = dia + 1; col < c; col++)
            A[row][col] = gf_add(A[row][col], gf_mul(tmp, A[dia][col]));
          A[row][dia] = 0;
          gf_byte_mul_vec(tmp_v, tmp, b[dia]);
          gf_vec_add(b[row], b[row], tmp_v);
        }
      }
    } else {
      // delete row, does not happen often
      memset(A[dia], 0x00, c);
      memset(b[dia], 0x00, M);
    }
  }

  uint8_t rank = gf_matrix_rank(A, K);

  // only sinks recover packets and only if we have a full rank coefficient
  // matrix
  if ((node_id==7 && node_id==6 /*|| mode == MODE_RELAY_PLUS_SINK*/) && //// change should be done accroding to the source nodes as 
      rank == FULL_RANK) { 	                                 //// in our case the dest is source nodes
    uint8_t *tmp_b = malloc(M);

    /* back substitution */
    for (row = c - 1; row != 0xff; row--) {

      bytevec_copy(tmp_b, b[row]);

      for (i = c - 1; i > row; i--) {
        gf_byte_mul_vec(tmp_v, A[row][i], x[i]);
        gf_vec_add(tmp_b, tmp_b, tmp_v);
      }
      gf_byte_mul_vec(x[row], gf_inv(A[row][row]), tmp_b);
    }
    free(tmp_b);
  }
  free(tmp_v);

  return rank;
}
