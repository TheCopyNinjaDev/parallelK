#include <stdio.h>
#include <stdlib.h>
#include "stdbool.h"
#include "math.h"
#include <mpi.h>


bool isPrime(int n) {
    // Check for special cases
    if (n <= 1) {
        return false;  // Numbers <= 1 are not prime
    }
    
    // Check for divisors from 2 to sqrt(n)
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            return false;  // Found a divisor other than 1 and itself
        }
    }
    
    return true;  // If no divisors other than 1 and itself, it's prime
}

int main(int argc, char *argv[]) {

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int L1 = 1;
    double L2 = pow(10, 8);

    double chunk = ceil((double)((L2-L1) / size));
    double Nmin_loc = L1 + chunk * rank;
    double Nmax_loc = L1 - 1 + chunk*(rank+1);

    int quantity_loc = 0;

    double start = MPI_Wtime();

    for(int i=Nmin_loc; i<=Nmax_loc; i++)
    {
        if (isPrime(i)){
            quantity_loc++;
        }
    }

    double end = MPI_Wtime();

    // Gather all local sums to root (rank 0)
    int *local_sums = NULL;
    
    if (rank == 0) {
        local_sums = (int *)malloc(size * sizeof(int));
    }
    
    MPI_Gather(&quantity_loc, 1, MPI_INT, local_sums, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Compute global sum on root
    if (rank == 0) {
        int global_sum = 0;
        for (int i = 0; i < size; i++) {
            global_sum += local_sums[i];
        }
        printf("Quantity of prime nums: %d\n", global_sum);
        free(local_sums);  // Free memory used for local_sums
    }

    MPI_Finalize();
    if (rank == 0) {
        printf("\nTime measured: %f\n", end - start);
    }
    return 0;
}
