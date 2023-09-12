/* -------------------------------------------------------------------------- */
/*                        Auction algorithm, templated                        */
/* -------------------------------------------------------------------------- */

#ifndef _AUCTION_H
#define _AUCTION_H
// #define VERBOSE
#include <cstdio>
#include <cstdlib>
#include <ctime>
#define NO_REPLACE -2
#define NOTHING_FOUND -1
#define MAX_ITER_AUCTION 100000
#include "array.hpp"

enum matrix_type { MLN, MEQN, MGN }; // M<N, M=N, M>N
typedef enum matrix_type matrix_t;

template <typename T = double> struct assignment {
    int agent;
    int object;
    T value;
};

template <typename T = double> struct assignments {
    bool is_empty;
    int size;
    int n_assignment;
    assignment<T> *result;
};

void reset_assignement(auction_array<bool> *array_mask);

template <typename T = double>
void find_available_idx(assignments<T> *result, int *idx) {
    *idx = -1;
    for (int i = 0; i < result->size; i++) {
        if (result->result[i].agent == -1) {
            *idx = i;
            return;
        }
    }
    return;
}

template <typename T = double>
bool check_eCS(auction_array<T> *profits, auction_array<T> *prices,
               auction_array<T> *cost_matrix, const double eps,
               assignments<T> *S) {

    for (int i = 0; i < cost_matrix->rows; i++) {
        for (int j = 0; j < cost_matrix->cols; j++) {
            if (!(profits->data[i] + prices->data[j] >=
                  cost_matrix->data[i * cost_matrix->cols + j] - eps)) {
                return false;
            }
        }
    }

    for (int i = 0; i < S->size; i++) {
        int agent  = S->result[i].agent;
        int object = S->result[i].object;
        if (agent != -1) {
            if (!(profits->data[agent] + prices->data[object] ==
                  cost_matrix->data[agent * cost_matrix->cols + object])) {
                return false;
            }
        }
    }

    return true;
}

template <typename T = double>
void forward(auction_array<T> *cost_matrix, auction_array<T> *prices,
             auction_array<T> *profits, auction_array<bool> *agents_mask,
             auction_array<bool> *objects_mask, assignments<T> *result,
             const double eps, T *lambda, bool *found) {

    bool auction_stop = false;

    for (int i = 0; i < cost_matrix->rows; i++) {
        bool assignment_found = false;
        bool max_found        = false;
        T max1                = -MIN_INF;
        T max2                = -MIN_INF;
        T best_value          = 0;
        int best_obj          = -1;
        // Select an unassigned agent
        if (agents_mask->data[i] == true) {
            continue;
        }
        for (int j = 0; j < cost_matrix->cols; j++) {
            T value = cost_matrix->data[i * cost_matrix->cols + j];
            T diff  = value - prices->data[j];
            // Find best object and associated value
            if (diff > max1) {
                if (max_found) {
                    max2 = max1;
                }
                max1       = diff;
                best_obj   = j;
                best_value = value;
                max_found  = true;
            }
            if (diff > max2 && j != best_obj) {
                max2 = diff;
            }
        }
        if (!max_found) {
            continue;
        }

        T bid                  = best_value - max2 + eps;
        prices->data[best_obj] = *lambda > bid ? *lambda : bid;
        profits->data[i]       = max2 - eps;
        if (*lambda <= bid) {
            agents_mask->data[i]         = true;
            objects_mask->data[best_obj] = true;

            bool new_assignment = true;

            for (int k = 0; k < result->size; k++) {
                int old_object = result->result[k].object;
                int old_agent  = result->result[k].agent;
                if (old_object == best_obj) {
                    agents_mask->data[old_agent] = false;
                    new_assignment               = false;
                    result->result[k].agent      = i;
                    result->result[k].value      = best_value;
                    break;
                }
            }
            if (new_assignment) {
                int free_idx;
                find_available_idx(result, &free_idx);

                result->result[free_idx].agent  = i;
                result->result[free_idx].object = best_obj;
                result->result[free_idx].value  = best_value;
                result->is_empty                = false;
                result->n_assignment++;
            }
            assignment_found = true;
        } else {
            assignment_found = false;
        }
        if (assignment_found) {
            auction_stop = true;
        }
    }
    *found = auction_stop;
}

