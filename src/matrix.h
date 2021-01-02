// matrix.h

#ifndef MATRIX_H
#define MATRIX_H

#include "frac.h"

typedef struct Matrix {
    int num_rows;
    int num_cols;
    float **data;
} matrix, *matrix_p;

// There are three types of row operations. You can add a scaled version
// of one row to another. You can swap two rows. Or you can multiple a
// row by a constant. (Here, we also add a NOP option that does nothing, for
// ease of use.)
enum op_type{LinearCombination, Swap, Scale, Nop};

typedef struct RowOperation {
    enum op_type type;
    int row1;
    int row2;
    float scale_factor;
    // You can construct a linked list of row operations.
    struct RowOperation *next;
} op, *op_p;

matrix_p        alloc_matrix(int num_rows, int num_cols);
void            free_matrix(matrix_p mat);
void            print_matrix(matrix_p mat);
matrix_p        read_matrix(char *filename);

matrix_p        matrix_mult(matrix_p mat1, matrix_p mat2);
matrix_p        matrix_add(matrix_p mat1, matrix_p mat2);

op_p            make_lc_op(int row1, int row2, float scale_factor);
op_p            make_swap_op(int row1, int row2);
op_p            make_scale_op(int row1, float scale_factor);
op_p            make_nop_op();
void            compute_lc(float *row1_p, float *row2_p, float scale_factor,
                           int num_cols);
void            compute_swap(float *row1_p, float *row2_p, int num_cols);
void            compute_scale(float *row_p, float scale_factor, int num_cols);
void            print_ops(op_p start_op);
op_p            convert_to_rref(matrix_p mat);
void            apply_ops(matrix_p mat, op_p start_op);
matrix_p        compute_inverse(matrix_p mat);

#endif //MATRIX_H
