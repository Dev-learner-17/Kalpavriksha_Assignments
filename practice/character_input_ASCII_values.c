#include <stdio.h>
#include <string.h>

int validateCount(int numberOfCharacters) {
    return numberOfCharacters >= 1 && numberOfCharacters <= 20;
}

int validateCharacter(char currentCharacter) {
    return currentCharacter >= 'a' && currentCharacter <= 'y';
}

int main() {
    int numberOfCharacters, index, asciiSum = 0;
    char inputCharacters[100];
    int asciiValue;

    printf("Enter the number of characters:\n");
    
    if (scanf("%d", &numberOfCharacters) != 1 ||
        !validateCount(numberOfCharacters)) {
        printf("Invalid input\n");
        return 1;
    }

    if (scanf(" %99[^\n]", inputCharacters) != 1) {
        printf("Invalid input\n");
        return 1;
    }

    int characterCount = 0;

    for (index = 0; inputCharacters[index] != '\0'; index++) {
        if (inputCharacters[index] != ' ' &&
            inputCharacters[index] != '\t') {
            if (!validateCharacter(inputCharacters[index])) {
                printf("Invalid character\n");
                return 1;
            }
            characterCount++;
        }
    }

    if (characterCount > numberOfCharacters) {
        printf("Error: Exceeded the number of characters entered\n");
        return 1;
    }

    if (characterCount < numberOfCharacters) {
        printf("Error: Fewer characters entered than expected\n");
        return 1;
    }

    for (index = 0; inputCharacters[index] != '\0'; index++) {
        if (inputCharacters[index] == ' ' || inputCharacters[index] == '\t') {
            continue;
        }

        asciiValue = (int)inputCharacters[index];
        asciiSum += asciiValue;

        printf("%c %d %c\n", inputCharacters[index], asciiValue, inputCharacters[index] + 1);
    }

    printf("Sum = %d\n", asciiSum);

    return 0;
}