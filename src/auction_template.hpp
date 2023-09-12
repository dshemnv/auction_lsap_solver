#ifndef _AUCTION_TEMPLATE
#define _AUCTION_TEMPLATE

#include "constants.hpp"

template <int rows, int cols>
void inline forward_template(val_t *cost_matrix, val_t *prices,
                             bool_t *agents_mask, object_t *agent_to_object,
                             agent_t *object_to_agent, int *agents_cnt) {
    // Forward iteration steps:
    // - Select an unassigned agent
    // - Find best an object that maximizes agent's benefits and calculate bid
    // - Update object price with bid or lambda
    // - Create a new assignment / update existing assignment if bid >= lambda
    // - Repeat for each unassigned agent
    int index_offset = 0;

FORWARD_ROWS_LOOP:
    for (int agent = 0; agent < rows; agent++) {
        bool assignment_found = false;
        if (agents_mask[agent] == true) {
            continue;
        }

        //* BEGIN FIND BEST OBJECT
        val_t max1, max2, best_cost, bid;
        object_t current_winner;

        max1           = -MIN_INF;
        max2           = -MIN_INF;
        best_cost      = 0;
        current_winner = -1;
        index_offset   = agent * cols;

    FIND_OBJECT_LOOP:
        for (int j = 0; j < cols; j++) {
            val_t value   = cost_matrix[index_offset + j];
            val_t benefit = value - prices[j];
            if (benefit > max1) {
                max2           = max1;
                max1           = benefit;
                current_winner = j;
                best_cost      = value;
            } else if (benefit > max2) {
                max2 = benefit;
            }
        }

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

#endif // ifndef _AUCTION_H
