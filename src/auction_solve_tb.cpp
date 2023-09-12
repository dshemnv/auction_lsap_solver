//
// g++ *.cpp -o main -std=c++17 -DDATA=data_8x8_8_objects.hpp -D_R=8 -Ofast
// -march=native -mtune=native && ./main
//
#include "auction_solve_simd.hpp"
#include "auction_solve_template.hpp"
#define STRINGIFY(X) STRINGIFY_(X)
#define STRINGIFY_(X) #X
#ifndef DATA
#include "bench_data_16.h"
#else
#include STRINGIFY(DATA)
#endif
#include "xtime_l.h"
#include "xil_printf.h"
#define printf xil_printf

#define EXP_T 10

double total_cost(object_t *assignment, double *cost_matrix) {
    double cost = 0.0;

    for (int i = 0; i < __ROWS; i++) {
        int row = i;
        int col = assignment[i];
        cost += cost_matrix[row * __COLS + col];
    }
    return cost;
}

int main(int argc, char **argv) {

    int ret = 0;

    float times_sum[MATRICES] = {0};
    float times_min[MATRICES] = {0};
    float times_max[MATRICES] = {0};

    for (unsigned int i = 0; i < MATRICES; i++) {
        times_min[i] = 1000;
        times_max[i] = -1000;
    }

    int agents  = __ROWS;
    int objects = __COLS;
    if (__ROWS > __COLS) {
        agents  = __COLS;
        objects = __ROWS;
    }

    int f_cnt;
    int correct;
    int errors;

    for (int j = 0; j < MATRICES; j += 1)
        times_sum[j] = 0;
    for (int j = 0; j < MATRICES; j += 1)
        times_min[j] = +10000;
    for (int j = 0; j < MATRICES; j += 1)
        times_max[j] = -10000;
    correct = 0;
    errors  = 0;
    int max_iter = 0;
    for (unsigned int m = 0; m < MATRICES; m++) {
        bool good = true;
        float auction_cost, hungarian_cost;
        for (int t = 0; t < EXP_T; t++) {
            f_cnt = 0;

            double *d_arr = arrays[m];
            int *golden   = results[m];

            val_t arr[SIZE];
            for (int i = 0; i < SIZE; i++) {
                arr[i] = (val_t)roundf(MULT * d_arr[i]);
            }

            val_t result[SIZE];

            XTime tstart, tend;
			XTime_GetTime(&tstart);

            solve_auction_template(arr, result, &f_cnt);

            XTime_GetTime(&tend);

			float time = 1.0  * (tend - tstart)/(COUNTS_PER_SECOND/1000000);


            auction_cost   = total_cost(result, d_arr);
            hungarian_cost = total_cost(golden, d_arr);

            if (abs(auction_cost - hungarian_cost)/hungarian_cost > 1.0f) {
                good = false;
            }
            for (int i = 0; i < agents; i++) {
                if (result[i] == -1) {
                    good = false;
                }
            }
            if (good) {
                times_sum[m] += time;
                times_min[m] = times_min[m] < time ? times_min[m] : time;
                times_max[m] = times_max[m] > time ? times_max[m] : time;
                max_iter = max_iter > f_cnt ? max_iter : f_cnt;
            }
        }
        errors += (!good);
        correct += good;
    }

    float sum_mean = 0;
    float min      = 1000;
    float max      = -1000;

    for (int m = 0; m < MATRICES; m++) {
        sum_mean += times_sum[m];
        min = times_min[m] < min ? times_min[m] : min;
        max = times_max[m] > max ? times_max[m] : max;
    }
    sum_mean /= EXP_T;
    float mean = sum_mean / MATRICES;

    int whole_mean      = mean;
    int floating_mean = (mean - whole_mean) * 1000;

    int whole_min      = min;
    int floating_min = (min - whole_min) * 10000;

    int whole_max      = max;
    int floating_max = (max - whole_max) * 1000;

    printf("[TMPL] Correct counter     %4d/%d\n\r", correct, MATRICES);
    printf("[TMPL] Average exec time\t %d.%.3d μs\n\r", whole_mean,
           floating_mean);
    printf("[TMPL] Minimum exec time\t %d.%.4d μs\n\r", whole_min,
           floating_min);
    printf("[TMPL] Maximum exec time\t %d.%.3d μs\n\r", whole_max,
           floating_max);
    printf("[TMPL] Max iterations\t %d\n\r", max_iter);
    printf("\n\r");

    //
    //
    //
    //
    //

    for (int j = 0; j < MATRICES; j += 1)
        times_sum[j] = 0;
    for (int j = 0; j < MATRICES; j += 1)
        times_min[j] = +10000;
    for (int j = 0; j < MATRICES; j += 1)
        times_max[j] = -10000;
    correct = 0;
    errors  = 0;
    max_iter = 0;
    for (unsigned int m = 0; m < MATRICES; m++) {
        bool good = true;
        float auction_cost, hungarian_cost;
        for (int t = 0; t < EXP_T; t++) {
            f_cnt = 0;

            double *d_arr = arrays[m];
            int *golden   = results[m];

            val_t arr[SIZE];
            for (int i = 0; i < SIZE; i++) {
                arr[i] = (val_t)roundf(MULT * d_arr[i]);
            }

            val_t result[SIZE];

            XTime tstart, tend;
			XTime_GetTime(&tstart);

            solve_auction_simd(arr, result, &f_cnt);

            XTime_GetTime(&tend);

            float time = 1.0  * (tend - tstart)/(COUNTS_PER_SECOND/1000000);

            auction_cost   = total_cost(result, d_arr);
            hungarian_cost = total_cost(golden, d_arr);

            if (abs(auction_cost - hungarian_cost)/hungarian_cost > 1.0f) {
                good = false;
            }
            for (int i = 0; i < agents; i++) {
                if (result[i] == -1) {
                	printf("Not enough iters\n\r");
                    good = false;
                }
            }
            if (good) {
                times_sum[m] += time;
                times_min[m] = times_min[m] < time ? times_min[m] : time;
                times_max[m] = times_max[m] > time ? times_max[m] : time;
                max_iter = max_iter > f_cnt ? max_iter : f_cnt;
            }
        }
        errors += (!good);
        correct += good;
    }

    sum_mean = 0;
    min      = 1000;
    max      = -1000;

    for (int m = 0; m < MATRICES; m++) {
        sum_mean += times_sum[m];
        min = times_min[m] < min ? times_min[m] : min;
        max = times_max[m] > max ? times_max[m] : max;
    }
    sum_mean /= EXP_T;
    mean = sum_mean / MATRICES;

    whole_mean    = mean;
    floating_mean = (mean - whole_mean) * 1000;

    whole_min    = min;
    floating_min = (min - whole_min) * 10000;

    whole_max    = max;
    floating_max = (max - whole_max) * 1000;

    printf("[SIMD] Correct counter     %4d/%d\n\r", correct, MATRICES);
    printf("[SIMD] Average exec time\t %d.%.3d μs\n\r", whole_mean,
           floating_mean);
    printf("[SIMD] Minimum exec time\t %d.%.4d μs\n\r", whole_min,
           floating_min);
    printf("[SIMD] Maximum exec time\t %d.%.3d μs\n\r", whole_max,
           floating_max);
    printf("[SIMD] Max iterations\t %d\n\r", max_iter);
    printf("\n\r");

    ret = 0;

    return ret;
}
