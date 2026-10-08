#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_STUDENTS 100
#define SUBJECT_COUNT 3

struct Student {
    int rollNumber;
    char name[50];
    float marks[SUBJECT_COUNT];
    float total;
    float average;
    char grade;
};

float calculateTotal(struct Student student) {
    return student.marks[0] + student.marks[1] + student.marks[2];
}

float calculateAverage(float total) {
    return total / SUBJECT_COUNT;
}

char calculateGrade(float average) {
    if (average < 0 || average > 100)
        return 'F';
    else if (average >= 85)
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
    int performanceStars = 0;

    if (grade == 'A')
        performanceStars = 5;
    else if (grade == 'B')
        performanceStars = 4;
    else if (grade == 'C')
        performanceStars = 3;
    else if (grade == 'D')
        performanceStars = 2;

    for (int starIndex = 0; starIndex < performanceStars; starIndex++)
        printf("*");
}

void printRollNumbers(struct Student students[], int index, int studentCount) {
    if (index >= studentCount)
        return;

    printf("%d ", students[index].rollNumber);
    printRollNumbers(students, index + 1, studentCount);
}

int inputStudent(struct Student students[], int studentIndex) {
    char line[150];
    char *lastToken;
    char *nameStart;
    int rollNumber;

    printf("\nEnter details for student %d:\n", studentIndex + 1);
    printf("Enter Roll Number, Name, and Marks in 3 subjects: ");

    if (fgets(line, sizeof(line), stdin) == NULL) {
        printf("Error: Unable to read student details.\n");
        return 0;
    }

    line[strcspn(line, "\n")] = '\0';

    lastToken = strrchr(line, ' ');

    if (lastToken == NULL) {
        printf("Error: Invalid input format. Please enter roll number, name, and 3 marks.\n");
        return 0;
    }

    students[studentIndex].marks[2] = (float)atof(lastToken + 1);
    *lastToken = '\0';

    lastToken = strrchr(line, ' ');

    if (lastToken == NULL) {
        printf("Error: Invalid input format. Please enter roll number, name, and 3 marks.\n");
        return 0;
    }

    students[studentIndex].marks[1] = (float)atof(lastToken + 1);
    *lastToken = '\0';

    lastToken = strrchr(line, ' ');

    if (lastToken == NULL) {
        printf("Error: Invalid input format. Please enter roll number, name, and 3 marks.\n");
        return 0;
    }

    students[studentIndex].marks[0] = (float)atof(lastToken + 1);
    *lastToken = '\0';

    nameStart = strchr(line, ' ');

    if (nameStart == NULL) {
        printf("Error: Invalid input format. Please provide a name.\n");
        return 0;
    }

    *nameStart = '\0';
    rollNumber = atoi(line);

    for (int previousStudentIndex = 0; previousStudentIndex < studentIndex; previousStudentIndex++) {
        if (students[previousStudentIndex].rollNumber == rollNumber) {
            printf("Error: Roll number %d already exists. Please enter a different roll number.\n", rollNumber);
            return 0;
        }
    }

    students[studentIndex].rollNumber = rollNumber;

    nameStart++;

    while (*nameStart == ' ')
        nameStart++;

    if (*nameStart == '\0') {
        printf("Error: Name cannot be empty.\n");
        return 0;
    }

    strncpy(students[studentIndex].name, nameStart, sizeof(students[studentIndex].name) - 1);
    students[studentIndex].name[sizeof(students[studentIndex].name) - 1] = '\0';

    for (int subjectIndex = 0; subjectIndex < SUBJECT_COUNT; subjectIndex++) {
        if (students[studentIndex].marks[subjectIndex] < 0 || students[studentIndex].marks[subjectIndex] > 100) {
            printf("Error: Marks must be between 0 and 100.\n");
            return 0;
        }
    }

    students[studentIndex].total = calculateTotal(students[studentIndex]);
    students[studentIndex].average = calculateAverage(students[studentIndex].total);
    students[studentIndex].grade = calculateGrade(students[studentIndex].average);

    return 1;
}

void displayPerformance(struct Student students[], int studentCount) {
    printf("\n===== Student Performance =====\n");

    for (int studentIndex = 0; studentIndex < studentCount; studentIndex++) {
        printf("\nRoll: %d\n", students[studentIndex].rollNumber);
        printf("Name: %s\n", students[studentIndex].name);
        printf("Total: %.2f\n", students[studentIndex].total);
        printf("Average: %.2f\n", students[studentIndex].average);
        printf("Grade: %c\n", students[studentIndex].grade);

        if (students[studentIndex].average < 35)
            continue;

        printf("Performance: ");
        printPerformance(students[studentIndex].grade);
        printf("\n");
    }
}

int main() {
    struct Student students[MAX_STUDENTS];
    int studentCount;

    printf("Enter number of students: ");

    if (scanf("%d", &studentCount) != 1) {
        printf("Error: Please enter a valid number of students.\n");
        return 1;
    }

    getchar();

    if (studentCount < 1 || studentCount > MAX_STUDENTS) {
        printf("Invalid number of students. Enter a value between 1 and %d.\n", MAX_STUDENTS);
        return 1;
    }

    for (int studentIndex = 0; studentIndex < studentCount; studentIndex++) {
        while (!inputStudent(students, studentIndex)) {
            printf("Please enter the student details again.\n");
        }
    }

    displayPerformance(students, studentCount);

    printf("\nList of Roll Numbers (via recursion): ");
    printRollNumbers(students, 0, studentCount);
    printf("\n");

    return 0;
}