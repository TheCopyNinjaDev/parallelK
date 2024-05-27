#include <stdio.h>
#include <stdlib.h>
#include <time.h> // for srand() and rand()

typedef struct {
    double x, y, z;
} Point;

void generatePointsToFile(const char *filename, int n) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }

    // Write number of points to the file
    fprintf(file, "%d\n", n);

    // Seed the random number generator
    srand(time(NULL));

    // Generate and write points to the file
    for (int i = 0; i < n; i++) {
        double x = (double)rand() / RAND_MAX * 10.0; // Random x coordinate between 0 and 10
        double y = (double)rand() / RAND_MAX * 10.0; // Random y coordinate between 0 and 10
        double z = (double)rand() / RAND_MAX * 10.0; // Random z coordinate between 0 and 10

        fprintf(file, "%.2lf %.2lf %.2lf\n", x, y, z);
    }

    fclose(file);
}

int main() {
    int n;

    printf("Enter number of points to generate: ");
    scanf("%d", &n);

    generatePointsToFile("points.txt", n);

    printf("Generated %d random points in points.txt\n", n);

    return 0;
}
