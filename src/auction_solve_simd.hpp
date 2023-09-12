#ifndef _solve_auction_simd_
#define _solve_auction_simd_

#include "constants.hpp"
#include "auction_simd.hpp"

#ifdef MULT
#define eps ((int)(0.0001 * MULT))
#else
#define eps 0.01
#endif

void solve_auction_simd(val_t *cost_matrix, object_t *out, int *forward_cnt);

#endif
