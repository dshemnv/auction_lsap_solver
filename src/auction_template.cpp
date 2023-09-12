#include "cauction.hpp"
#include <chrono>
#define STRINGIFY(X) STRINGIFY_(X)
#define STRINGIFY_(X) #X
#ifndef DATA
#include "data/bench_data_16.hpp"
#else
#include STRINGIFY(DATA)
#endif
#define EXP_T 10

#ifndef _R
#define _R 16
#endif

#ifndef _C
#define _C 16
#endif

#define SIZE (_R * _C)

double total_cost(assignment_result *assignment, d_array *cost_matrix) {
    double cost = 0.0;

    for (int i = 0; i < assignment->len; i++) {
        int row = i;
        int col = assignment->agent_to_object[i];
        cost += cost_matrix->data[row * cost_matrix->cols + col];
    }
    return cost;
}

int main(int argc, char **argv) {
    int ret                   = 0;
    float eps                 = 0.0001f;
    float times_sum[MATRICES] = {0};
    float times_min[MATRICES] = {0};
    float times_max[MATRICES] = {0};

    int max_iter = 0;

    for (int i = 0; i < MATRICES; i++) {
        times_min[i] = 100000;
        times_max[i] = -100000;
    }
    int correct = 0;
    std::cout << "Processing " << STRINGIFY(DATA) << std::endl;
    for (int m = 0; m < MATRICES; m++) {
        bool good = true;
        int iter  = 0;
        double auction_cost, hungarian_cost;
        std::cout << "Progress: " << (m + 1) << " / " << MATRICES
                  << (m == MATRICES - 1 ? "\n" : "\r");
        for (int t = 0; t < EXP_T; t++) {
            iter = 0;

            double *d_arr = arrays[m];
            int *golden   = results[m];

            d_array arr;
            arr.cols = _C;
            arr.rows = _R;
            arr.data = d_arr;

            assignment_result golden_res;
            golden_res.len             = _R;
            golden_res.row_idx         = {0};
            golden_res.agent_to_object = golden;

            assignment_result *result;
            std::chrono::time_point<std::chrono::high_resolution_clock> tstart,
                tend;
            tstart = std::chrono::high_resolution_clock::now();
#ifdef FORWARD
            result = solve_forward(&arr, eps);
#else
            result = solve(&arr, eps);
#endif
            tend = std::chrono::high_resolution_clock::now();

            int time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                              tend - tstart)
                              .count();

            double exec_time = time_ns / 1000.0; // convert to μs

            auction_cost   = total_cost(result, &arr);
            hungarian_cost = total_cost(&golden_res, &arr);
            iter           = result->n_iter;

            if (abs(auction_cost - hungarian_cost) / hungarian_cost > 1.0f) {
                good = false;
            }

            for (int i = 0; i < _R; i++) {
                if (result->agent_to_object[i] !=
                    golden_res.agent_to_object[i]) {
                    if (result->agent_to_object[i] == -1) {
                        // printf("Done %d iters\n", iter);
                        printf("Not enough iters\n");
                        good = false;
                        // print_array(result, 1, C);
                    }
                }
            }
            if (good) {
                times_sum[m] += exec_time;
                times_min[m] =
                    times_min[m] < exec_time ? times_min[m] : exec_time;
                times_max[m] =
                    times_max[m] > exec_time ? times_max[m] : exec_time;
            }
        }
        if (good) {
            correct++;
        }
        if (iter < MAX_ITER_AUCTION) {
            max_iter = max_iter > iter ? max_iter : iter;
        }
    }

    float sum_mean = 0;
    float min      = MAXFLOAT;
    float max      = -MAXFLOAT;

    for (int m = 0; m < MATRICES; m++) {
        sum_mean += times_sum[m] / EXP_T;
        min = times_min[m] < min ? times_min[m] : min;
        max = times_max[m] > max ? times_max[m] : max;
    }
    float mean = sum_mean / MATRICES;

    double precision = (correct / (double)MATRICES) * 100.0;

    if (precision < 80.0) {
        ret = 1;
    }

    printf("[TEMPLATE] Precision     %.3f \n\r", precision);
    printf("[TEMPLATE] Average exec time\t %.5f μs\n\r", mean);
    printf("[TEMPLATE] Minimum exec time\t %.5f μs\n\r", min);
    printf("[TEMPLATE] Maximum exec time\t %.5f μs\n\r", max);
    printf("[TEMPLATE] Maximum iterations\t %d", max_iter);
    printf("\n");
    printf("\n");

    return ret;
}