#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> 
#include <float.h>

typedef struct {
    double x, y, z;
} Point;

typedef struct {
    Point c;
    double r;
} Sphere;

double distance(Point a, Point b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

Point centroid(Point a, Point b, Point c) {
    Point p;
    p.x = (a.x + b.x + c.x) / 3.0;
    p.y = (a.y + b.y + c.y) / 3.0;
    p.z = (a.z + b.z + c.z) / 3.0;
    return p;
}

Sphere makeSphere2(Point a, Point b) {
    Sphere s;
    s.c.x = (a.x + b.x) / 2.0;
    s.c.y = (a.y + b.y) / 2.0;
    s.c.z = (a.z + b.z) / 2.0;
    s.r = distance(a, b) / 2.0;
    return s;
}

Sphere makeSphere3(Point a, Point b, Point c) {
    Point cen = centroid(a, b, c);
    Sphere s;
    s.c = cen;
    s.r = distance(cen, a);
    return s;
}

Sphere welzl(Point *P, Point *R, int n, int r) {
    if (n == 0 || r == 3) {
        if (r == 0) return (Sphere){{0, 0, 0}, 0};
        if (r == 1) return (Sphere){R[0], 0};
        if (r == 2) return makeSphere2(R[0], R[1]);
        if (r == 3) return makeSphere3(R[0], R[1], R[2]);
    }

    int idx = rand() % n;
    Point p = P[idx];

    P[idx] = P[n - 1];

    Sphere D = welzl(P, R, n - 1, r);

    if (distance(D.c, p) <= D.r) {
        P[n - 1] = p;
        return D;
    }

    R[r] = p;
    return welzl(P, R, n - 1, r + 1);
}

Sphere findMinBoundingSphere(Point *points, int n) {
    Point R[3];
    return welzl(points, R, n, 0);
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int world_size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int n;
    Point *points = NULL;

    if (rank == 0) {

        // RANDOM RANDOM RANDOM RANDOM RANDOM RANDOM RANDOM RANDOM RANDOM RANDOM RANDOM RANDOM RANDOM RANDOM RANDOM RANDOM

        // printf("Enter number of points: ");
        // scanf("%d", &n);
        // srand(time(NULL));
        // // Allocate memory for n points
        // points = (Point *)malloc(n * sizeof(Point));
        // if (points == NULL) {
        //     printf("Memory allocation failed.\n");
        //     return 1; // Exit program with error
        // }

        // // printf("Random points (x y z):\n");
        // for (int i = 0; i < n; i++) {
        //     points[i].x = (double)rand() / RAND_MAX * 10.0; // Random x coordinate between 0 and 10
        //     points[i].y = (double)rand() / RAND_MAX * 10.0; // Random y coordinate between 0 and 10
        //     points[i].z = (double)rand() / RAND_MAX * 10.0; // Random z coordinate between 0 and 10

        //     // printf("%.2lf %.2lf %.2lf\n", points[i].x, points[i].y, points[i].z);
        // }

        //FILE FILE FILE FILE FILE FILE FILE FILE FILE FILE FILE FILE FILE FILE FILE FILE FILE FILE FILE FILE FILE FILE FILE
        FILE *file = fopen("points.txt", "r");
        if(file == NULL)
        {
            printf("Error opening file. \n");
            return 1;
        }

        // Read number of points from the file
        fscanf(file, "%d", &n);
        
        points = (Point *)malloc(n * sizeof(Point));
        if(points == NULL)
        {
            printf("Memory allocation failed.\n");
            fclose(file);
            return 1;
        }

        for(int i = 0; i < n; i++)
        {
            fscanf(file, "%lf %lf %lf", &points[i].x, &points[i].y, &points[i].z);
        }

        fclose(file);

    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int local_n = n / world_size;
    Point *local_points = (Point *)malloc(local_n * sizeof(Point));
    MPI_Scatter(points, local_n * sizeof(Point), MPI_BYTE, local_points, local_n * sizeof(Point), MPI_BYTE, 0, MPI_COMM_WORLD);

    double start = MPI_Wtime();

    Sphere local_sphere = findMinBoundingSphere(local_points, local_n);

    double end = MPI_Wtime();

    Sphere global_sphere;
    MPI_Allreduce(&local_sphere, &global_sphere, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Center: (%lf, %lf, %lf)\n", global_sphere.c.x, global_sphere.c.y, global_sphere.c.z);
        printf("Radius: %lf\n", global_sphere.r);
        free(points);
    }

    free(local_points);

    MPI_Finalize();
    if (rank == 0) {
        printf("\nTime measured: %f\n", end - start);
    }
    return 0;
}
