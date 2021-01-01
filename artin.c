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
                        printf("\t");
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
                        int sum = 0;
                        for (int k = 0; k < shared_length; k++)
                                sum += mat1->data[i][k] * mat2->data[k][j];
                        new_mat->data[i][j] = sum;
                }
        }
        return new_mat;
}

int main(int argc, char **argv)
{
        if (argc == 1) {
                printf("Argument required.");
                exit(0);
        }
        if (strcmp(argv[1], "-m") == 0) {
                matrix_p mat1 = read_file(argv[2]);
                matrix_p mat2 = read_file(argv[3]);
                matrix_p product = multiply(mat1, mat2);
                print_matrix(product);
                free_matrix(mat1);
                free_matrix(mat2);
                free_matrix(product);
        }
}
