#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define N 1000000  // number of random points
#define MAX_ITER 100  // maximum iterations for Mandelbrot set
#define BOUNDING_BOX_AREA 3.5 * 3.5  // adjust bounding box size as needed

double mandelbrot(double x, double y, int max_iter) {
    double zx = 0, zy = 0, cx = x, cy = y;
    int i;
    for (i = 0; i < max_iter; i++) {
        double zx_squared = zx * zx;
        double zy_squared = zy * zy;
        if (zx_squared + zy_squared > 4) {
            return 0;
        }
        zy = 2 * zx * zy + cy;
        zx = zx_squared - zy_squared + cx;
    }
    return 1;
}

int main(int argc, char **argv) {
    int rank, num_tasks;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);

    double start = MPI_Wtime();

    srand(time(NULL) + rank);  // seed random number generator

    double S_local = 0, I_x_local = 0, I_y_local = 0, I_polar_local = 0;

    for (int i = 0; i < N / num_tasks; i++) {
        double x = (double)rand() / RAND_MAX * 3.5 - 2;  // random x in [-2, 1]
        double y = (double)rand() / RAND_MAX * 3.5 - 1.5;  // random y in [-1.5, 1.5]
        double in_out = mandelbrot(x, y, MAX_ITER);
        if (in_out == 1) {
            S_local += 1;
            I_x_local += y * y;
            I_y_local += x * x;
            I_polar_local += x * x + y * y;
        }
    }

    double S, I_x, I_y, I_polar;
    MPI_Reduce(&S_local, &S, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&I_x_local, &I_x, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&I_y_local, &I_y, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&I_polar_local, &I_polar, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        S *= BOUNDING_BOX_AREA / N;
        I_x *= BOUNDING_BOX_AREA / N;
        I_y *= BOUNDING_BOX_AREA / N;
        I_polar *= BOUNDING_BOX_AREA / N;

        printf("Estimated area: %f\n", S);
        printf("Estimated moment of inertia about x-axis: %f\n", I_x);
        printf("Estimated moment of inertia about y-axis: %f\n", I_y);
        printf("Estimated polar moment of inertia: %f\n", I_polar);
    }

    double end = MPI_Wtime();

    MPI_Finalize();
    if (rank == 0) {
        printf("\nTime measured: %f\n", end - start);
    }
    return 0;
}
