
#include "./gf.h"
#include "./gf_precomp.h"

uint8_t gf_add(uint8_t a, uint8_t b) { return a ^ b; }

void gf_vec_add(uint8_t *r, uint8_t *a, uint8_t *b) {

  uint8_t i;
  for (i = 0; i < M; i++)
    r[i] = gf_add(a[i], b[i]);
}

/* fast modular multiplication by table-lookups */
uint8_t gf_mul_precomp(uint8_t a, uint8_t b) {

  if (a == 0x00 || b == 0x00)
    return 0x00;
#if GF == 256
  uint16_t x = gf256_precomp_log[a];
  uint16_t y = gf256_precomp_log[b];
  return gf256_precomp_antilog[MOD255(x + y)];

#elif GF == 16
  uint16_t x = gf16_precomp_log[a];
  uint16_t y = gf16_precomp_log[b];
  return gf16_precomp_antilog[MOD15(x + y)];
#endif

return 0;
}

/* modular multiplicattion by computation */
uint8_t gf_mul_comp(uint8_t a, uint8_t b) {

#if GF == 256
  /* primitive polynomial = x^8 + x^4 + x^3 + x^1 + 1 = 0x11b
     --> see AES
     code from
     https://en.wikipedia.org/wiki/Finite_field_arithmetic#C_programming_example
  */
  uint8_t res = 0;
  while (a && b) {
    if (b & 1)
      res ^= a;
    if (a & 0x80)
      a = (a << 1) ^ 0x11b;
    else
      a <<= 1;
    b >>= 1;
  }
  return res;

#elif GF == 16
  /* primitive polynoial = x^4 + x + 1 = 0x13
     primitive polynomial from
     http://web.eecs.utk.edu/~plank/plank/papers/CS-07-593/primitive-polynomial-table.txt
  */
  uint8_t res = 0;
  while (a && b) {
    if (b & 1)
      res ^= a;
    if (a & 0x08)
      a = (a << 1) ^ 0x13;
    else
      a <<= 1;
    b >>= 1;
  }
  return res;

#elif GF == 2
  return a & b;
#endif
}

/* bytewise multiplicaton of vector b with byte a */
void gf_byte_mul_vec(uint8_t *r, uint8_t a, uint8_t *b) {

  uint8_t i;
  for (i = 0; i < M; i++)
    r[i] = gf_mul(a, b[i]);
}

/* fast modular inverse by table-lookups */
uint8_t gf_inv_precomp(uint8_t a) {

  if (a == 0x00 || a == 0x01)
    return a;
#if GF == 256
  uint8_t x = gf256_precomp_log[a];
  return gf256_precomp_antilog[0xff - x];

#elif GF == 16
  uint8_t x = gf16_precomp_log[a];
  return gf16_precomp_antilog[0x0f - x];
#endif

return 0;
}

/* modular inverse by computation */
uint8_t gf_inv_comp(uint8_t a) {

#if GF == 256
  /*computing a^254 using exponentiation by squaring*/
  uint8_t j, b = a;
  for (j = 14; --j;)
    b = gf_mul_comp(b, j & 1 ? b : a);
  return b;

#elif GF == 16
  /*computing a^14 using exponentiation by squaring*/
  uint8_t j, b = a;
  for (j = 6; --j;)
    b = gf_mul_comp(b, j & 1 ? b : a);
  return b;
#elif GF == 2
  return a;
#endif
}

void gf_print_byte(char *c, uint8_t b) { PRINT_DEBUG("%s: 0x%x\n", c, b); }

uint8_t gf_choose_random() {
#if GF == 2
  /* (uint8_t)random_rand() returns alternating odd and even values */
  random_init(random_rand() >> 1);
#endif
  return GF_MOD((uint8_t)random_rand());
}

/* generate a random vector of bytes */
void gf_vec_choose_random(uint8_t *v, uint8_t n) {

  uint8_t i;
  for (i = 0; i < n; i++)
    v[i] = gf_choose_random();
}

/* returns the dot product of two vectors */
uint8_t gf_vec_dot_vec(uint8_t *a, uint8_t *b, uint8_t n) {

  uint8_t res = 0;
  uint8_t i;
  for (i = 0; i < n; i++)
    res = gf_add(res, gf_mul(a[i], b[i]));

  return res;
}

/* calculate row-vector times matrix
*  res = vec * mat
*/
void gf_vec_dot_matrix(uint8_t *res, uint8_t *vec, uint8_t **mat, uint8_t row,
                       uint8_t col) {
  uint8_t i, j;
  for (i = 0; i < col; i++) {
    res[i] = 0;
    for (j = 0; j < row; j++)
      res[i] = gf_add(res[i], gf_mul(vec[j], mat[j][i]));
  }
}

