#define TYPE double
#include "array.hpp"
#include "auction.hpp"
#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include <string>

using namespace std;
int main(int argc, char *argv[]) {
    static int show;
    static struct option long_options[] = {
        {"rows",    required_argument, NULL,  'r'},
        {"cols",    required_argument, NULL,  'c'},
        {"array",   required_argument, NULL,  'i'},
        {"epsilon", required_argument, NULL,  'e'},
        {"show",    no_argument,       &show, 1  }
    };

    if (argc < 3) {
        cerr << "Usage: \n"
             << "\t" << argv[0] << " -r rows -c cols -i input_file -e epsilon"
             << endl;
        exit(EXIT_FAILURE);
    }

    int n_agents     = 0;
    int n_objects    = 0;
    double epsilon   = 0;
    const char *path = NULL;

    int c;
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "r:c:i:e:", long_options,
                            &option_index)) != -1) {
        switch (c) {
        case ('r'):
            n_agents = atoi(optarg);
            break;
        case ('c'):
            n_objects = atoi(optarg);
            break;
        case ('i'):
            path = optarg;
            break;
        case ('e'):
            epsilon = atof(optarg);
        default:
            break;
        }
    }

    auction_array<TYPE> A;
    init<TYPE>(&A, n_agents, n_objects, 0);

    matrix_t mat_type;
    if (n_agents == n_objects) {
        mat_type = MEQN;
    } else if (n_agents > n_objects) {
        mat_type = MGN;
    } else if (n_agents < n_objects) {
        mat_type = MLN;
    }

    read_array<TYPE>(path, &A);
    if (show == 1) {
        cout << "Loaded array:" << endl;
        print_array(&A);
    }

    if (mat_type == MGN) {
        auction_array<TYPE> tA;
        init<TYPE>(&tA, n_objects, n_agents, 0);
        transpose<TYPE>(&A, &tA);

        if (epsilon == 0) {
            epsilon = 1e-3 / n_objects;
        }

        assignments<TYPE> result;
        assignment<TYPE> assig = {.agent = -1, .object = -1, .value = -1};
        result.is_empty        = true;
        result.size            = tA.rows;
        result.n_assignment    = 0;
        result.result          = new assignment<TYPE>[result.size];
        for (int i = 0; i < result.size; i++) {
            result.result[i] = assig;
        }

        int niter = 0;
        clock_t t;
        t = clock();
        solve_jacobi<TYPE>(&tA, epsilon, &result, mat_type, &niter);
        t = clock() - t;

        double timing = (double)t / CLOCKS_PER_SEC;
        cout << setprecision(4);
        cout << "Assignment took " << timing * 1000.0 << "ms" << endl;

        auction_array<int> agent_to_object;
        auction_array<int> indexes;

        init<int>(&agent_to_object, 1, result.n_assignment, -1);
        init<int>(&indexes, 1, result.n_assignment, -1);

        assignements_to_arrays<TYPE>(&result, &agent_to_object, &indexes,
                                     mat_type);
        if (show == 1) {
            print_array<int>(&indexes);
            print_array<int>(&agent_to_object);
        }

        delete[] tA.data;
        delete[] agent_to_object.data;
        delete[] indexes.data;
        delete[] result.result;
    } else {

        if (epsilon == 0) {
            epsilon = 1e-3 / n_agents;
        }

        assignments<TYPE> result;
        assignment<TYPE> assig = {.agent = -1, .object = -1, .value = -1};
        result.is_empty        = true;
        result.size            = A.rows;
        result.n_assignment    = 0;
        result.result          = new assignment<TYPE>[result.size];
        for (int i = 0; i < result.size; i++) {
            result.result[i] = assig;
        }

        int niter = 0;
        clock_t t;
        t = clock();
        solve_jacobi<TYPE>(&A, epsilon, &result, mat_type, &niter);
        t = clock() - t;

        double timing = (double)t / CLOCKS_PER_SEC;
        cout << setprecision(4);
        cout << "Assignment took " << timing * 1000.0 << "ms" << endl;

        auction_array<int> agent_to_object;
        auction_array<int> indexes;

        init<int>(&agent_to_object, 1, result.n_assignment, -1);
        init<int>(&indexes, 1, result.n_assignment, -1);

        assignements_to_arrays<TYPE>(&result, &agent_to_object, &indexes,
                                     mat_type);
        if (show == 1) {
            print_array<int>(&indexes);
            print_array<int>(&agent_to_object);
        }
        delete[] agent_to_object.data;
        delete[] indexes.data;
        delete[] result.result;
    }

    delete[] A.data;
    return EXIT_SUCCESS;
}
