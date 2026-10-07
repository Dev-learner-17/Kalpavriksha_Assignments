#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_STUDENTS 100

struct Student {
    int rollNumber;
    char name[50];
    float marks[3];
    int total;
    float average;
    char grade;
};

int calculateTotal(struct Student student) {
    return (int)(student.marks[0] + student.marks[1] + student.marks[2]);
}

float calculateAverage(int total) {
    return total / 3.0f;
}

char calculateGrade(float average) {
    if (average >= 85)
        return 'A';
    else if (average >= 70)
        return 'B';
    else if (average >= 50)
        return 'C';
    else if (average >= 35)
        return 'D';
    else
        return 'F';
}

void printPerformance(char grade) {
    int stars = 0;

    if (grade == 'A')
        stars = 5;
    else if (grade == 'B')
        stars = 4;
    else if (grade == 'C')
        stars = 3;
    else if (grade == 'D')
        stars = 2;

    for (int i = 0; i < stars; i++)
        printf("*");
}

void printRollNumbers(struct Student students[], int index, int n) {
    if (index >= n)
        return;

    printf("%d ", students[index].rollNumber);
    printRollNumbers(students, index + 1, n);
}

int main() {
    struct Student students[MAX_STUDENTS];
    int n;

    printf("Enter number of students: ");
    scanf("%d", &n);
    getchar();

    if (n < 1 || n > MAX_STUDENTS) {
        printf("Invalid number of students.\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        char line[150];
        char *lastToken;
        char *nameStart;
        int rollNumber;
        int duplicate;

        printf("\nEnter details for student %d:\n", i + 1);
        printf("Enter Roll Number, Name, and Marks in 3 subjects: ");

        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n")] = '\0';

        lastToken = strrchr(line, ' ');

        if (lastToken == NULL)
            return 1;

        students[i].marks[2] = (float)atof(lastToken + 1);
        *lastToken = '\0';

        lastToken = strrchr(line, ' ');

        if (lastToken == NULL)
            return 1;

        students[i].marks[1] = (float)atof(lastToken + 1);
        *lastToken = '\0';

        lastToken = strrchr(line, ' ');

        if (lastToken == NULL)
            return 1;

        students[i].marks[0] = (float)atof(lastToken + 1);
        *lastToken = '\0';

        nameStart = strchr(line, ' ');

        if (nameStart != NULL) {
            *nameStart = '\0';
            rollNumber = atoi(line);
            duplicate = 0;

            for (int j = 0; j < i; j++) {
                if (students[j].rollNumber == rollNumber) {
                    duplicate = 1;
                    break;
                }
            }

            if (duplicate) {
                printf("Error: Roll number %d already exists.\n", rollNumber);
                i--;
                continue;
            }

            students[i].rollNumber = rollNumber;
            nameStart++;

            while (*nameStart == ' ')
                nameStart++;

            strncpy(students[i].name, nameStart, sizeof(students[i].name) - 1);
            students[i].name[sizeof(students[i].name) - 1] = '\0';
        }

        students[i].total = calculateTotal(students[i]);
        students[i].average = calculateAverage(students[i].total);
        students[i].grade = calculateGrade(students[i].average);
    }

    printf("\n===== Student Performance =====\n");

    for (int i = 0; i < n; i++) {
        printf("\nRoll: %d\n", students[i].rollNumber);
        printf("Name: %s\n", students[i].name);
        printf("Total: %d\n", students[i].total);
        printf("Average: %.2f\n", students[i].average);
        printf("Grade: %c\n", students[i].grade);

        if (students[i].average < 35)
            continue;

        printf("Performance: ");
        printPerformance(students[i].grade);
        printf("\n");
    }

    printf("\nList of Roll Numbers (via recursion): ");
    printRollNumbers(students, 0, n);
    printf("\n");

    return 0;
}