template <typename T = double>
void simple_forward(auction_array<T> *cost_matrix, auction_array<T> *prices,
                    auction_array<bool> *agents_mask, assignments<T> *result,
                    const double eps) {

    for (int i = 0; i < cost_matrix->rows; i++) {
        bool max_found = false;
        T max1         = -MIN_INF;
        T max2         = -MIN_INF;
        T best_value   = 0;
        int best_obj   = -1;
        // Select an unassigned agent
        if (agents_mask->data[i] == true) {
            continue;
        }
        for (int j = 0; j < cost_matrix->cols; j++) {
            T value = cost_matrix->data[i * cost_matrix->cols + j];
            T diff  = value - prices->data[j];
            // Find best object and associated value
            if (diff > max1) {
                if (max_found) {
                    max2 = max1;
                }
                max1       = diff;
                best_obj   = j;
                best_value = value;
                max_found  = true;
            }
            if (diff > max2 && j != best_obj) {
                max2 = diff;
            }
        }
        if (!max_found) {
            continue;
        }

        T bid = max1 - max2 + eps;
        prices->data[best_obj] += bid;
        agents_mask->data[i] = true;

        bool new_assignment = true;

        for (int k = 0; k < result->size; k++) {
            int old_object = result->result[k].object;
            int old_agent  = result->result[k].agent;
            if (old_object == best_obj) {
                agents_mask->data[old_agent] = false;
                new_assignment               = false;
                result->result[k].agent      = i;
                result->result[k].value      = best_value;
                break;
            }
        }
        if (new_assignment) {
            int free_idx;
            find_available_idx(result, &free_idx);

            result->result[free_idx].agent  = i;
            result->result[free_idx].object = best_obj;
            result->result[free_idx].value  = best_value;
            result->is_empty                = false;
            result->n_assignment++;
        }
    }
}

template <typename T = double>
void backward(auction_array<T> *cost_matrix, auction_array<T> *prices,
              auction_array<T> *profits, auction_array<bool> *agents_mask,
              auction_array<bool> *objects_mask, assignments<T> *result,
              const double eps, T *lambda, bool *found) {
    bool auction_stop = false;

    for (int j = 0; j < cost_matrix->cols; j++) {
        bool assignment_found = false;
        bool max_found        = false;
        T max1                = -MIN_INF;
        T max2                = -MIN_INF;
        T best_value          = 0;
        int best_agent        = -1;
        // Select an unassigned object
        if (objects_mask->data[j] == true) {
            continue;
        }

        if (!(prices->data[j] > *lambda)) {
            continue;
        }
        for (int i = 0; i < cost_matrix->rows; i++) {
            T value = cost_matrix->data[i * cost_matrix->cols + j];
            T diff  = value - profits->data[i];
            // Find best agent and associated value
            if (diff > max1) {
                if (max_found) {
                    max2 = max1;
                }
                max1       = diff;
                best_agent = i;
                best_value = value;
                max_found  = true;
            }
            if (diff > max2 && i != best_agent) {
                max2 = diff;
            }
        }

        if (!max_found) {
            continue;
        }

        if (max1 >= (*lambda + eps)) {

            T bid = max2 - eps;

            prices->data[j]           = *lambda > bid ? *lambda : bid;
            profits->data[best_agent] = best_value - prices->data[j];

            objects_mask->data[j]         = true;
            agents_mask->data[best_agent] = true;

            bool new_assignment = true;

            for (int k = 0; k < result->size; k++) {
                int old_object = result->result[k].object;
                int old_agent  = result->result[k].agent;
                if (old_agent == best_agent) {
                    objects_mask->data[old_object] = false;
                    new_assignment                 = false;
                    result->result[k].object       = j;
                    result->result[k].value        = best_value;
                    break;
                }
            }
            if (new_assignment) {
                int free_idx;
                find_available_idx(result, &free_idx);

                result->result[free_idx].agent  = best_agent;
                result->result[free_idx].object = j;
                result->result[free_idx].value  = best_value;
                result->is_empty                = false;
                result->n_assignment++;
            }
            assignment_found = true;
        } else {
            prices->data[j]        = max1 - eps;
            int n_objs_small_price = 0;
            T new_lambda           = *lambda;

            for (int o = 0; o < prices->cols; o++) {
                if (prices->data[o] < *lambda) {
                    n_objs_small_price++;
                    if (prices->data[o] < new_lambda) {
                        new_lambda = prices->data[o];
                    }
                }
            }
            if (n_objs_small_price >= (cost_matrix->cols - cost_matrix->rows)) {
                *lambda = new_lambda;
            }
            assignment_found = false;
        }
        if (assignment_found) {
            auction_stop = true;
        }
    }
    *found = auction_stop;
}

template <typename T = double>
bool check_prices_lower_than_lambda(auction_array<bool> *objects_mask,
                                    auction_array<T> *prices, T *lambda) {
    for (int i = 0; i < prices->cols; i++) {
        if ((objects_mask->data[i] == false) && (prices->data[i] > *lambda)) {
            return false;
        }
    }
    return true;
}

template <typename T = double>
bool assignment_found(auction_array<bool> *assigned_agents) {
    for (int i = 0; i < assigned_agents->cols; i++) {
        if (!assigned_agents->data[i]) {
            return false;
        }
    }
    return true;
}

