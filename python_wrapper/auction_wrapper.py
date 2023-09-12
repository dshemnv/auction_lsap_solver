# ---------------------------------------------------------------------------- #
#                Python script to load and use auction C library               #
# ---------------------------------------------------------------------------- #

from ctypes import *
import numpy as np
from scipy.sparse import random
from scipy.optimize import linear_sum_assignment
import time
from pprint import pprint


class array(Structure):
    _fields_ = [("cols", c_int), ("rows", c_int), ("data", POINTER(c_double))]


class assignement_result(Structure):
    _fields_ = [("agent_to_object", POINTER(c_int)),
                ("row_idx", POINTER(c_int)), ("len", c_int)]


def convert_np_to_double_ptr(numpy_array: np.ndarray):
    cols, rows = numpy_array.shape
    arr = numpy_array.astype(np.float64)
    ptr_arr = arr.ctypes.data_as(POINTER(c_double))
    return ptr_arr, (cols, rows)


def convert_double_ptr_to_d_array(ptr_array, rows, cols):
    return array(cols=cols, rows=rows, data=ptr_array)


def convert_assignment_result_to_np(assignment_result, shape):
    return (np.ctypeslib.as_array(assignment_result.contents.agent_to_object,
                                  shape=shape),
            np.ctypeslib.as_array(assignment_result.contents.row_idx,
                                  shape=shape))


def load_from_txt(file_path):
    arr = []
    with open(file_path, "r") as f:
        for line in f.readlines():
            line = line.strip().split(" ")
            line = list(map(lambda x: float(x), line))
            arr.append(line)
    return np.array(arr)


def save_as_text(arr, path):
    rows, _ = arr.shape
    with open(path, "w") as f:
        for row in range(rows):
            for val in arr[row, :]:
                f.write(f"{val:.3f} ")
            if row != rows - 1:
                f.write("\n")


class CAuctionWrapper:

    def __init__(self, lib_path) -> None:
        self.lib = CDLL(lib_path)

    def solve_auction(self, cost_matrix: np.ndarray, eps: float = 0.01):
        c_solver = self.lib.solve
        c_solver.argtypes = [POINTER(array), c_float]
        c_solver.restype = POINTER(assignement_result)

        # print(" [Py]: Convert numpy to ptr to array of doubles")
        p_cost_matrix_d, (cols, rows) = convert_np_to_double_ptr(cost_matrix)
        # print(" [Py]: Convert ptr of doubles to ptr to structure d_array")
        c_cost_matrix = convert_double_ptr_to_d_array(p_cost_matrix_d, rows,
                                                      cols)

        # print(" [Py]: Calling C solver")
        t1 = time.perf_counter()
        result = c_solver(c_cost_matrix, eps)
        res_len = result.contents.len
        t = time.perf_counter() - t1
        # print(" [Py]: C solver end")
        return convert_assignment_result_to_np(result, (res_len, )), t

    def solve_forward_simple(self, cost_matrix: np.ndarray, eps: float = 0.01):
        c_solver = self.lib.solve_forward
        c_solver.argtypes = [POINTER(array), c_float]
        c_solver.restype = POINTER(assignement_result)

        # print(" [Py]: Convert numpy to ptr to array of doubles")
        p_cost_matrix_d, (cols, rows) = convert_np_to_double_ptr(cost_matrix)
        # print(" [Py]: Convert ptr of doubles to ptr to structure d_array")
        c_cost_matrix = convert_double_ptr_to_d_array(p_cost_matrix_d, rows,
                                                      cols)

        # print(" [Py]: Calling C solver")
        t1 = time.perf_counter()
        result = c_solver(c_cost_matrix, eps)
        res_len = result.contents.len
        t = time.perf_counter() - t1
        # print(" [Py]: C solver end")
        return convert_assignment_result_to_np(result, (res_len, )), t


def gen_test_arrays(size, n_samples):
    return [np.random.uniform(0.0, 0.999, size=size) for i in range(n_samples)]


