#include "auction.hpp"

void reset_assignement(auction_array<bool> *array_mask) {
    int indx = array_mask->cols == 1 ? array_mask->rows : array_mask->cols;
    for (int i = 0; i < indx; i++) {
        array_mask->data[i] = false;
    }
}