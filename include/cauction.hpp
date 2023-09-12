/* -------------------------------------------------------------------------- */
/*                      C Interface for Python shared lib                     */
/* -------------------------------------------------------------------------- */

#include "auction.hpp"
#include <stdio.h>
#include <stdlib.h>

extern "C" {
typedef struct d_array {
    int rows;
    int cols;
    double *data;
} d_array;

typedef struct assignment_result {
    int *agent_to_object;
    int *row_idx;
    int len;
    int n_iter;
} assignment_result;

void print_d_array(d_array *array);
assignment_result *solve(d_array *cost_matrix, float eps);
assignment_result *solve_forward(d_array *cost_matrix, float eps);
void swap_c(int *a, int *b);
void sort_together_c(int *array1, int *array2, int length);
}
/* -------------------------------------------------------------------------- */