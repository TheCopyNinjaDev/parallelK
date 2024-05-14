#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>

#define MAX_V 20  // Maximum number of vertices

// Adjacency matrix representation of graphs
bool adj1[MAX_V][MAX_V];
bool adj2[MAX_V][MAX_V];
int n1, n2;  // Number of vertices in graph 1 and graph 2

// Arrays to store the mappings
int map[MAX_V];    // map[i] = j means vertex i in graph1 is mapped to vertex j in graph2
bool used[MAX_V];  // To keep track of used vertices in graph2

// Check if vertex `v1` can be mapped to vertex `v2`
bool isMappable(int v1, int v2) {
    if (adj1[v1][v1] != adj2[v2][v2]) {
        return false;  // Vertices must have the same degree
    }

    for (int i = 0; i < n1; i++) {
        if (adj1[v1][i] != adj2[v2][i]) {
            return false;  // Adjacent vertices must have matching degrees
        }
    }

    return true;
}

// Ullman's algorithm to check if graphs are isomorphic (parallelized with MPI)
bool isIsomorphicUtil(int depth) {
    if (depth == n1) {
        return true;  // All vertices of graph1 are mapped
    }

    int my_rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    for (int v2 = my_rank; v2 < n2; v2 += num_procs) {
        if (!used[v2] && isMappable(depth, v2)) {
            map[depth] = v2;
            used[v2] = true;

            if (isIsomorphicUtil(depth + 1)) {
                return true;
            }

            used[v2] = false;  // Backtrack
        }
    }

    return false;
}

// Wrapper function to check isomorphism using MPI
bool isIsomorphicMPI() {
    if (n1 != n2) {
        return false;  // Graphs must have the same number of vertices
    }

    for (int i = 0; i < n1; i++) {
        map[i] = -1;  // Initialize mapping as unmapped
    }

    return isIsomorphicUtil(0);
}

// Example usage
int main(int argc, char *argv[]) {
    int my_rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    double start = MPI_Wtime();

    // Initialize adjacency matrices for two graphs
    n1 = 4;
    n2 = 4;

    // Example graphs (adjacency matrices)
    bool graph1[MAX_V][MAX_V] = {
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 1, 0, 1},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 1, 0, 1},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 1, 0, 1},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 1, 0, 1},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 1, 0, 1},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 1, 0, 1},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
    };

    bool graph2[MAX_V][MAX_V] = {
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 1, 0, 1},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 1, 0, 1},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 1, 0, 1},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 1, 0, 1},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 1, 0, 1},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {0, 1, 0, 1},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
    };

    // Copy graph1 and graph2 to adjacency matrices
    for (int i = 0; i < n1; i++) {
        for (int j = 0; j < n1; j++) {
            adj1[i][j] = graph1[i][j];
            adj2[i][j] = graph2[i][j];
        }
    }

    bool isomorphic = isIsomorphicMPI();

    if (my_rank == 0) {
        if (isomorphic) {
            printf("Graphs are isomorphic!\n");
        } else {
            printf("Graphs are not isomorphic.\n");
        }
    }

    double end = MPI_Wtime();

    MPI_Finalize();


    if (my_rank == 0) {
        printf("\nTime measured: %f", end - start);
    }
    return 0;
}
