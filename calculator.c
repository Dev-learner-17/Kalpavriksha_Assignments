#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#define MAX 1000
#define DIVISION_ERROR 1
#define INVALID_ERROR 2

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



int main(){
    char expression[MAX];
    int numbers[MAX];
    char operator[MAX];

    int numbersTop = -1;
    int operatorTop = -1;
    int i = 0;
    int error = 0;
    int numberPresent = 0;
    int expectNumber = 1;

    fgets(expression, MAX, stdin);

    while(expression[i] != '\0'){

        if(isspace((unsigned char) expression[i])){
            i++;
            continue;
        }

        if(isdigit((unsigned char)expression[i])){
            if(!expectNumber){
                error = INVALID_ERROR;
                break;
            }

            int num = 0;

            while(isdigit((unsigned char) expression[i])){
                num = num * 10 + (expression[i] - '0');
                i++;
            }

            numbers[++numbersTop] = num;
            numberPresent = 1;
            expectNumber = 0;
        }

        else if(expression[i] == '+' || expression[i] == '-' || expression[i] == '*' || expression[i] == '/'){
            
            if(expectNumber){
                error = INVALID_ERROR;
                break;
            }

            char currentOperator = expression[i];

            while (operatorTop >= 0 && precedence(operator[operatorTop]) >= precedence(currentOperator)){

                if(numbersTop < 1){
                    error = INVALID_ERROR;
                    break;
                }

                int b = numbers[numbersTop--];
                int a = numbers[numbersTop--];
                char op = operator[operatorTop--];

                int result = calculate(a, b, op, &error);

                if(error){
                    break;
                }

                numbers[++numbersTop] = result;

            }
            if(error){
                break;
            }

            operator[++operatorTop] = currentOperator;
            expectNumber = 1;
            i++;

        }

        else {
            error = INVALID_ERROR;
            break;
        }

    }

    if (!error) {
        if (!numberPresent || expectNumber){
            error = INVALID_ERROR;
        }
    }

    while (!error && operatorTop >= 0) {

        if (numbersTop < 1) {
            error = INVALID_ERROR;
            break;
        }

        int b = numbers[numbersTop--];
        int a = numbers[numbersTop--];
        char op = operator[operatorTop--];

        int result = calculate(a, b, op, &error);

        if (error)
            break;

        numbers[++numbersTop] = result;
    }

    if (error == DIVISION_ERROR){
        printf("Error: Division by zero.\n");
    }
    else if (error == INVALID_ERROR || numbersTop != 0){
        printf("Error: Invalid expression.\n");
    }
    else{
        printf("%d\n", numbers[numbersTop]);
    }

    return 0;
}