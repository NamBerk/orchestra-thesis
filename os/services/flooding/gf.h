/*
SEEMOO - Secure Mobile Networking Lab and Project, Winter 2017/18
author: Jan Sturm
*/

#ifndef FLOODING_GF_H_
#define FLOODING_GF_H_

#include <stdlib.h>
#include <stdio.h>

#include "random.h"
#include "./params.h"
#include "./util.h"

#define GF_MOD(a) ((a) & (GF - 1))

uint8_t gf_choose_random();

void gf_print_byte(char *c, uint8_t b);
void gf_vec_print(char *c, uint8_t *vec, uint8_t dim);
void gf_matrix_print(char *c, uint8_t **mat, uint8_t dim_r, uint8_t dim_c);

#endif