template <typename T = double>
void solve_simple(auction_array<T> *cost_matrix, const double eps,
                  assignments<T> *result, int *n_iter) {
    int n_agents  = cost_matrix->rows;
    int n_objects = cost_matrix->cols;

    assert(n_objects == n_agents);

    auction_array<T> prices;
    auction_array<bool> assigned_agents;

    init<T>(&prices, 1, n_objects, 0);
    init<bool>(&assigned_agents, 1, n_agents, false);

    int n_loops = 0;

    while (!assignment_found(&assigned_agents)) {
        n_loops++;
        if (n_loops > MAX_ITER_AUCTION) {
#ifdef VERBOSE
            printf("Maximum iterations reached, exiting.\n");
#endif
        }
        simple_forward(cost_matrix, &prices, &assigned_agents, result, eps);
    }
    *n_iter = n_loops;
}

template <typename T = double>
void solve_jacobi(auction_array<T> *cost_matrix, const double eps,
                  assignments<T> *result, const matrix_t mat_type, int *niter) {
    int n_agents  = cost_matrix->rows;
    int n_objects = cost_matrix->cols;

    auction_array<T> prices;
    init<T>(&prices, 1, n_objects, 0);
    auction_array<T> profits;
    init<T>(&profits, n_agents, 1, 1); // Satisfy eCS (2a)
    T lambda = 0;

    auction_array<bool> assigned_agents;
    init<bool>(&assigned_agents, 1, n_agents, false);
    auction_array<bool> assigned_objects;
    init<bool>(&assigned_objects, 1, n_objects, false);

    int n_loops = 0;

    while (true) {
        bool forward_found  = false;
        bool backward_found = false;
        if (n_loops > MAX_ITER_AUCTION) {
            dump_array("dumped_array_v2.txt", cost_matrix);
#ifdef VERBOSE
            printf("Maximum iterations reached, exiting.\n");
#endif
            break;
        }

        while (!forward_found) {
            forward<T>(cost_matrix, &prices, &profits, &assigned_agents,
                       &assigned_objects, result, eps, &lambda, &forward_found);
            n_loops++;
            if (n_loops > MAX_ITER_AUCTION) {
                printf("Max iterations reached in forward\n");
                dump_array("faulty.txt", cost_matrix);
                exit(EXIT_FAILURE);
            }
        };

        if (!assignment_found(&assigned_agents)) {
            while (!backward_found) {

                backward<T>(cost_matrix, &prices, &profits, &assigned_agents,
                            &assigned_objects, result, eps, &lambda,
                            &backward_found);
                if (check_prices_lower_than_lambda(&assigned_objects, &prices,
                                                   &lambda)) {
                    break;
                }
                n_loops++;
                if (n_loops > MAX_ITER_AUCTION) {
                    printf("Max iterations reached in backward\n");
                    dump_array("faulty.txt", cost_matrix);
                    exit(EXIT_FAILURE);
                }
            }
        }

        if (assignment_found(&assigned_agents)) {
            while (true) {
                backward<T>(cost_matrix, &prices, &profits, &assigned_agents,
                            &assigned_objects, result, eps, &lambda,
                            &backward_found);
                if (check_prices_lower_than_lambda(&assigned_objects, &prices,
                                                   &lambda)) {
                    break;
                }
                n_loops++;
                if (n_loops > MAX_ITER_AUCTION) {
                    printf("Max iterations reached in final backward\n");
                    dump_array("faulty.txt", cost_matrix);
                    exit(EXIT_FAILURE);
                }
            }
            break;
        }
    }
    // std::cout << "Asignment found in " << n_loops << " loops" << std::endl;
    delete[] profits.data;
    delete[] prices.data;
    delete[] assigned_agents.data;
    delete[] assigned_objects.data;
    *niter = n_loops;
}

template <typename T = double>
void assignements_to_arrays(assignments<T> *results,
                            auction_array<int> *agent_to_object,
                            auction_array<int> *row_indexes,
                            matrix_t mat_type) {
    assert(!results->is_empty);
    int agent, object;
    int next_agent_idx = 0;
    int next_obj_idx   = 0;

    for (int i = 0; i < results->size; i++) {
        agent  = results->result[i].agent;
        object = results->result[i].object;
        if (agent != -1 && object != -1) {
            if (mat_type == MLN) {
                row_indexes->data[next_obj_idx++]       = agent;
                agent_to_object->data[next_agent_idx++] = object;
            } else if (mat_type == MGN) {
#ifdef VERBOSE
                printf("Agent %d Object %d\n", agent, object);
#endif
                row_indexes->data[next_obj_idx++] =
                    object; // because of transposition
                agent_to_object->data[next_agent_idx++] = agent;
            } else {
                row_indexes->data[next_agent_idx++] = i;
                agent_to_object->data[agent]        = object;
            }
        }
    }
    if (mat_type != MEQN) {
        sort_together<int>(row_indexes, agent_to_object);
    }
}

#endif // ifndef _AUCTION_H