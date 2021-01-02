// matrix.c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "frac.h"
#include "matrix.h"

/*
 * Procedures that deal with matrix space management and I/O.
 */

matrix_p alloc_matrix(int num_rows, int num_cols)
{
        // We need to allocate a 2d array on the heap.
        float **data = calloc(num_rows, sizeof(float *));
        if (data == NULL)
                return NULL;
        for (int i = 0; i < num_rows; i++) {
                float *subarray = calloc(num_cols, sizeof(float));
                if (subarray == NULL)
                        return NULL;
                data[i] = subarray;
        }
        matrix_p result = malloc(sizeof(matrix));
        if (result == NULL)
                return NULL;
        result->num_rows = num_rows;
        result->num_cols = num_cols;
        result->data = data;
        return result;
}

matrix_p alloc_id_matrix(int num_rows)
{
        int i;
        matrix_p id = alloc_matrix(num_rows, num_rows);
        for (i = 0; i < num_rows; i++)
                id->data[i][i] = 1;
        return id;
}

void free_matrix(matrix_p mat)
{
        for (int i = 0; i < mat->num_rows; i++)
                free(mat->data[i]);
        free(mat->data);
        free(mat);
}

void print_matrix(matrix_p mat)
{
        printf("%dx%d\n", mat->num_rows, mat->num_cols);
        for (int i = 0; i < mat->num_rows; i++) {
                for (int j = 0; j < mat->num_cols; j++) {
                        printf("%f", mat->data[i][j]);
                        printf(",");
                }
                printf("\n");
        }
}

matrix_p read_matrix(char *filename)
{
        FILE *csv = fopen(filename, "r");
        int num_rows, num_cols;
        if (fscanf(csv, "%dx%d\n", &num_rows, &num_cols) != 2)
                return NULL;
        matrix_p mat = alloc_matrix(num_rows, num_cols);
        int row = 0, col = 0;
        while (fscanf(csv, "%f,", &mat->data[row][col]) == 1) {
                col++;
                if (col == num_cols) {
                        col = 0;
                        row++;
                }
        }
        return mat;
}

/*
 * Procedures that deal with matrix arithmetic.
 */

/*
 * Requires:
 *      Two matrices of the form m x n and n x l.
 * Effects:
 *      Returns the matrix product of the form m x l.
 */
matrix_p matrix_mult(matrix_p mat1, matrix_p mat2)
{
        int num_rows = mat1->num_rows;
        int num_cols = mat2->num_cols;
        int shared_length = mat1->num_cols;
        float **data1 = mat1->data;
        float **data2 = mat2->data;

        matrix_p new_mat = alloc_matrix(num_rows, num_cols);
        for (int i = 0; i < num_rows; i++) {
                for (int j = 0; j < num_cols; j++) {
                        // Keeping this in a variable on the stack reduces
                        // the number of unnecessary memory operations.
                        float sum = 0;
                        for (int k = 0; k < shared_length; k++) {
                                sum = add(sum, mult(data1[i][k], data2[k][j]));
                        }
                        new_mat->data[i][j] = sum;
                }
        }
        return new_mat;
}

/*
 * Requires:
 *      Two matrices of the form m x n and m x n.
 * Returns:
 *      The matrix product of the form m x n.
 */
matrix_p matrix_add(matrix_p mat1, matrix_p mat2)
{
        int num_rows = mat1->num_rows;
        int num_cols = mat1->num_cols;
        float **data1 = mat1->data;
        float **data2 = mat2->data;

        matrix_p new_mat = alloc_matrix(num_rows, num_cols);
        for (int i = 0; i < num_rows; i++) {
                for (int j = 0; j < num_cols; j++) {
                        new_mat->data[i][j] = add(data1[i][j], data2[i][j]);
                }
        }
        return new_mat;
}

/*
 * Procedures to compute the inverse of a matrix.
 */

op_p make_lc_op(int row1, int row2, float scale_factor)
{
        op_p new_op = malloc(sizeof(op));
        if (new_op == NULL)
                return NULL;
        new_op->type = LinearCombination;
        // In this case, row1 = row1 + scale_factor * row2.
        new_op->row1 = row1;
        new_op->row2 = row2;
        new_op->scale_factor = scale_factor;
        return new_op;
}

op_p make_swap_op(int row1, int row2)
{
        op_p new_op = malloc(sizeof(op));
        if (new_op == NULL)
                return NULL;
        new_op->type = Swap;
        new_op->row1 = row1;
        new_op->row2 = row2;
        return new_op;
}

