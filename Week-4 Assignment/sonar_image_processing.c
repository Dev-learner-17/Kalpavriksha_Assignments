#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_MATRIX_SIZE 2
#define MAX_MATRIX_SIZE 10
#define MIN_INTENSITY 0
#define MAX_INTENSITY 255
#define NEIGHBOR_RADIUS 1
#define ROW_BUFFER_COUNT 2
#define HALF_DIVISOR 2

enum ProgramStatus {
    STATUS_SUCCESS,
    STATUS_FAILURE
};

typedef struct {
    int **cells;
    int *values;
    int size;
} SonarMatrix;

static int **cellAt(const SonarMatrix *matrix, int row, int column)
{
    return matrix->cells + row * matrix->size + column;
}

static void freeMatrix(SonarMatrix *matrix)
{
    free(matrix->cells);
    free(matrix->values);
    matrix->cells = NULL;
    matrix->values = NULL;
    matrix->size = 0;
}

static int allocateMatrix(SonarMatrix *matrix, int size)
{
    int totalCells = size * size;

    matrix->size = size;
    matrix->cells = malloc((size_t)totalCells * sizeof(*matrix->cells));
    matrix->values = malloc((size_t)totalCells * sizeof(*matrix->values));
    if (matrix->cells == NULL || matrix->values == NULL) {
        freeMatrix(matrix);
        return STATUS_FAILURE;
    }
    int **cellCursor = matrix->cells;
    int *valueCursor = matrix->values;
    int **cellEnd = matrix->cells + totalCells;
    while (cellCursor < cellEnd) {
        *cellCursor = valueCursor;
        cellCursor++;
        valueCursor++;
    }
    return STATUS_SUCCESS;
}

static int discardRestOfLine()
{
    int character = getchar();

    while (character != '\n' && character != EOF) {
        character = getchar();
    }
    return character;
}

static int readMatrixSize(int *size)
{
    while (1) {
        int value;

        printf("Enter matrix size (%d-%d): ", MIN_MATRIX_SIZE, MAX_MATRIX_SIZE);
        if (scanf("%d", &value) != 1) {
            if (discardRestOfLine() == EOF) {
                return STATUS_FAILURE;
            }
            printf("Invalid input. Please enter an integer.\n");
            continue;
        }
        discardRestOfLine();
        if (value >= MIN_MATRIX_SIZE && value <= MAX_MATRIX_SIZE) {
            *size = value;
            return STATUS_SUCCESS;
        }
        printf("Size out of range.\n");
    }
}

static void fillRandomIntensities(SonarMatrix *matrix)
{
    for (int row = 0; row < matrix->size; row++) {
        for (int column = 0; column < matrix->size; column++) {
            **cellAt(matrix, row, column) = MIN_INTENSITY + rand() % (MAX_INTENSITY - MIN_INTENSITY + 1);
        }
    }
}

static void printMatrix(const SonarMatrix *matrix)
{
    for (int row = 0; row < matrix->size; row++) {
        for (int column = 0; column < matrix->size; column++) {
            printf("%5d", **cellAt(matrix, row, column));
        }
        printf("\n");
    }
}

static void swapCells(int **first, int **second)
{
    int *temporary = *first;

    *first = *second;
    *second = temporary;
}

static void transposeMatrix(SonarMatrix *matrix)
{
    for (int row = 0; row < matrix->size; row++) {
        for (int column = row + 1; column < matrix->size; column++) {
            swapCells(cellAt(matrix, row, column), cellAt(matrix, column, row));
        }
    }
}

static void reverseRows(SonarMatrix *matrix)
{
    for (int row = 0; row < matrix->size; row++) {
        for (int column = 0; column < matrix->size / HALF_DIVISOR; column++) {
            int mirrorColumn = matrix->size - 1 - column;
            swapCells(cellAt(matrix, row, column), cellAt(matrix, row, mirrorColumn));
        }
    }
}

static void rotateClockwise(SonarMatrix *matrix)
{
    transposeMatrix(matrix);
    reverseRows(matrix);
}

static int originalValueAt(const SonarMatrix *matrix, int rowOffset, int neighborRow, int neighborColumn, const int *previousRowValues, const int *currentRowValues)
{
    if (rowOffset < 0) {
        return *(previousRowValues + neighborColumn);
    }
    if (rowOffset == 0) {
        return *(currentRowValues + neighborColumn);
    }
    return **cellAt(matrix, neighborRow, neighborColumn);
}

static int averageOfNeighbors(const SonarMatrix *matrix, int row, int column, const int *previousRowValues, const int *currentRowValues)
{
    int sum = 0;
    int count = 0;

    for (int rowOffset = -NEIGHBOR_RADIUS; rowOffset <= NEIGHBOR_RADIUS; rowOffset++) {
        int neighborRow = row + rowOffset;
        if (neighborRow < 0 || neighborRow >= matrix->size) {
            continue;
        }
        for (int columnOffset = -NEIGHBOR_RADIUS; columnOffset <= NEIGHBOR_RADIUS; columnOffset++) {
            int neighborColumn = column + columnOffset;
            if (neighborColumn < 0 || neighborColumn >= matrix->size) {
                continue;
            }
            sum += originalValueAt(matrix, rowOffset, neighborRow, neighborColumn, previousRowValues, currentRowValues);
            count++;
        }
    }
    return sum / count;
}

static int applySmoothingFilter(SonarMatrix *matrix)
{
    int *rowBuffers = malloc((size_t)(ROW_BUFFER_COUNT * matrix->size) * sizeof(*rowBuffers));

    if (rowBuffers == NULL) {
        return STATUS_FAILURE;
    }
    int *previousRowValues = rowBuffers;
    int *currentRowValues = rowBuffers + matrix->size;

    for (int row = 0; row < matrix->size; row++) {
        for (int column = 0; column < matrix->size; column++) {
            *(currentRowValues + column) = **cellAt(matrix, row, column);
        }
        for (int column = 0; column < matrix->size; column++) {
            **cellAt(matrix, row, column) = averageOfNeighbors(matrix, row, column, previousRowValues, currentRowValues);
        }
        int *swapHolder = previousRowValues;
        previousRowValues = currentRowValues;
        currentRowValues = swapHolder;
    }
    free(rowBuffers);
    return STATUS_SUCCESS;
}

int main()
{
    SonarMatrix matrix = { NULL, NULL, 0 };
    int size;

    srand((unsigned int)time(NULL));
    if (readMatrixSize(&size) != STATUS_SUCCESS) {
        printf("Input ended. Exiting program.\n");
        return STATUS_FAILURE;
    }
    if (allocateMatrix(&matrix, size) != STATUS_SUCCESS) {
        printf("Memory allocation failed. Exiting program.\n");
        return STATUS_FAILURE;
    }

    fillRandomIntensities(&matrix);
    printf("\nOriginal Randomly Generated Matrix:\n");
    printMatrix(&matrix);

    rotateClockwise(&matrix);
    printf("\nMatrix after 90 degree Clockwise Rotation:\n");
    printMatrix(&matrix);

    if (applySmoothingFilter(&matrix) != STATUS_SUCCESS) {
        printf("Memory allocation failed. Exiting program.\n");
        freeMatrix(&matrix);
        return STATUS_FAILURE;
    }
    printf("\nMatrix after Applying 3x3 Smoothing Filter:\n");
    printMatrix(&matrix);

    freeMatrix(&matrix);
    printf("\nProgram completed successfully. Memory released.\n");
    return STATUS_SUCCESS;
}
