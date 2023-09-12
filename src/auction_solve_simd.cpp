#include "auction_solve_simd.hpp"

void solve_auction_simd(val_t *cost_matrix, object_t *a2o, int *forward_cnt) {
    // Begin with forward auction
    // _Check if assignment found
    // Do a backward auction

    // Array to store updated prices after bidding phase.
    val_t prices[__COLS];
    val_t o2a[__COLS];

    bool_t agents_mask[__ROWS];

    for (int i = 0; i < __ROWS; i++) {
        a2o[i]         = -1;
        agents_mask[i] = false;
    }

    for (int i = 0; i < __COLS; i++) {
        prices[i] = 0;
        o2a[i]    = -1;
    }

    int loop_cnt;
    int agents_cnt = 0;

    for (loop_cnt = 0; loop_cnt < MAX_ITER; loop_cnt++) {
        forward_simd<__ROWS, __COLS>(cost_matrix, prices, agents_mask, a2o, o2a,
                                     &agents_cnt);
        *forward_cnt += 1;
        if (agents_cnt == __ROWS) {
            break;
        }
    }
}
