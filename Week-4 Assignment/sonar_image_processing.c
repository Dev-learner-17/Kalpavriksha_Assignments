
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

#define MINIMUM_MATRIX_SIZE 2
#define MAXIMUM_MATRIX_SIZE 10
#define MINIMUM_PIXEL_VALUE 0
#define MAXIMUM_PIXEL_VALUE 255
#define WINDOW_RADIUS 1
#define PRINT_FIELD_WIDTH 4

int readMatrixSize(int *matrixSize)
{
    int inputResult;

    printf("Enter matrix size (%d-%d): ",
           MINIMUM_MATRIX_SIZE, MAXIMUM_MATRIX_SIZE);

    while (1)
    {
        inputResult = scanf("%d", matrixSize);

        if (inputResult == EOF)
        {
            return 0;
        }

        if (inputResult == 1 &&
            *matrixSize >= MINIMUM_MATRIX_SIZE &&
            *matrixSize <= MAXIMUM_MATRIX_SIZE)
        {
            int inputCharacter;

            while ((inputCharacter = getchar()) != '\n' &&
                   inputCharacter != EOF)
            {
                if (inputCharacter != ' ' &&
                    inputCharacter != '\t' &&
                    inputCharacter != '\r')
                {
                    fprintf(stderr, "Invalid input.\n");
                    return 0;
                }
            }

            return 1;
        }

        int inputCharacter;

        while ((inputCharacter = getchar()) != '\n' &&
               inputCharacter != EOF)
        {
        }

        if (inputCharacter == EOF)
        {
            return 0;
        }

        fprintf(stderr,
                "Invalid input. Enter an integer from %d to %d.\n",
                MINIMUM_MATRIX_SIZE, MAXIMUM_MATRIX_SIZE);

        printf("Enter matrix size (%d-%d): ",
               MINIMUM_MATRIX_SIZE, MAXIMUM_MATRIX_SIZE);
    }
}

void generateMatrix(int **matrix, int matrixSize)
{
    int totalCells = matrixSize * matrixSize;

    for (int cell = 0; cell < totalCells; cell++)
    {
        **(matrix + cell) =
            MINIMUM_PIXEL_VALUE +
            rand() % (MAXIMUM_PIXEL_VALUE - MINIMUM_PIXEL_VALUE + 1);
    }
}

void displayMatrix(int **matrix, int matrixSize)
{
    for (int row = 0; row < matrixSize; row++)
    {
        for (int column = 0; column < matrixSize; column++)
        {
            printf("%*d",
                   PRINT_FIELD_WIDTH,
                   **(matrix + row * matrixSize + column));
        }

        printf("\n");
    }
}

void swapPointers(int **firstPointer, int **secondPointer)
{
    int *temporaryPointer = *firstPointer;

    *firstPointer = *secondPointer;
    *secondPointer = temporaryPointer;
}

void rotateMatrixClockwise90(int **matrix, int matrixSize)
{
    /* Transpose by swapping pointers, not pixel values. */
    for (int row = 0; row < matrixSize; row++)
    {
        for (int column = row + 1;
             column < matrixSize;
             column++)
        {
            int **firstCell = matrix + row * matrixSize + column;
            int **secondCell = matrix + column * matrixSize + row;

            swapPointers(firstCell, secondCell);
        }
    }

    /* Reverse each row by swapping pointers. */
    for (int row = 0; row < matrixSize; row++)
    {
        for (int column = 0;
             column < matrixSize / 2;
             column++)
        {
            int **leftCell = matrix + row * matrixSize + column;

            int **rightCell =
                matrix + row * matrixSize +
                (matrixSize - 1 - column);

            swapPointers(leftCell, rightCell);
        }
    }
}

void applyMeanSmoothing(int **matrix, int matrixSize)
{
    int *previousRow = malloc(
        (size_t)matrixSize * sizeof(*previousRow));

    int *currentRow = malloc(
        (size_t)matrixSize * sizeof(*currentRow));

    if (previousRow == NULL || currentRow == NULL)
    {
        free(previousRow);
        free(currentRow);

        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    for (int row = 0; row < matrixSize; row++)
    {
        int **matrixRow = matrix + row * matrixSize;

        /* Preserve the original values of the current row. */
        for (int column = 0; column < matrixSize; column++)
        {
            *(currentRow + column) = **(matrixRow + column);
        }

        for (int column = 0; column < matrixSize; column++)
        {
            int sum = 0;
            int neighborCount = 0;

            for (int rowOffset = -WINDOW_RADIUS;
                 rowOffset <= WINDOW_RADIUS;
                 rowOffset++)
            {
                int neighborRow = row + rowOffset;

                if (neighborRow < 0 || neighborRow >= matrixSize)
                {
                    continue;
                }

                for (int columnOffset = -WINDOW_RADIUS;
                     columnOffset <= WINDOW_RADIUS;
                     columnOffset++)
                {
                    int neighborColumn = column + columnOffset;

                    if (neighborColumn < 0 ||
                        neighborColumn >= matrixSize)
                    {
                        continue;
                    }

                    int pixelValue;

                    if (neighborRow == row - 1)
                    {
                        pixelValue = *(previousRow + neighborColumn);
                    }
                    else if (neighborRow == row)
                    {
                        pixelValue = *(currentRow + neighborColumn);
                    }
                    else
                    {
                        pixelValue =
                            **(matrix +
                               neighborRow * matrixSize +
                               neighborColumn);
                    }

                    sum += pixelValue;
                    neighborCount++;
                }
            }

            /* Integer division gives the floor of the average. */
            **(matrixRow + column) = sum / neighborCount;
        }

        /* Retain the original row for the next iteration. */
        int *temporaryRow = previousRow;
        previousRow = currentRow;
        currentRow = temporaryRow;
    }

    free(previousRow);
    free(currentRow);
}

int main(void)
{
    int matrixSize;

    if (!readMatrixSize(&matrixSize))
    {
        fprintf(stderr, "Unable to read a valid matrix size.\n");
        return EXIT_FAILURE;
    }

    size_t totalCells =
        (size_t)matrixSize * (size_t)matrixSize;

    /* Contiguous storage for the actual pixel values. */
    int *pixelValues = malloc(totalCells * sizeof(*pixelValues));

    /* The matrix is represented by pointers to its pixel values. */
    int **matrix = malloc(totalCells * sizeof(*matrix));

    if (pixelValues == NULL || matrix == NULL)
    {
        free(pixelValues);
        free(matrix);

        fprintf(stderr, "Memory allocation failed.\n");
        return EXIT_FAILURE;
    }

    for (size_t cell = 0; cell < totalCells; cell++)
    {
        *(matrix + cell) = pixelValues + cell;
    }

    srand((unsigned int)time(NULL));

    generateMatrix(matrix, matrixSize);

    printf("\nOriginal Randomly Generated Matrix:\n");
    displayMatrix(matrix, matrixSize);

    rotateMatrixClockwise90(matrix, matrixSize);

    printf("\nMatrix After 90-Degree Clockwise Rotation:\n");
    displayMatrix(matrix, matrixSize);

    applyMeanSmoothing(matrix, matrixSize);

    printf("\nMatrix After Applying 3x3 Smoothing Filter:\n");
    displayMatrix(matrix, matrixSize);

    free(matrix);
    free(pixelValues);

    return EXIT_SUCCESS;
}
