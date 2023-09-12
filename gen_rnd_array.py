# ---------------------------------------------------------------------------- #
#          Script to generate random assignement problems by creating          #
# ---------------------------------------------------------------------------- #
import numpy as np
from scipy.optimize import linear_sum_assignment
import argparse
import time
import os, sys


def gen(_range=[10, 300], type="float", size=(3, 3), samples=1, agents=None):
    arrs = []
    results = []
    for i in range(samples):
        if type == "float":
            arr = np.random.uniform(0.0, 0.999, size=size)
        elif type == "integer":
            arr = np.random.randint(_range[0], _range[1], size)
        if (agents is not None) and (agents < arr.shape[0]):
            for i in range(arr.shape[1]):
                mask = np.full(arr.shape[0], True)
                mask[np.random.choice(mask.shape[0], agents,
                                      replace=False)] = False
                arr[i, mask.astype(bool)] = 0
        _, rows = linear_sum_assignment(-arr)
        arrs.append(arr)
        results.append(rows)
    return arrs, results


def write(arr, output_file):
    with open(output_file, 'w') as f:
        for row in range(arr.shape[0]):
            for col in range(arr.shape[1]):
                f.write(f"{arr[row, col]} ")
            if row < arr.shape[0] - 1:
                f.write("\n")


def write_c_format(arrays, output_file, results):
    with open(output_file, 'w') as f:
        f.write(f"#define MATRICES {len(arrays)}\n\n")

        for (i, array), res in zip(enumerate(arrays), results):
            f.write(f"double arr{i}[{array.shape[0] * array.shape[1]}] = " +
                    "{\n")
            for r, row in enumerate(array):
                f.write("    ") if r == 0 else None
                for c, col in enumerate(row):
                    if (c < array.shape[1] - 1 and r < array.shape[0]):
                        f.write(f"{col:.4f}, ")
                    elif (c == array.shape[1] - 1 and r == array.shape[0] - 1):
                        f.write(f"{col:.4f}")
                    else:
                        f.write(f"{col:.4f},")
                f.write("\n    ") if r < array.shape[0] - 1 else None
            f.write("};\n")
            f.write(f"\nint res{i}[{array.shape[0]}] = {{")
            for lv, val in enumerate(res):
                f.write(f"{val}, " if lv < res.shape[0] - 1 else f"{val}}};")
            f.write("\n\n")

        f.write(f"double *arrays[{len(arrays)}] = {{ ")
        arr_txt = ", ".join([f"arr{i}" for i in range(len(arrays))])
        f.write(arr_txt)
        f.write(" };\n")
        res_txt = ", ".join([f"res{i}" for i in range(len(arrays))])
        f.write(f"int *results[{len(arrays)}] = {{ ")
        f.write(res_txt)
        f.write(" };\n\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument("--rows", help="Number of rows", type=int)
    parser.add_argument("--cols", help="Number of cols", type=int)
    parser.add_argument("--float", action="store_true", help="Use floats")
    parser.add_argument("--range",
                        nargs=2,
                        help="Range for numbers to generate",
                        required=False)
    parser.add_argument("--output",
                        type=str,
                        help="Output file",
                        default="array.txt")
    parser.add_argument("--samples",
                        type=int,
                        help="Number of samples to generate",
                        default=1)
    parser.add_argument("--agents",
                        type=int,
                        help="Matrix density",
                        default=None)
    parser.add_argument("--batch_agents",
                        nargs='+',
                        help="Batch agents per line",
                        type=int)

    args = parser.parse_args()
    size = (args.rows, args.cols)
    _type = "float" if args.float else "integer"

    if args.batch_agents is not None:
        for agents in args.batch_agents:
            arrays, res = gen(args.range,
                              _type,
                              size,
                              samples=args.samples,
                              agents=agents)
            os.makedirs(f"{size[0]}x{size[1]}", exist_ok=True)
            write_c_format(
                arrays,
                f"{size[0]}x{size[1]}/data_{size[0]}x{size[1]}_{agents}_agents.hpp",
                res)
    else:
        arrays, res = gen(args.range,
                          _type,
                          size,
                          samples=args.samples,
                          agents=args.agents)
        # print(arrays)
        write_c_format(arrays, args.output, res)