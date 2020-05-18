#ifndef __MATRIX_H
#define __MATRIX_H

#include <stdint.h>

typedef struct
{
    uint16_t  col;
    uint16_t  row;
    float     **mat;
}matrix_t;

void mat_reset(matrix_t *mat_ptr, uint16_t mode);
int mat_init(matrix_t *mat_ptr, uint16_t row, uint16_t col, uint16_t mode);
void mat_free(matrix_t *mat_ptr);
void print_mat(matrix_t *mat_ptr);

#endif //__MATRIX_H
