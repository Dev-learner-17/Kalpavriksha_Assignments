#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "analyzer.h"

int readInteger(const char *prompt, int *value)
{
    char input[INPUT_BUFFER_SIZE];
    char *endPointer;
    long convertedValue;

    printf("%s", prompt);

    if (fgets(input, sizeof(input), stdin) == NULL)
        return ZERO_VALUE;

    convertedValue = strtol(input, &endPointer, 10);

    while (*endPointer == ' ' || *endPointer == '\t')
        endPointer++;

    if (endPointer == input ||
        (*endPointer != '\n' && *endPointer != '\0') ||
        convertedValue < INT_MIN || convertedValue > INT_MAX)
    {
        printf("Invalid integer input.\n");
        return ZERO_VALUE;
    }

    *value = (int)convertedValue;
    return ONE_VALUE;
}

int readNonNegativeInteger(const char *prompt, int *value)
{
    if (!readInteger(prompt, value))
        return ZERO_VALUE;

    if (*value < ZERO_VALUE)
    {
        printf("Value cannot be negative.\n");
        return ZERO_VALUE;
    }

    return ONE_VALUE;
}

int readFloat(const char *prompt, float *value)
{
    char input[INPUT_BUFFER_SIZE];
    char *endPointer;
    float convertedValue;

    printf("%s", prompt);

    if (fgets(input, sizeof(input), stdin) == NULL)
        return ZERO_VALUE;

    convertedValue = strtof(input, &endPointer);

    while (*endPointer == ' ' || *endPointer == '\t')
        endPointer++;

    if (endPointer == input ||
        (*endPointer != '\n' && *endPointer != '\0') ||
        !isfinite(convertedValue) || convertedValue < 0.0f)
    {
        printf("Invalid non-negative numeric input.\n");
        return ZERO_VALUE;
    }

    *value = convertedValue;
    return ONE_VALUE;
}

int readString(const char *prompt, char *value, size_t capacity)
{
    size_t length;
    int character;

    printf("%s", prompt);

    if (fgets(value, (int)capacity, stdin) == NULL)
        return ZERO_VALUE;

    length = strcspn(value, "\n");

    if (value[length] == '\n')
        value[length] = '\0';
    else
    {
        character = getchar();

        if (character != EOF)
        {
            while (character != '\n' && character != EOF)
                character = getchar();

            printf("Input is too long.\n");
            return ZERO_VALUE;
        }
    }

    if (value[ZERO_VALUE] == '\0')
    {
        printf("Input cannot be empty.\n");
        return ZERO_VALUE;
    }

    return ONE_VALUE;
}