op_p make_scale_op(int row1, float scale_factor)
{
        op_p new_op = malloc(sizeof(op));
        if (new_op == NULL)
                return NULL;
        new_op->type = Scale;
        new_op->row1 = row1;
        new_op->scale_factor = scale_factor;
        return new_op;
}

op_p make_nop_op()
{
        op_p new_op = malloc(sizeof(op));
        if (new_op == NULL)
                return NULL;
        new_op->type = Nop;
        return new_op;
}

void compute_lc(float *row1_p, float *row2_p, float scale_factor, int num_cols)
{
        int i;
        for (i = 0; i < num_cols; i++)
                row1_p[i] = add(row1_p[i], mult(scale_factor, row2_p[i]));
}

void compute_swap(float *row1_p, float *row2_p, int num_cols)
{
        float temp[num_cols];
        memcpy(temp, row2_p, sizeof(float) * num_cols);
        memcpy(row2_p, row1_p, sizeof(float) * num_cols);
        memcpy(row1_p, temp, sizeof(float) * num_cols);
}

void compute_scale(float *row_p, float scale_factor, int num_cols)
{
        int i;
        for (i = 0; i < num_cols; i++)
                row_p[i] = mult(row_p[i], scale_factor);
}

void print_ops(op_p start_op)
{
        for (op_p op = start_op; op != NULL; op = op->next) {
                switch (op->type) {
                case Nop:
                        printf("Nop.\n");
                        break;
                case LinearCombination:
                        printf("Compute row %d += %f * row %d.\n", op->row1,
                                op->scale_factor, op->row2);
                        break;
                case Swap:
                        printf("Swap rows %d and %d.\n", op->row1, op->row2);
                        break;
                case Scale:
                        printf("Scale row %d by %f.\n", op->row1,
                                op->scale_factor);
                        break;
                default:
                        break;
                }
        }
}

op_p convert_to_rref(matrix_p mat)
{
        int col, row;
        op_p new_op;
        float scale_factor;

        op_p start_op = make_nop_op();
        op_p current_op = start_op;
        int size = mat->num_cols;
        float **data = mat->data;

        // Use the typical algorithm to convert mat into reduced row
        // echelon form. At each step, we keep track of the row operation
        // performed.
        for (col = 0; col < size; col++) {
                // Move down from the top and find the first nonzero element
                // in column col.
                int non_zero_row = -1;
                for (int row = col; row < size; row++) {
                        if (data[row][col] != 0) {
                                non_zero_row = row;
                                break;
                        }
                }
                if (non_zero_row == -1)
                        return NULL; // The matrix is not invertible.

                // Scale the row by this value.
                scale_factor = divide(1, data[non_zero_row][col]);
                compute_scale(data[non_zero_row], scale_factor, size);
                new_op = make_scale_op(non_zero_row, scale_factor);
                current_op->next = new_op;
                current_op = new_op;

                // Zero out the remaining rows in the column.
                for (row = 0; row < size; row++) {
                        if (data[row][col] == 0 || row == non_zero_row)
                                continue;
                        scale_factor = mult(-1, divide(data[row][col], data[non_zero_row][col]));
                        compute_lc(data[row], data[non_zero_row], scale_factor, size);
                        new_op = make_lc_op(row, non_zero_row, scale_factor);
                        current_op->next = new_op;
                        current_op = new_op;
                }

                // Finally, if necessary, we swap non_zero_row with row col.
                if (non_zero_row == col)
                        continue;
                compute_swap(data[non_zero_row], data[col], size);
                new_op = make_swap_op(non_zero_row, col);
                current_op->next = new_op;
                current_op = new_op;
        }
        return start_op;
}

void apply_ops(matrix_p mat, op_p start_op)
{
        int i;
        int num_cols = mat->num_cols;
        float **data = mat->data;
        float *temp = malloc(sizeof(float *) * num_cols);

        // Compute the row operations.
        for (op_p op = start_op; op != NULL; op = op->next) {
                switch (op->type) {
                case Nop:
                        break;
                case LinearCombination:
                        compute_lc(data[op->row1], data[op->row2], op->scale_factor, num_cols);
                        break;
                case Swap:
                        compute_swap(data[op->row1], data[op->row2], num_cols);
                        break;
                case Scale:
                        compute_scale(data[op->row1], op->scale_factor, num_cols);
                        break;
                default:
                        break;
                }
        }
        free(temp);
}

matrix_p compute_inverse(matrix_p mat)
{
        op_p start_op = convert_to_rref(mat);
        if (start_op == NULL)
                return NULL;
        matrix_p inv = alloc_id_matrix(mat->num_rows);
        apply_ops(inv, start_op);
        return inv;
}
