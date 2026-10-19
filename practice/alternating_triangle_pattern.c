#include <stdio.h>

int main() {
    int numberOfRows;
    int rowNumber;
    int columnNumber;

    printf("Enter the number of Rows or Depth of pattern : \n");
    if (scanf("%d", &numberOfRows) != 1) {
        printf("Invalid input\n");
        return 1;
    }

    if (numberOfRows < 1 || numberOfRows > 10) {
        printf("Invalid input\n");
        return 1;
    }

    for (rowNumber = 1; rowNumber <= numberOfRows; rowNumber++) {
        for (columnNumber = 1; columnNumber <= rowNumber; columnNumber++) {
            if (rowNumber % 2 == 1) {
                printf("*");
            } else {
                printf("#");
            }
        }
        printf("\n");
    }

    return 0;
}