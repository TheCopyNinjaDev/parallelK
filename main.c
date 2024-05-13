#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

// Function to find the order statistic in an array
int order_statistics(int *a, int n, int k) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Parallel code will go here
    // For now, let's assume we're just finding the k-th order statistic
    // without parallelization as an example

    int i, j, l = 0, r = n-1, mid, swap;
    while (1) {
        if (r <= l + 1) {
            if (r == l + 1 && a[r] < a[l])
                swap = a[l], a[l] = a[r], a[r] = swap;
            return a[k];
        }

        mid = (l + r) >> 1;
        swap = a[mid], a[mid] = a[l + 1], a[l + 1] = swap;
        if (a[l] > a[r])
            swap = a[l], a[l] = a[r], a[r] = swap;
        if (a[l + 1] > a[r])
            swap = a[l + 1], a[l + 1] = a[r], a[r] = swap;
        if (a[l] > a[l + 1])
            swap = a[l], a[l] = a[l + 1], a[l + 1] = swap;

        i = l + 1;
        j = r;
        int cur = a[l + 1];
        while (1) {
            while (a[++i] < cur);
            while (a[--j] > cur);
            if (i > j)
                break;
            swap = a[i], a[i] = a[j], a[j] = swap;
        }

        a[l + 1] = a[j];
        a[j] = cur;

        if (j >= k)
            r = j - 1;
        if (j <= k)
            l = i;
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start = MPI_Wtime();

    int n = 100000000; // Total size of the array
    int k = 32935543;  // The order statistic we're looking for
    int *a = NULL;
    int local_n = n / size; // Size of the sub-array for each process

    // Allocate memory for the sub-array on each process
    int *local_a = malloc(local_n * sizeof(int));

    // Seed the random number generator
    srand(time(NULL) + rank);

    // Each process fills its sub-array with random data
    for (int i = 0; i < local_n; i++) {
        local_a[i] = rand() % 100; // Random numbers between 0 and 99
    }

    // // Initialize the sub-array with the given values
    // int values[] = {12, 1, 23, 213, 13, 2, 4, 6, 8, 9};
    // for (int i = 0; i < local_n; i++) {
    //     local_a[i] = values[i + rank * local_n];
    // }

    // Gather all sub-arrays into the root process
    if (rank == 0) {
        a = malloc(n * sizeof(int));
    }
    MPI_Gather(local_a, local_n, MPI_INT, a, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // The root process now has the full array and can call the order_statistics function
    if (rank == 0) {
        int result = order_statistics(a, n, k);
        printf("The %d-th order statistic is %d\n", k, result);
        free(a);
    }

    free(local_a);
    double end = MPI_Wtime();
    MPI_Finalize();

    if (rank == 0) {
        printf("\nTime measured: %f", end - start);
    }
    return 0;
}


