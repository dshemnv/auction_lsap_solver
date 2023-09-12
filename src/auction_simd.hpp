#ifndef _AUCTION_SIMD
#define _AUCTION_SIMD

#include "constants.hpp"
#include <arm_neon.h>

inline int mini(const int a, const int b) { return (a < b) ? a : b; }

inline int maxi(const int a, const int b) { return (a > b) ? a : b; }

inline int32x4_t shuffle_s32_1032(const int32x4_t a) {
    int32x2_t a32 = vget_high_s32(a);
    int32x2_t b10 = vget_low_s32(a);
    return vcombine_s32(a32, b10);
}

inline int32x4_t shuffle_s32_2301(const int32x4_t a) {
    int32x2_t a01 = vrev64_s32(vget_low_s32(a));
    int32x2_t b23 = vrev64_s32(vget_high_s32(a));
    return vcombine_s32(a01, b23);
}

template <int size = 16>
void inline vec_forward(const int *cost_matrix, const int *prices, int &_idx1,
                        int &_max1, int &_max2, int &_bestcost) {
    const int32x4_t increment = {4, 4, 4, 4};
    int32x4_t indices         = {0, 1, 2, 3};
    int32x4_t idx1            = indices;

    int32x4_t cm   = vld1q_s32(cost_matrix);
    int32x4_t pr   = vld1q_s32(prices);
    int32x4_t max1 = cm - pr;
    int32x4_t max2 = vdupq_n_s32(0);

    for (int i = 4; i < size; i += 4) {
        indices                = vaddq_s32(indices, increment);
        const int32x4_t rn     = vld1q_s32(cost_matrix + i);
        const int32x4_t in     = vld1q_s32(prices + i);
        const int32x4_t values = rn - in;

        const int32x4_t temp = vminq_s32(values, max1);
        max2                 = vmaxq_s32(max2, temp);

        const uint32x4_t gt = vcgtq_s32(values, max1);
        idx1                = vbslq_s32(gt, indices, idx1);
        max1                = vmaxq_s32(values, max1);
    }

#if 0
    int32x4_t l_max1 = shuffle_s32_1032( max1 );
    int32x4_t l_max2 = shuffle_s32_1032( max2 );
    int32x4_t l_idx1 = shuffle_s32_1032( idx1 );

    max2      = vmaxq_s32(max2,       l_max2); // new max 2
    auto temp = vminq_s32(l_max1,     max1);  // new max1
    max2      = vmaxq_s32(max2,       temp);

    auto  gt  = vcgtq_s32(l_max1,     max1);
    idx1      = vbslq_s32(gt, l_idx1, idx1);
    max1      = vmaxq_s32(l_max1,     max1);
    l_max1    = shuffle_s32_2301( max1 );
    l_max2    = shuffle_s32_2301( max2 );
    l_idx1    = shuffle_s32_2301( idx1 );

    max2      = vmaxq_s32(max2,       l_max2); // new max 2
    temp      = vminq_s32(l_max1,     max1);   // new max 1
    max2      = vmaxq_s32(max2,       temp);

    gt        = vcgtq_s32(l_max1,     max1);
    idx1      = vbslq_s32(gt, l_idx1, idx1);
    max1      = vmaxq_s32(l_max1,     max1);
    _idx1     = vgetq_lane_s32(idx1, 0);
    _max1     = vgetq_lane_s32(max1, 0);
    _max2     = vgetq_lane_s32(max2, 0);
    _bestcost = cost_matrix[_idx1];

#else

    int32_t t_max1[4];
    int32_t t_max2[4];
    int32_t t_idx[4];

    vst1q_s32(t_max1, max1);
    vst1q_s32(t_max2, max2);
    vst1q_s32(t_idx, idx1);
    /*
        printf("t_max1 %5d %5d %5d %5d\n", (int)t_max1[0], (int)t_max1[1],
       (int)t_max1[2], (int)t_max1[3]); printf("t_max2 %5d %5d %5d %5d\n",
       (int)t_max2[0], (int)t_max2[1], (int)t_max2[2], (int)t_max2[3]);
        printf("t_idx  %5d %5d %5d %5d\n", (int)t_idx[0],  (int)t_idx[1],
       (int)t_idx[2],  (int)t_idx[3]);
    */
    int t1_max1, t2_max1, t3_max1;
    int t1_max2, t2_max2, t3_max2;
    int t1_idx, t2_idx, t3_idx;

    if (t_max1[0] >= t_max1[1]) {
        t1_max1 = t_max1[0];
        t1_max2 = (t_max2[0] > t_max1[1] ? t_max2[0] : t_max1[1]);
        t1_idx  = t_idx[0];
    } else {
        t1_max1 = t_max1[1];
        t1_max2 = (t_max2[1] > t_max1[0] ? t_max2[1] : t_max1[0]);
        t1_idx  = t_idx[1];
    }

    if (t_max1[2] >= t_max1[3]) {
        t2_max1 = t_max1[2];
        t2_max2 = (t_max2[2] > t_max1[3] ? t_max2[2] : t_max1[3]);
        t2_idx  = t_idx[2];
    } else {
        t2_max1 = t_max1[3];
        t2_max2 = (t_max2[3] > t_max1[2] ? t_max2[3] : t_max1[2]);
        t2_idx  = t_idx[3];
    }

    if (t1_max1 >= t2_max1) {
        t3_max1 = t1_max1;
        t3_max2 =
            maxi(t2_max1, t1_max2); //(t2_max1 > t1_max2 ? t2_max1 : t1_max2);
        t3_idx = t1_idx; //(t1_max1 !=  t2_max1) ? t1_idx : (t1_idx < t2_idx ?
                         // t1_idx : t2_idx);
    } else {
        t3_max1 = t2_max1;
        t3_max2 =
            maxi(t1_max1, t2_max2); //((t1_max1 > t2_max2 ? t1_max1 : t2_max2);
        t3_idx = t2_idx; //(t1_max1 !=  t2_max1) ? t2_idx : (t1_idx < t2_idx ?
                         // t1_idx : t2_idx);
    }

#endif

    _idx1     = t3_idx;
    _max1     = t3_max1;
    _max2     = t3_max2;
    _bestcost = cost_matrix[t3_idx];
}