def cost_calc(assignment_vector, cost_matrix):
    return np.sum(cost_matrix[assignment_vector, range(cost_matrix.shape[0])])


def perform_bench(sizes, n_samples, n_exp, agents=None, func: callable = None):
    time_res = {"mean": 0, "max": 0, "min": 0}
    good_cnt = 0
    results = {size: time_res.copy() for size in sizes}
    for size in sizes:
        test_mats = gen_test_arrays(size, n_samples)
        if (agents is not None) and (agents < size[0]):
            for mat in test_mats:
                for i in range(mat.shape[1]):
                    mask = np.full(mat.shape[0], True)
                    mask[np.random.choice(mask.shape[0], agents,
                                          replace=False)] = False
                    mat[i, mask.astype(bool)] = 0.0
        all_times = []
        good_cnt = 0
        for mat in test_mats:
            good_flag = False
            for _ in range(n_exp):
                rows, cols = linear_sum_assignment(-mat)
                if (func is not None):
                    (agent_to_object, row_idx), c_auction_t = func(mat, 0.0001)
                else:
                    (agent_to_object,
                     row_idx), c_auction_t = c_auction.solve_auction(
                         mat, 0.0001)
                # if np.array_equal(cols, agent_to_object):
                #     all_times.append(c_auction_t * 1000000)
                #     good_flag = True

                print(
                    f"Auction cost {cost_calc(agent_to_object, mat):.3f}, Hungarian cost: {cost_calc(cols, mat):.3f}"
                )

                if (np.abs(
                        cost_calc(cols, mat) - cost_calc(agent_to_object, mat))
                        / cost_calc(cols, mat) <= 0.1) and (all(
                            agent_to_object != -1)):
                    all_times.append(c_auction_t * 1000000)
                    good_flag = True
            if good_flag:
                good_cnt += 1
        if all_times:
            results[size]["max"] = f"{np.max(all_times):.3f}"
            results[size]["min"] = f"{np.min(all_times):.3f}"
            results[size]["mean"] = f"{np.mean(all_times):.3f}"
        print(f"Precision for {size}, {(good_cnt/n_samples) * 100:.3f}")
    return results


if __name__ == "__main__":
    c_auction = CAuctionWrapper(
        "/home/dshem/Documents/these/ressources/papers/rapido2023/experiments/auction_c/bin/cauction_lib.so"
    )
    # test_mat = np.array([[0.08411499, 0.92580858, 1.78757778,  0.71264366],
    # [1.14656623,  0.85612268, 1.35644848, 0.15593028],
    # [2.30138558,  0.03428558,  0.06794686,  0.94001399],
    # [0.09391892,  0.33946436, 0.4026701,   1.52602039]])

    # np.random.seed(10)
    # test_mat = np.abs(np.random.randn(20, 10))
    # test_mat = random(10, 5, density=0.3, dtype=np.double).A
    # print(test_mat.shape)
    # test_mat = load_from_txt("sparse_arr.txt")
    # test_mat = load_from_txt("assym_array.txt")
    # print("[Py]: Loaded matrix from txt to np")

    # results = perform_bench(
    #     ((8, 8), (16, 16), (24, 24), (32, 32), (48, 48), (64, 64)), 100, 10)
    results = perform_bench([(5000, 5000)],
                            1,
                            1,
                            5000,
                            func=c_auction.solve_forward_simple)
    pprint(results)
    # print("[Py]: Calling solver")
    # (agent_to_object,
    #  row_idx), c_auction_t = c_auction.solve_auction(test_mat, 0.0001)
    # print("[Py]: Solver finished")
    # t1 = time.perf_counter()
    # rows, cols = linear_sum_assignment(-test_mat)
    # hung_t = time.perf_counter() - t1

    # print(test_mat)
    # print("C Auction result:")
    # print(f"Took {c_auction_t * 1000000:.3f} µs")
    # print(row_idx)
    # print(agent_to_object)
    # print("Hungarian result")
    # print(f"Took {hung_t * 1000000:.3f} µs")
    # print(rows)
    # print(cols)