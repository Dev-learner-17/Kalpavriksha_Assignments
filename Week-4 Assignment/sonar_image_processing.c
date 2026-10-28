#include <stdio.h>
#include <stdlib.h>

enum MatrixLimits {
    MIN_MATRIX_SIZE = 2,
    MAX_MATRIX_SIZE = 10,
    MIN_INTENSITY = 0,
    MAX_INTENSITY = 255
};

enum SmoothingWindow {
    NEIGHBOR_RADIUS = 1,
    ROW_BUFFER_COUNT = 2
};

enum ProgramStatus {
    STATUS_SUCCESS = 0,
    STATUS_FAILURE = 1
};

enum InputResult {
    INPUT_ENDED = 0
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
    int **cellCursor;
    int *valueCursor;
    int **cellEnd;

    matrix->size = size;
    matrix->cells = malloc((size_t)totalCells * sizeof(*matrix->cells));
    matrix->values = malloc((size_t)totalCells * sizeof(*matrix->values));
    if (matrix->cells == NULL || matrix->values == NULL) {
        freeMatrix(matrix);
        return STATUS_FAILURE;
    }
    cellCursor = matrix->cells;
    valueCursor = matrix->values;
    cellEnd = matrix->cells + totalCells;
    while (cellCursor < cellEnd) {
        *valueCursor = MIN_INTENSITY;
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

static int readMatrixSize()
{
    int size;

    while (1) {
        printf("Enter matrix size (%d-%d): ", MIN_MATRIX_SIZE, MAX_MATRIX_SIZE);
        if (scanf("%d", &size) != 1) {
            if (discardRestOfLine() == EOF) {
                return INPUT_ENDED;
            }
            printf("Invalid input. Please enter an integer.\n");
            continue;
        }
        discardRestOfLine();
        if (size >= MIN_MATRIX_SIZE && size <= MAX_MATRIX_SIZE) {
            return size;
        }
        printf("Size out of range.\n");
    }
}

static int readIntensity(int row, int column, int *intensity)
{
    int value;

    while (1) {
        printf("Value for row %d, column %d: ", row + 1, column + 1);
        if (scanf("%d", &value) != 1) {
            if (discardRestOfLine() == EOF) {
                return STATUS_FAILURE;
            }
            printf("Invalid input. Please enter an integer.\n");
            continue;
        }
        discardRestOfLine();
        if (value < MIN_INTENSITY || value > MAX_INTENSITY) {
            printf("Value out of range.\n");
            continue;
        }
        *intensity = value;
        return STATUS_SUCCESS;
    }
}

static int readMatrixValues(SonarMatrix *matrix)
{
    int row;
    int column;
    int intensity;

    printf("Enter %d sonar intensity values (%d-%d), row by row:\n", matrix->size * matrix->size, MIN_INTENSITY, MAX_INTENSITY);
    for (row = 0; row < matrix->size; row++) {
        for (column = 0; column < matrix->size; column++) {
            if (readIntensity(row, column, &intensity) != STATUS_SUCCESS) {
                return STATUS_FAILURE;
            }
            **cellAt(matrix, row, column) = intensity;
        }
    }
    return STATUS_SUCCESS;
}

static void printMatrix(const SonarMatrix *matrix)
{
    int row;
    int column;

    for (row = 0; row < matrix->size; row++) {
        for (column = 0; column < matrix->size; column++) {
            printf("%5d", **cellAt(matrix, row, column));
        }
        printf("\n");
    }
}

static void rotateClockwise(SonarMatrix *matrix)
{
    int layer;
    int position;
    int firstIndex;
    int lastIndex;
    int offset;
    int *savedTopPointer;

    for (layer = 0; layer < matrix->size / 2; layer++) {
        firstIndex = layer;
        lastIndex = matrix->size - 1 - layer;
        for (position = firstIndex; position < lastIndex; position++) {
            offset = position - firstIndex;
            savedTopPointer = *cellAt(matrix, firstIndex, position);
            *cellAt(matrix, firstIndex, position) = *cellAt(matrix, lastIndex - offset, firstIndex);
            *cellAt(matrix, lastIndex - offset, firstIndex) = *cellAt(matrix, lastIndex, lastIndex - offset);
            *cellAt(matrix, lastIndex, lastIndex - offset) = *cellAt(matrix, position, lastIndex);
            *cellAt(matrix, position, lastIndex) = savedTopPointer;
        }
    }
}

static int applySmoothingFilter(SonarMatrix *matrix)
{
    int *rowBuffers = malloc((size_t)(ROW_BUFFER_COUNT * matrix->size) * sizeof(*rowBuffers));
    int *previousRowValues;
    int *currentRowValues;
    int *swapHolder;
    int row;
    int column;
    int rowOffset;
    int columnOffset;
    int neighborRow;
    int neighborColumn;
    int neighborValue;
    int sum;
    int count;

    if (rowBuffers == NULL) {
        return STATUS_FAILURE;
    }
    previousRowValues = rowBuffers;
    currentRowValues = rowBuffers + matrix->size;

    for (row = 0; row < matrix->size; row++) {
        for (column = 0; column < matrix->size; column++) {
            *(currentRowValues + column) = **cellAt(matrix, row, column);
        }
        for (column = 0; column < matrix->size; column++) {
            sum = 0;
            count = 0;
            for (rowOffset = -NEIGHBOR_RADIUS; rowOffset <= NEIGHBOR_RADIUS; rowOffset++) {
                neighborRow = row + rowOffset;
                if (neighborRow < 0 || neighborRow >= matrix->size) {
                    continue;
                }
                for (columnOffset = -NEIGHBOR_RADIUS; columnOffset <= NEIGHBOR_RADIUS; columnOffset++) {
                    neighborColumn = column + columnOffset;
                    if (neighborColumn < 0 || neighborColumn >= matrix->size) {
                        continue;
                    }
                    if (rowOffset < 0) {
                        neighborValue = *(previousRowValues + neighborColumn);
                    } else if (rowOffset == 0) {
                        neighborValue = *(currentRowValues + neighborColumn);
                    } else {
                        neighborValue = **cellAt(matrix, neighborRow, neighborColumn);
                    }
                    sum += neighborValue;
                    count++;
                }
            }
            **cellAt(matrix, row, column) = sum / count;
        }
        swapHolder = previousRowValues;
        previousRowValues = currentRowValues;
        currentRowValues = swapHolder;
    }

    free(rowBuffers);
    return STATUS_SUCCESS;
}

int main()
{
    SonarMatrix matrix = { NULL, NULL, 0 };
    int size = readMatrixSize();

    if (size == INPUT_ENDED) {
        return STATUS_FAILURE;
    }
    if (allocateMatrix(&matrix, size) != STATUS_SUCCESS) {
        printf("Memory allocation failed.\n");
        return STATUS_FAILURE;
    }
    if (readMatrixValues(&matrix) != STATUS_SUCCESS) {
        freeMatrix(&matrix);
        return STATUS_FAILURE;
    }

    printf("\nOriginal Matrix:\n");
    printMatrix(&matrix);

    rotateClockwise(&matrix);
    printf("\nMatrix after 90 degree Clockwise Rotation:\n");
    printMatrix(&matrix);

    if (applySmoothingFilter(&matrix) != STATUS_SUCCESS) {
        printf("Memory allocation failed.\n");
        freeMatrix(&matrix);
        return STATUS_FAILURE;
    }
    printf("\nMatrix after Applying 3x3 Smoothing Filter:\n");
    printMatrix(&matrix);

    freeMatrix(&matrix);
    return STATUS_SUCCESS;
}