/* returns the rank of matrix A, on which gaussian elimination was already
 * performed */
uint8_t gf_matrix_rank(uint8_t **A, uint8_t n) {

  uint8_t i, rank = 0, zero[n];
  memset(zero, 0x00, n);

  for (i = 0; i < n; i++) {
    if (memcmp(A[i], zero, n))
      ++rank;
  }
  return rank;
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
  PRINT_DEMO("%s: [", c);
  for (i = 0; i < dim; i++) {
    if (i != dim - 1)
      PRINT_DEMO("0x%x , ", vec[i]);
    else
      PRINT_DEMO("0x%x", vec[i]);
  }
  PRINT_DEMO("]\n");
}

/* unused functions

void gf_matrix_dot_vec(uint8_t *res, uint8_t **mat, uint8_t *vec, uint8_t n) {

  uint8_t i;
  for (i = 0; i < n; i++)
    res[i] = gf_vec_dot_vec(mat[i], vec, n);
}

void gf_matrix_adjoint(uint8_t **A, uint8_t **adj) {
  if (K == 1) {
    adj[0][0] = 1;
    return;
  }

  uint8_t i, j;
  // temp is used to store cofactors of A[][]
  uint8_t **temp = malloc(K * sizeof(uint8_t *)); // To store cofactors
  for (i = 0; i < K; i++)
    temp[i] = malloc(K * sizeof(uint8_t));

  for (i = 0; i < K; i++) {
    for (j = 0; j < K; j++) {
      // Get cofactor of A[i][j]
      get_cofactor(A, temp, i, j, K);

      // Interchanging rows and columns to get the
      // transpose of the cofactor matrix
      adj[j][i] = gf_matrix_det(temp, K - 1);
    }
  }

  for (i = 0; i < K; i++)
    free(temp[i]);
  free(temp);
}

uint8_t gf_matrix_inverse(uint8_t **A_inv, uint8_t **A, uint8_t n) {

  uint8_t det = gf_matrix_det(A, n);
  if (det == 0) {
    return 0;
  }
  uint8_t **adj;
  adj = malloc(n * sizeof(uint8_t *));
  uint8_t i, j;
  for (i = 0; i < n; i++)
    adj[i] = malloc(n * sizeof(uint8_t));

  // find adjoint
  gf_matrix_adjoint(A, adj);

  // 1/det(A)
  det = gf_inv(det);

  // A_inv = adj(A)/det(A)
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      A_inv[i][j] = gf_mul(adj[i][j], det);

  for (i = 0; i < n; i++)
    free(adj[i]);
  free(adj);
  return 1;
}

// Function to get cofactor of mat[p][q] in temp[][]. n is current
// dimension of mat[][]
void get_cofactor(uint8_t **A, uint8_t **temp, uint8_t p, uint8_t q,
                  uint8_t n) {
  uint8_t i = 0, j = 0, row, col;

  // Looping for each element of the matrix
  for (row = 0; row < n; row++) {
    for (col = 0; col < n; col++) {
      //  Copying into temporary matrix only those element
      //  which are not in given row and column
      if (row != p && col != q) {
        temp[i][j++] = A[row][col];

        // Row is filled, so increase row index and
        // reset col index
        if (j == n - 1) {
          j = 0;
          i++;
        }
      }
    }
  }
}

// http://www.geeksforgeeks.org/determinant-of-a-matrix/
uint8_t gf_matrix_det(uint8_t **A, uint8_t n) {

  uint8_t det = 0;

  if (n == 1)
    return A[0][0];
  uint8_t i;
  uint8_t **temp = malloc(K * sizeof(uint8_t *));
  for (i = 0; i < K; i++)
    temp[i] = malloc(K * sizeof(uint8_t));

  for (i = 0; i < n; i++) {

    get_cofactor(A, temp, 0, i, n);
    det = gf_add(det, gf_mul(A[0][i], gf_matrix_det(temp, n - 1)));
  }

  for (i = 0; i < K; i++)
    free(temp[i]);
  free(temp);
  return det;
}

void gf_matrix_mul_nxn(uint8_t **res, uint8_t **a, uint8_t **b) {

  int i, j, k;

  for (i = 0; i < K; i++) {
    for (j = 0; j < K; j++) {
      res[i][j] = 0;
      for (k = 0; k < K; k++) {
        res[i][j] = gf_add(res[i][j], gf_mul(a[i][k], b[k][j]));
      }
    }
  }
}
*/
