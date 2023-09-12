/* -------------------------------------------------------------------------- */
/*                        Simple templated array class                        */
/* -------------------------------------------------------------------------- */

#ifndef _ARRAY_H
#define _ARRAY_H
#define ARR_MAX 10000.0
#define MIN_INF 1e32

#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

template <typename T = double> struct auction_array {
    int rows;
    int cols;
    T *data;
};

template <typename T>
void reset_array(auction_array<T> *input_array, const T val) {
    int indx = input_array->cols == 1 ? input_array->rows : input_array->cols;
    for (int i = 0; i < indx; i++) {
        input_array->data[i] = val;
    }
}

template <typename T> void print_array(auction_array<T> *input_array) {
    T *ptr_arr = input_array->data;
    std::cout << std::fixed;
    std::cout << std::setprecision(2);
    std::cout << "[";
    for (int i = 0; i < input_array->rows; i++) {
        for (int j = 0; j < input_array->cols; j++) {
            if (i == input_array->rows - 1 && j == input_array->cols - 1 &&
                i != 0) {
                std::cout << " " << ptr_arr[i * input_array->cols + j];
            } else if (i == 0) {
                if (j != input_array->cols - 1) {
                    std::cout << ptr_arr[i * input_array->cols + j] << " ";
                } else {
                    std::cout << ptr_arr[i * input_array->cols + j];
                }
            } else {
                std::cout << " " << ptr_arr[i * input_array->cols + j];
            }
        }
        if (i < input_array->rows - 1) {
            std::cout << "\n";
        }
    }
    std::cout << "]" << std::endl;
}

template <> inline void print_array<int>(auction_array<int> *input_array) {
    int *ptr_arr = input_array->data;
    std::cout << "[";
    for (int i = 0; i < input_array->rows; i++) {
        for (int j = 0; j < input_array->cols; j++) {
            if (i == input_array->rows - 1 && j == input_array->cols - 1 &&
                i != 0) {
                std::cout << " " << ptr_arr[i * input_array->cols + j];
            } else if (i == 0) {
                if (j != input_array->cols - 1) {
                    std::cout << ptr_arr[i * input_array->cols + j] << " ";
                } else {
                    std::cout << ptr_arr[i * input_array->cols + j];
                }
            } else {
                std::cout << " " << ptr_arr[i * input_array->cols + j];
            }
        }
        if (i < input_array->rows - 1) {
            std::cout << "\n";
        }
    }
    std::cout << "]" << std::endl;
}

template <typename T = double>
void set_size(const int rows, const int cols, auction_array<T> *array) {
    array->cols = cols;
    array->rows = rows;
    array->data = new T[rows * cols];
}

template <typename T = double>
void read_array(const char *array_file, auction_array<T> *output_array) {
    std::ifstream file;
    T tmp_val;

    file.open(array_file);
    if (!file.is_open()) {
        std::cerr << "Incorrect file path:" << array_file << std::endl;
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < output_array->rows; i++) {
        for (int j = 0; j < output_array->cols; j++) {
            file >> tmp_val;
            output_array->data[i * output_array->cols + j] = tmp_val;
        }
    }
    file.close();
}

template <typename T = double>
void dump_array(const char *output_file, auction_array<T> *input_array) {
    std::ofstream file;

    file.open(output_file);
    if (!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < input_array->rows; i++) {
        for (int j = 0; j < input_array->cols; j++) {
            file << input_array->data[i * input_array->cols + j];
            file << " ";
        }
        if (i <= input_array->rows - 1) {
            file << "\n";
        }
    }
    file.close();
}

template <typename T = double> void max_val(auction_array<T> *array, T *max) {
    T *ptr_array = array->data;
    T max_val    = array->data[0];
    for (int i = 0; i < array->rows; i++) {
        for (int j = 0; j < array->cols; j++) {
            if (ptr_array[i * array->cols + j] > max_val) {
                max_val = ptr_array[i * array->cols + j];
            }
        }
    }
    *max = max_val;
}

template <typename T = double> void fill(auction_array<T> *arr, const T val) {
    T *ptr_tab = arr->data;
    for (int i = 0; i < arr->rows; i++) {
        for (int j = 0; j < arr->cols; j++) {
            ptr_tab[i * arr->cols + j] = val;
        }
    }
}

template <typename T = double>
void init(auction_array<T> *array, const int rows, const int cols,
          const T fill_val) {
    set_size(rows, cols, array);
    fill<T>(array, fill_val);
}

