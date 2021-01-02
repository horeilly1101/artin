// main.c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "frac.h"
#include "matrix.h"

int main(int argc, char **argv)
{
        if (argc == 1) {
                printf("Argument required.\n");
                exit(0);
        }
        // Multiply the input matrices.
        if (strcmp(argv[1], "-m") == 0) {
                matrix_p mat1 = read_matrix(argv[2]);
                matrix_p mat2 = read_matrix(argv[3]);
                matrix_p product = matrix_mult(mat1, mat2);
                print_matrix(product);
                free_matrix(mat1);
                free_matrix(mat2);
                free_matrix(product);
        }
        // Add the input matrices.
        if (strcmp(argv[1], "-a") == 0) {
                matrix_p mat1 = read_matrix(argv[2]);
                matrix_p mat2 = read_matrix(argv[3]);
                matrix_p sum = matrix_add(mat1, mat2);
                print_matrix(sum);
                free_matrix(mat1);
                free_matrix(mat2);
                free_matrix(sum);
        }
        // Compute the inverse of the input matrix.
        if (strcmp(argv[1], "-i") == 0) {
                matrix_p mat = read_matrix(argv[2]);
                matrix_p inv = compute_inverse(mat);
                free(mat);
                if (inv == NULL) {
                        printf("Not invertible.\n");
                        exit(0);
                }
                print_matrix(inv);
                free_matrix(inv);
        }
}
