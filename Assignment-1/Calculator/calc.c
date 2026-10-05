#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define EXPR_MAX_LENGTH 1000

enum ErrorCode {
    NO_ERROR,
    DIVISION_ERROR,
    INVALID_ERROR
};

int precedence(char op) {
    if (op == '+' || op == '-')
        return 1;

    if (op == '*' || op == '/')
        return 2;

    return 0;
}

int calculate(int a, int b, char op, int *error) {
    if (op == '+')
        return a + b;

    if (op == '-')
        return a - b;

    if (op == '*')
        return a * b;

    if (op == '/') {
        if (b == 0) {
            *error = DIVISION_ERROR;
            return 0;
        }

        return a / b;
    }

    *error = INVALID_ERROR;
    return 0;
}

int reduceTop(int operands[], int *operandsTop, char operators[], int *operatorsTop, int *error) {

    if (*operandsTop < 1 || *operatorsTop < 0) {
        *error = INVALID_ERROR;
        return 0;
    }

    int b = operands[(*operandsTop)--];
    int a = operands[(*operandsTop)--];
    char op = operators[(*operatorsTop)--];

    int result = calculate(a, b, op, error);

    if (*error)
        return 0;

    operands[++(*operandsTop)] = result;

    return 1;
}

int evaluateExpression(const char *expression, int *result) {
    int operands[EXPR_MAX_LENGTH];
    char operators[EXPR_MAX_LENGTH];

    int operandsTop = -1;
    int operatorsTop = -1;
    int i = 0;
    int error = NO_ERROR;
    int operandPresent = 0;
    int expectOperand = 1;

    while (expression[i] != '\0') {

        if (isspace((unsigned char)expression[i])) {
            i++;
            continue;
        }

        if (isdigit((unsigned char)expression[i])) {

            if (!expectOperand) {
                error = INVALID_ERROR;
                break;
            }

            int num = 0;

            while (isdigit((unsigned char)expression[i])) {
                num = num * 10 + (expression[i] - '0');
                i++;
            }

            operands[++operandsTop] = num;
            operandPresent = 1;
            expectOperand = 0;
        }

        else if (expression[i] == '+' || expression[i] == '-' || expression[i] == '*' || expression[i] == '/') {
            if (expectOperand) {
                error = INVALID_ERROR;
                break;
            }

            char currentOperator = expression[i];

            while (operatorsTop >= 0 && precedence(operators[operatorsTop]) >= precedence(currentOperator)) {
                if (!reduceTop(operands, &operandsTop, operators, &operatorsTop, &error)) {
                    break;
                }
            }

            if (error){
                break;
            }

            operators[++operatorsTop] = currentOperator;
            expectOperand = 1;
            i++;
        }

        else {
            error = INVALID_ERROR;
            break;
        }
    }

    if (!error && (!operandPresent || expectOperand)){
        error = INVALID_ERROR;
    }

    while (!error && operatorsTop >= 0) {

        if (!reduceTop(operands, &operandsTop, operators, &operatorsTop, &error)) {
            break;
        }
    }

    if (!error && operandsTop == 0)
        *result = operands[operandsTop];

    return error;
}

int main() {
    char expression[EXPR_MAX_LENGTH];
    int result;

    if (fgets(expression, EXPR_MAX_LENGTH, stdin) == NULL) {
        printf("Error: Unable to read expression.\n");
        return 1;
    }

    expression[strcspn(expression, "\n")] = '\0';

    int error = evaluateExpression(expression, &result);

    if (error == DIVISION_ERROR) {
        printf("Error: Division by zero.\n");
    }
    else if (error == INVALID_ERROR) {
        printf("Error: Invalid expression.\n");
    }
    else {
        printf("%d\n", result);
    }

    return 0;
}