template <int rows, int cols>
void inline forward_simd(val_t *cost_matrix, val_t *prices, bool_t *agents_mask,
                         object_t *agent_to_object, agent_t *object_to_agent,
                         int *agents_cnt) {
    // Forward iteration steps:
    // - Select an unassigned agent
    // - Find best an object that maximizes agent's benefits and calculate bid
    // - Update object price with bid or lambda
    // - Create a new assignment / update existing assignment if bid >= lambda
    // - Repeat for each unassigned agent
    int index_offset = 0;

FORWARD_ROWS_LOOP:
    for (int agent = 0; agent < rows; agent++) {
        if (agents_mask[agent] == true) {
            continue;
        }

        //* BEGIN FIND BEST OBJECT
        val_t max1, max2, best_cost, bid;
        object_t current_winner;

        vec_forward<cols>(cost_matrix + (agent * cols), prices, current_winner,
                          max1, max2, best_cost);

        // If no max found, impossible to assign the agent
        if (current_winner == -1) {
            continue;
        }
        //* END FIND BEST OBJECT

        // Update prices and profits
        bid = max1 - max2 + eps;
        prices[current_winner] += bid;

        //* BEGIN UPDATE ASSIGNMENT
        agents_mask[agent] = true;
        *agents_cnt += 1;

        int old_agent = object_to_agent[current_winner];
        if (old_agent != -1) {
            agent_to_object[old_agent] = -1;
            agents_mask[old_agent]     = false;
            *agents_cnt -= 1;
        }
        agent_to_object[agent]          = current_winner;
        object_to_agent[current_winner] = agent;
        //* END UPDATE ASSIGNMENT
    }
}

#endif
