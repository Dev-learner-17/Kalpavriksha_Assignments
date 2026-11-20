#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "vfs.h"

static void clearInputBuffer()
{
    int character;

    while ((character = getchar()) != '\n' && character != EOF) {
    }
}

static int readInteger(const char *prompt, int *value)
{
    char input[INPUT_BUFFER_SIZE];
    size_t index = 0;
    unsigned int number = 0;
    unsigned int maximumInteger = ((unsigned int)-1) >> 1;
    bool hasDigit = false;

    if (prompt == NULL || value == NULL) {
        return -1;
    }

    printf("%s", prompt);
    fflush(stdout);

    if (fgets(input, sizeof(input), stdin) == NULL) {
        return 0;
    }

    if (strchr(input, '\n') == NULL && !feof(stdin)) {
        clearInputBuffer();
        return -1;
    }

    while (input[index] == ' ' || input[index] == '\t') {
        index++;
    }

    while (input[index] >= '0' && input[index] <= '9') {
        unsigned int digit = (unsigned int)(input[index] - '0');

        if (number > (maximumInteger - digit) / 10U) {
            return -1;
        }

        number = number * 10U + digit;
        hasDigit = true;
        index++;
    }

    while (input[index] == ' ' || input[index] == '\t' ||
           input[index] == '\n' || input[index] == '\r') {
        index++;
    }

    if (!hasDigit || input[index] != '\0') {
        return -1;
    }

    *value = (int)number;
    return 1;
}

int getValidInput(const char *prompt, int minimum, int maximum, int *value)
{
    int inputStatus;

    if (prompt == NULL || value == NULL || minimum > maximum) {
        return 0;
    }

    while (true) {
        inputStatus = readInteger(prompt, value);

        if (inputStatus == 0) {
            return 0;
        }

        if (inputStatus == 1 && *value >= minimum && *value <= maximum) {
            return 1;
        }

        printf("Invalid input. Enter a value between %d and %d.\n",
               minimum, maximum);
    }
}

char *readCommandLine()
{
    size_t capacity = BLOCK_SIZE;
    size_t length = 0;
    char *line = malloc(capacity);
    int character;

    if (line == NULL) {
        return NULL;
    }

    while ((character = getchar()) != EOF && character != '\n') {
        if (length + 1 >= capacity) {
            size_t newCapacity = capacity * 2;
            char *resizedLine = realloc(line, newCapacity);

            if (resizedLine == NULL) {
                free(line);
                return NULL;
            }

            line = resizedLine;
            capacity = newCapacity;
        }

        line[length++] = (char)character;
    }

    if (character == EOF && length == 0) {
        free(line);
        return NULL;
    }

    line[length] = '\0';
    return line;
}
static char *skipWhitespace(char *text)
{
    while (*text != '\0' && isspace((unsigned char)*text)) {
        text++;
    }

    return text;
}

char *extractToken(char **text)
{
    char *tokenStart;
    size_t tokenLength;

    *text = skipWhitespace(*text);

    if (**text == '\0') {
        return NULL;
    }

    tokenStart = *text;

    while (**text != '\0' && !isspace((unsigned char)**text)) {
        (*text)++;
    }

    tokenLength = (size_t)(*text - tokenStart);

    if (**text != '\0') {
        **text = '\0';
        (*text)++;
    }

    if (tokenLength == 0) {
        return NULL;
    }

    return tokenStart;
}

char *extractWriteContent(char *text)
{
    char *contentStart;
    char *contentEnd;
    char *remainingText;
    char quote;

    text = skipWhitespace(text);

    if (*text == '"' || *text == '\'') {
        quote = *text;
        contentStart = text + 1;
        contentEnd = strchr(contentStart, quote);

        if (contentEnd == NULL) {
            return NULL;
        }

        remainingText = skipWhitespace(contentEnd + 1);

        if (*remainingText != '\0') {
            return NULL;
        }

        *contentEnd = '\0';
        return contentStart;
    }

    return text;
}
