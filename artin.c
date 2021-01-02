//
// Created by Hugh O'Reilly on 1/1/21.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct Matrix {
        int num_rows;
        int num_cols;
        float **data;
} matrix, *matrix_p;

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

matrix_p read_file(char *filename)
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
 * Procedures that deal with matrix operations.
 */

/*
 * Requires:
 *      Two matrices of the form m x n and n x l.
 * Returns:
 *      The matrix product of the form m x l.
 */
matrix_p multiply(matrix_p mat1, matrix_p mat2)
{
        int num_rows = mat1->num_rows;
        int num_cols = mat2->num_cols;
        int shared_length = mat1->num_cols;

        matrix_p new_mat = alloc_matrix(num_rows, num_cols);
        for (int i = 0; i < num_rows; i++) {
                for (int j = 0; j < num_cols; j++) {
                        // Keeping this in a variable on the stack reduces
                        // the number of unnecessary memory operations.
                        float sum = 0;
                        for (int k = 0; k < shared_length; k++) {
                                sum += mat1->data[i][k] * mat2->data[k][j];
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
matrix_p add(matrix_p mat1, matrix_p mat2)
{
        int num_rows = mat1->num_rows;
        int num_cols = mat1->num_cols;

        matrix_p new_mat = alloc_matrix(num_rows, num_cols);
        for (int i = 0; i < num_rows; i++) {
                for (int j = 0; j < num_cols; j++) {
                        new_mat->data[i][j] = mat1->data[i][j] + mat2->data[i][j];
                }
        }
        return new_mat;
}

// There are three types of row operations. You can add a scaled version
// of one row to another. You can swap two rows. Or you can multiple a
// row by a constant. (Here, we add a NOP option to simplify the algorithm
// below.)
enum op_type{LinearCombination, Swap, Scale, Nop};

typedef struct RowOperation {
        enum op_type type;
        int row1;
        int row2;
        float scale_factor;
        // We will be constructing a linked list of row operations.
        struct RowOperation *next;
} op, *op_p;

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
        op_p start_op = make_nop_op();
        op_p current_op = start_op;
        int size = mat->num_cols;

        // Keep track of a temp row to use when swapping rows.
        float *temp = malloc(sizeof(float) * size);
        op_p new_op;
        float scale_factor;

        for (int col = 0; col < size; col++) {
                // Move down from the top and find the first nonzero element
                // in column col.
                int non_zero_row = -1;
                for (int row = col; row < size; row++) {
                        if (mat->data[row][col] != 0) {
                                non_zero_row = row;
                                break;
                        }
                }
                if (non_zero_row == -1)
                        return NULL; // The matrix is not invertible.

                // Scale the row by this value.
                scale_factor = 1 / mat->data[non_zero_row][col];
                for (int i = col; i < size; i++)
                        mat->data[non_zero_row][i] *= scale_factor;
                new_op = make_scale_op(non_zero_row, scale_factor);
                current_op->next = new_op;
                current_op = new_op;

                // Zero out the remaining rows in the column.
                for (int row = 0; row < size; row++) {
                        if (mat->data[row][col] == 0 || row == non_zero_row)
                                continue;
                        scale_factor = -1 * mat->data[row][col] / mat->data[non_zero_row][col];
                        for (int i = col; i < size; i++)
                                mat->data[row][i] += scale_factor * mat->data[non_zero_row][i];
                        new_op = make_lc_op(row, non_zero_row, scale_factor);
                        current_op->next = new_op;
                        current_op = new_op;
                }

                // Finally, if necessary, we swap non_zero_row with row col.
                if (non_zero_row == col)
                        continue;
                memcpy(temp, mat->data[non_zero_row], sizeof(float) * size);
                memcpy(mat->data[non_zero_row], mat->data[col], sizeof(float) * size);
                memcpy(mat->data[col], temp, sizeof(float) * size);
                new_op = make_swap_op(non_zero_row, col);
                current_op->next = new_op;
                current_op = new_op;
        }
        free(temp);
        return start_op;
}

matrix_p construct_inverse(int size, op_p start_op)
{
        int i;
        float *temp = malloc(sizeof(float) * size);
        matrix_p mat = alloc_matrix(size, size);

        // Start with the identity.
        for (i = 0; i < size; i++)
                mat->data[i][i] = 1;
        // Compute the row operations.
        for (op_p op = start_op; op != NULL; op = op->next) {
                switch (op->type) {
                case Nop:
                        break;
                case LinearCombination:
                        for (i = 0; i < size; i++)
                                mat->data[op->row1][i] += op->scale_factor * mat->data[op->row2][i];
                        break;
                case Swap:
                        memcpy(temp, mat->data[op->row2], sizeof(float) * size);
                        memcpy(mat->data[op->row2], mat->data[op->row1], sizeof(float) * size);
                        memcpy(mat->data[op->row1], temp, sizeof(float) * size);
                        break;
                case Scale:
                        for (i = 0; i < size; i++)
                                mat->data[op->row1][i] *= op->scale_factor;
                        break;
                default:
                        break;
                }
        }
        free(temp);
        return mat;
}

int main(int argc, char **argv)
{
        if (argc == 1) {
                printf("Argument required.\n");
                exit(0);
        }
        // Multiply the input matrices.
        if (strcmp(argv[1], "-m") == 0) {
                matrix_p mat1 = read_file(argv[2]);
                matrix_p mat2 = read_file(argv[3]);
                matrix_p product = multiply(mat1, mat2);
                print_matrix(product);
                free_matrix(mat1);
                free_matrix(mat2);
                free_matrix(product);
        }
        // Add the input matrices.
        if (strcmp(argv[1], "-a") == 0) {
                matrix_p mat1 = read_file(argv[2]);
                matrix_p mat2 = read_file(argv[3]);
                matrix_p sum = add(mat1, mat2);
                print_matrix(sum);
                free_matrix(mat1);
                free_matrix(mat2);
                free_matrix(sum);
        }
        // Compute the inverse of the input matrix.
        if (strcmp(argv[1], "-i") == 0) {
                matrix_p mat = read_file(argv[2]);
                int size = mat->num_rows;
                op_p start_op;
                if ((start_op = convert_to_rref(mat)) == NULL) {
                        printf("Not invertible.\n");
                        exit(0);
                }
                free(mat);
//                print_ops(start_op);
//                printf("\n");
                matrix_p inverse = construct_inverse(size, start_op);
                print_matrix(inverse);
                free_matrix(inverse);
        }
}