template <typename T = double>
void find_top2_with_pos_in_row(auction_array<T> *input_array, int row, T *max1,
                               T *max2, int *pos_max) {
    if (row >= input_array->rows) {
        fprintf(stderr, "Row %d is out of bounds. Arrays has %d rows\n ", row,
                input_array->rows);
        exit(EXIT_FAILURE);
    }
    T *ptr_row = input_array->data + row * input_array->cols;
    T max_val1 = -MIN_INF;
    T max_val2 = -MIN_INF;

    int pos_val = ptr_row[0] > ptr_row[1] ? 0 : 1;

    for (int i = 0; i < input_array->cols; i++) {
        if (ptr_row[i] > max_val1) {
            max_val2 = max_val1;

            max_val1 = ptr_row[i];
            pos_val  = i;
        } else if (ptr_row[i] > max_val2) {
            max_val2 = ptr_row[i];
        }
    }
    max_val2 = (max_val1 == max_val2) ? -MIN_INF : max_val2;
    assert(max_val1 > max_val2 || max_val2 == -MIN_INF);
    *max1    = max_val1;
    *max2    = max_val2;
    *pos_max = pos_val;
}

template <typename T = double>
void find_top1_with_pos_in_row(auction_array<T> *array, int row, T *max,
                               int *pos) {
    T *ptr_row  = array->data + row * array->cols;
    T max_val   = ptr_row[0];
    int pos_val = 0;

    for (int i = 1; i < array->cols; i++) {
        if (ptr_row[i] > max_val) {
            max_val = ptr_row[i];
            pos_val = i;
        }
    }
    *max = max_val;
    *pos = pos_val;
}

template <typename T = double>
void find_top2_with_pos_in_col(auction_array<T> *input_array, int col, T *max1,
                               T *max2, int *pos1) {
    if (col > input_array->cols - 1) {
        fprintf(stderr, "Column %d is out of bounds. Arrays has %d columns\n ",
                col, input_array->cols);
        exit(EXIT_FAILURE);
    }
    T *ptr_col   = input_array->data;
    int pos_val1 = ptr_col[col] > ptr_col[input_array->cols + col]
                       ? col
                       : input_array->cols + col;

    T max_val1 = MIN_INF;
    T max_val2 = MIN_INF;

    // puts("[C]: Searching top2 values in array");
    // print_array(input_array);
    for (int i = 0; i < input_array->rows; i++) {
        if (ptr_col[i * input_array->cols + col] > max_val1) {
            max_val2 = max_val1;

            max_val1 = ptr_col[i * input_array->cols + col];
            pos_val1 = i;
        } else if (ptr_col[i * input_array->cols + col] > max_val2) {
            max_val2 = ptr_col[i * input_array->cols + col];
        }
    }
    // std::cout << "Treating column: " << col << std::endl;
    // std::cout << "Max 1: " << max_val1 << " Max 2: " << max_val2 <<
    // std::endl;
    max_val2 = (max_val1 == max_val2) ? MIN_INF : max_val2;
    assert(max_val1 > max_val2 || max_val2 == MIN_INF);
    *max1 = max_val1;
    *max2 = max_val2;
    *pos1 = pos_val1;
}

template <typename T = double>
void add_val_to_row(auction_array<T> *array, int row, T val) {
    T *ptr_row = array->data + row * array->cols;
    for (int i = 0; i < array->cols; i++) {
        ptr_row[i] += val;
    }
}

template <typename T = double>
void sub_val_to_row(auction_array<T> *array, int row, T val) {
    T *ptr_row = array->data + row * array->cols;
    for (int i = 0; i < array->cols; i++) {
        ptr_row[i] -= val;
    }
}

template <typename T = double>
void sub_vals_to_arr_row(auction_array<T> *arr, int row,
                         auction_array<T> *vals) {
    T *ptr_arr  = arr->data;
    T *ptr_vals = vals->data;

    if (arr->cols != vals->cols) {
        fprintf(stderr,
                "Row should have the same length as array. Arrays has %d "
                "columns, row has %d columns.\n",
                arr->cols, vals->cols);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < arr->cols; i++) {
        ptr_arr[row * arr->cols + i] =
            ptr_arr[row * arr->cols + i] - ptr_vals[i];
    }
}

template <typename T = double>
void add_val_to_col(auction_array<T> *array, int col, T val) {
    T *ptr_col = array->data;
    for (int i = 0; i < array->rows; i++) {
        ptr_col[i * array->cols + col] += val;
    }
}

template <typename T = double>
void sub_val_to_col(auction_array<T> *array, int col, T val) {
    T *ptr_col = array->data;
    for (int i = 0; i < array->rows; i++) {
        ptr_col[i * array->cols + col] -= val;
    }
}

template <typename T = double> void swap(T *a, T *b) {
    T tmp = *b;
    *b    = *a;
    *a    = tmp;
}

template <typename T = double>
void sort_together(auction_array<T> *array1, auction_array<T> *array2) {
    for (int i = 0; i < array1->cols - 1; i++) {
        for (int j = 0; j < array1->cols - i - 1; j++) {
            if (array1->data[j] > array1->data[j + 1]) {
                swap<T>(array1->data + j, array1->data + j + 1);
                swap<T>(array2->data + j, array2->data + j + 1);
            }
        }
    }
}

template <typename T = double>
void transpose(auction_array<T> *input, auction_array<T> *output) {
    for (int i = 0; i < input->rows; i++) {
        for (int j = 0; j < input->cols; j++) {
            output->data[j * input->rows + i] =
                input->data[i * input->cols + j];
        }
    }
}

#endif
