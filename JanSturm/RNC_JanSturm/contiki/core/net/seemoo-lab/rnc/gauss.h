/*
SEEMOO - Secure Mobile Networking Lab and Project, Winter 2017/18
author: Jan Sturm
*/

#ifndef RNC_GAUSS_H_
#define RNC_GAUSS_H_

#include "./util.h"

void swap_row(uint8_t **A, uint8_t **b, uint8_t r1, uint8_t r2);

uint8_t gaussian_elimination_iter(uint8_t **A, uint8_t **x, uint8_t **b, uint8_t c,
                               uint8_t index_new,  uint8_t mode);

#endif
