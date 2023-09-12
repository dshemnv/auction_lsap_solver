#include "cauction.hpp"
#include <new>

extern "C" {

void print_d_array(d_array *array) {
    printf("[");
    for (int i = 0; i < array->rows; i++) {
        for (int j = 0; j < array->cols; j++) {
            printf("%lf ", array->data[i * array->cols + j]);
        }
        if (i != array->rows - 1) {
            printf("\n");
        }
    }
    printf("]\n");
}

assignment_result *solve_forward(d_array *cost_matrix, float eps) {
    int rows = cost_matrix->rows;
    int cols = cost_matrix->cols;

    assert(cols == rows);

    auction_array<double> cpp_cost_matrix;
    init<double>(&cpp_cost_matrix, rows, cols, 0);

    for (int i = 0; i < cpp_cost_matrix.rows; i++) {
        for (int j = 0; j < cpp_cost_matrix.cols; j++) {
            cpp_cost_matrix.data[i * cols + j] =
                cost_matrix->data[i * cols + j];
        }
    }
    assignments<double> result;
    assignment<double> assig = {.agent = -1, .object = -1, .value = -1};
    result.size              = cpp_cost_matrix.rows;
    result.is_empty          = true;
    result.n_assignment      = 0;
    result.result = new (std::nothrow) assignment<double>[result.size];
    for (int i = 0; i < result.size; i++) {
        result.result[i] = assig;
    }

    int n_iter = 0;

    solve_simple<double>(&cpp_cost_matrix, eps, &result, &n_iter);

    int *agent_to_obj = (int *)malloc(result.n_assignment * sizeof(int));
    int *row_idx      = (int *)malloc(result.n_assignment * sizeof(int));
    int a_idx         = 0;
    for (int i = 0; i < result.size; i++) {
        int agent  = result.result[i].agent;
        int object = result.result[i].object;
        if (agent != -1 && object != -1) {
            row_idx[a_idx++]    = i;
            agent_to_obj[agent] = object;
        }
    }

    assignment_result *res =
        (assignment_result *)malloc(sizeof(assignment_result));

    res->len             = result.n_assignment;
    res->agent_to_object = agent_to_obj;
    res->row_idx         = row_idx;
    res->n_iter          = n_iter;

    return res;
}

assignment_result *solve(d_array *cost_matrix, float eps) {
    // Convert d_array to array<double>
    int rows = cost_matrix->rows;
    int cols = cost_matrix->cols;

    matrix_t mat_type;
    if (rows == cols) {
        mat_type = MEQN;
    } else if (rows > cols) {
        mat_type = MGN;
    } else if (rows < cols) {
        mat_type = MLN;
    }

    auction_array<double> cpp_cost_matrix;
    init<double>(&cpp_cost_matrix, rows, cols, 0);

    for (int i = 0; i < cpp_cost_matrix.rows; i++) {
        for (int j = 0; j < cpp_cost_matrix.cols; j++) {
            cpp_cost_matrix.data[i * cols + j] =
                cost_matrix->data[i * cols + j];
        }
    }

    if (mat_type == MGN) {
        auction_array<double> t_cpp_cost_matrix;
        init<double>(&t_cpp_cost_matrix, cols, rows, 0);

        transpose(&cpp_cost_matrix, &t_cpp_cost_matrix);

        if (eps == 0) {
            eps = 1e-3 / cols;
        }

        assignments<double> result;
        assignment<double> assig = {.agent = -1, .object = -1, .value = -1};
        result.size              = t_cpp_cost_matrix.rows;
        result.is_empty          = true;
        result.n_assignment      = 0;
        result.result = new (std::nothrow) assignment<double>[result.size];
        for (int i = 0; i < result.size; i++) {
            result.result[i] = assig;
        }
        int niter = 0;

        solve_jacobi<double>(&t_cpp_cost_matrix, eps, &result, mat_type,
                             &niter);

        int *agent_to_obj = (int *)malloc(result.n_assignment * sizeof(int));
        int *row_idx      = (int *)malloc(result.n_assignment * sizeof(int));
        int a_idx         = 0;
        int o_idx         = 0;
        for (int i = 0; i < result.size; i++) {
            int agent  = result.result[i].agent;
            int object = result.result[i].object;
            if (agent != -1 && object != -1) {
                if (mat_type == MLN) {
                    assert(a_idx < result.n_assignment);
                    assert(o_idx < result.n_assignment);
                    agent_to_obj[a_idx++] = object;
                    row_idx[o_idx++]      = agent;
                } else if (mat_type == MGN) {
                    agent_to_obj[a_idx++] = agent;
                    row_idx[o_idx++]      = object;

                } else {
                    row_idx[a_idx++]    = i;
                    agent_to_obj[agent] = object;
                }
            }
        }

        if (mat_type != MEQN) {
            sort_together_c(row_idx, agent_to_obj, result.n_assignment);
        }

        assignment_result *res =
            (assignment_result *)malloc(sizeof(assignment_result));

        res->len             = result.n_assignment;
        res->agent_to_object = agent_to_obj;
        res->row_idx         = row_idx;
        res->n_iter          = niter;

        return res;
    } else {

        // puts("[C]: Converted input array to array<double>");
        // print_d_array(cost_matrix);
        // Init results
        if (eps == 0) {
            eps = 1e-3 / rows;
        }
        assignments<double> result;
        assignment<double> assig = {.agent = -1, .object = -1, .value = -1};
        result.size              = rows;
        result.is_empty          = true;
        result.n_assignment      = 0;
        result.result = new (std::nothrow) assignment<double>[result.size];
        for (int i = 0; i < result.size; i++) {
            result.result[i] = assig;
        }
        // puts("[C]: Initialized result structure");
        // Solve using solve_jacobi
        // puts("[C]: Start solving");
        int niter = 0;

        solve_jacobi<double>(&cpp_cost_matrix, eps, &result, mat_type, &niter);
        // puts("[C]: End solving");
        // Convert assignments<double> to assignment_result
        int *agent_to_obj = (int *)malloc(result.n_assignment * sizeof(int));
        int *row_idx      = (int *)malloc(result.n_assignment * sizeof(int));
        int a_idx         = 0;
        int o_idx         = 0;
        for (int i = 0; i < result.size; i++) {
            int agent  = result.result[i].agent;
            int object = result.result[i].object;
            if (agent != -1 && object != -1) {
                if (mat_type != MEQN) {
                    assert(a_idx < result.n_assignment);
                    assert(o_idx < result.n_assignment);
                    agent_to_obj[a_idx++] = object;
                    row_idx[o_idx++]      = agent;
                } else {
                    row_idx[a_idx++]    = i;
                    agent_to_obj[agent] = object;
                }
            }
        }
        if (mat_type != MEQN) {
            sort_together_c(row_idx, agent_to_obj, result.n_assignment);
        }
        // printf("%d\n", result.size);
        // for (int i = 0; i < result.size; i++) {
        //     printf("idx %d val %d\n", row_idx[i], agent_to_obj[i]);
        // }
        //
        // sort_together_c(row_idx, agent_to_obj, result.n_assignment);
        // puts("[C]: Converted result into arrays");
        assignment_result *res =
            (assignment_result *)malloc(sizeof(assignment_result));

        res->len             = result.n_assignment;
        res->agent_to_object = agent_to_obj;
        res->row_idx         = row_idx;
        res->n_iter          = niter;

        // puts("[C]: Populated result");

        return res;
    }
}

void swap_c(int *a, int *b) {
    int tmp = *b;
    *b      = *a;
    *a      = tmp;
}

void sort_together_c(int *array1, int *array2, int length) {
    for (int i = 0; i < length - 1; i++) {
        for (int j = 0; j < length - i - 1; j++) {
            if (array1[j] > array1[j + 1]) {
                swap(&array1[j], &array1[j + 1]);
                swap(&array2[j], &array2[j + 1]);
            }
        }
    }
}

void transpose(d_array *input, d_array *output) {
    for (int i = 0; i < input->rows; i++) {
        for (int j = 0; j < input->cols; j++) {
            output->data[j * input->rows + i] =
                input->data[i * input->cols + j];
        }
    }
}
}