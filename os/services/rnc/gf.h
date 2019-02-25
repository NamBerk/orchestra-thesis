/*
   SEEMOO - Secure Mobile Networking Lab and Project, Winter 2017/18
   author: Jan Sturm
 */

#ifndef RNC_GF_H_
#define RNC_GF_H_

#include <stdlib.h>
#include <stdio.h>

#include "os/lib/random.h"
#include "./params.h"
#include "./rnc.h"
#include "./util.h"



#define GF_MOD(a) ((a) & (GF - 1))
#define MOD255(a) ((a)>=0xff ? (a)-0xff : (a))
#define MOD15(a) ((a)>=0x0f ? (a)-0x0f : (a))

#if (GF == 256 || GF == 16)
#define gf_mul gf_mul_precomp
#define gf_inv gf_inv_precomp
#elif GF == 2
#define gf_mul gf_mul_comp
#define gf_inv gf_inv_comp
#endif

uint8_t gf_choose_random();
void gf_vec_choose_random(uint8_t *v, uint8_t n);

uint8_t gf_add(uint8_t a, uint8_t b);
void gf_vec_add(uint8_t *r,uint8_t *a, uint8_t *b);
void gf_sum_bytevec(uint8_t *r,uint8_t *a, uint8_t n);

uint8_t gf_mul(uint8_t a, uint8_t b);
void gf_byte_mul_vec(uint8_t *r, uint8_t a, uint8_t *b);
uint8_t gf_inv(uint8_t a);

uint8_t gf_vec_dot_vec(uint8_t *a, uint8_t *b, uint8_t n);
void gf_vec_dot_matrix(uint8_t *res, uint8_t *vec, uint8_t **mat, uint8_t row, uint8_t col);

void gf_print_byte(char *c, uint8_t b);
void gf_vec_print(char *c, uint8_t *vec, uint8_t dim);
void gf_matrix_print(char *c, uint8_t **mat, uint8_t dim_r, uint8_t dim_c);

/* unused functions

   void gf_matrix_dot_vec(uint8_t *res, uint8_t **mat, uint8_t *vec, uint8_t n);
   uint8_t gf_matrix_det(uint8_t **A, uint8_t n);
   uint8_t gf_matrix_inverse(uint8_t **A_inv, uint8_t **A, uint8_t n);
   uint8_t gf_matrix_rank(uint8_t **A, uint8_t n);
   void get_cofactor(uint8_t **A, uint8_t **temp, uint8_t p, uint8_t q, uint8_t n);
   void gf_matrix_mul_nxn(uint8_t **res, uint8_t **a, uint8_t **b);

 */

#endif
