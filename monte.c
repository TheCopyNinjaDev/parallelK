#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define MASTER_RANK 0
#define NUM_POINTS_TOTAL 1000000 // Total number of random points to generate
#define MAX_ITERATIONS 1000       // Maximum iterations to determine if a point is in the set

// Function to check if a point (c_real, c_imag) is in the Mandelbrot set
int inMandelbrotSet(double c_real, double c_imag) {
    double z_real = 0.0, z_imag = 0.0;
    double temp;

    for (int i = 0; i < MAX_ITERATIONS; i++) {
        if (z_real * z_real + z_imag * z_imag > 4.0) {
            return 0; // Point escaped to infinity
        }
        // Update z = z^2 + c
        temp = z_real * z_real - z_imag * z_imag + c_real;
        z_imag = 2 * z_real * z_imag + c_imag;
        z_real = temp;
    }

    return 1; // Point likely in the Mandelbrot set
}

int main(int argc, char *argv[]) {
    int rank, num_procs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    double start = MPI_Wtime();

    srand(rank); // Seed random number generator with rank to ensure unique sequences

    // Calculate number of points per process
    int num_points_per_proc = NUM_POINTS_TOTAL / num_procs;
    int points_in_mandelbrot_local = 0;

    // Each process generates and checks num_points_per_proc random points
    for (int i = 0; i < num_points_per_proc; i++) {
        double c_real = 4.0 * ((double)rand() / RAND_MAX) - 2.0;
        double c_imag = 4.0 * ((double)rand() / RAND_MAX) - 2.0;

        points_in_mandelbrot_local += inMandelbrotSet(c_real, c_imag);
    }

    // Reduce the local counts to calculate the total points inside the Mandelbrot set
    int total_points_in_mandelbrot;
    MPI_Reduce(&points_in_mandelbrot_local, &total_points_in_mandelbrot, 1, MPI_INT, MPI_SUM, MASTER_RANK, MPI_COMM_WORLD);

    if (rank == MASTER_RANK) {
        // Calculate area approximation
        double total_area = 16.0 * ((double)total_points_in_mandelbrot / NUM_POINTS_TOTAL);

        // Calculate moment of inertia approximation (using distance squared from the origin)
        double total_moment_of_inertia = 0.0;
        for (int i = 0; i < NUM_POINTS_TOTAL; i++) {
            double c_real = 4.0 * ((double)rand() / RAND_MAX) - 2.0;
            double c_imag = 4.0 * ((double)rand() / RAND_MAX) - 2.0;
            if (inMandelbrotSet(c_real, c_imag)) {
                double distance_squared = c_real * c_real + c_imag * c_imag;
                total_moment_of_inertia += distance_squared;
            }
        }
        total_moment_of_inertia *= 16.0 / NUM_POINTS_TOTAL;

        printf("Approximate area of the Mandelbrot set: %f\n", total_area);
        printf("Approximate moment of inertia relative to the origin: %f\n", total_moment_of_inertia);
    }

    double end = MPI_Wtime();
    MPI_Finalize();
    if (rank == 0) {
        printf("\nTime measured: %f", end - start);
    }
    return 0;
}
