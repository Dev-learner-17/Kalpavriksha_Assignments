#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_STUDENTS 100
#define MIN_STUDENTS 1
#define SUBJECT_COUNT 3
#define MAX_NAME_LENGTH 50
#define INPUT_BUFFER_SIZE 150
#define MIN_MARKS 0.0f
#define MAX_MARKS 100.0f
#define GRADE_A_MIN 85.0f
#define GRADE_B_MIN 70.0f
#define GRADE_C_MIN 50.0f
#define GRADE_D_MIN 35.0f
#define NO_STARS 0
#define GRADE_A_STARS 5
#define GRADE_B_STARS 4
#define GRADE_C_STARS 3
#define GRADE_D_STARS 2

struct Student {
    int rollNumber;
    char name[MAX_NAME_LENGTH];
    float marks[SUBJECT_COUNT];
    float total;
    float average;
    char grade;
};

float calculateTotal(struct Student student) {
    float total = 0.0f;

    for (int subjectIndex = 0; subjectIndex < SUBJECT_COUNT; subjectIndex++)
        total += student.marks[subjectIndex];

    return total;
}

float calculateAverage(float total) {
    return total / SUBJECT_COUNT;
}

char calculateGrade(float average) {
    if (average < MIN_MARKS || average > MAX_MARKS)
        return 'F';
    else if (average >= GRADE_A_MIN)
        return 'A';
    else if (average >= GRADE_B_MIN)
        return 'B';
    else if (average >= GRADE_C_MIN)
        return 'C';
    else if (average >= GRADE_D_MIN)
        return 'D';
    else
        return 'F';
}

void printPerformance(char grade) {
    int performanceStars = NO_STARS;

    if (grade == 'A')
        performanceStars = GRADE_A_STARS;
    else if (grade == 'B')
        performanceStars = GRADE_B_STARS;
    else if (grade == 'C')
        performanceStars = GRADE_C_STARS;
    else if (grade == 'D')
        performanceStars = GRADE_D_STARS;

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
    char line[INPUT_BUFFER_SIZE];
    char *lastToken;
    char *nameStart;
    int rollNumber;

    printf("\nEnter details for student %d:\n", studentIndex + 1);
    printf("Enter Roll Number, Name, and Marks in %d subjects: ", SUBJECT_COUNT);

    if (fgets(line, sizeof(line), stdin) == NULL) {
        printf("Error: Unable to read student details.\n");
        return 0;
    }

    line[strcspn(line, "\n")] = '\0';

    for (int subjectIndex = SUBJECT_COUNT - 1; subjectIndex >= 0; subjectIndex--) {
        lastToken = strrchr(line, ' ');

        if (lastToken == NULL) {
            printf("Error: Invalid input format. Please enter roll number, name, and %d marks.\n", SUBJECT_COUNT);
            return 0;
        }

        students[studentIndex].marks[subjectIndex] = (float)atof(lastToken + 1);
        *lastToken = '\0';
    }

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
        if (students[studentIndex].marks[subjectIndex] < MIN_MARKS || students[studentIndex].marks[subjectIndex] > MAX_MARKS) {
            printf("Error: Marks must be between %.0f and %.0f.\n", MIN_MARKS, MAX_MARKS);
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

        if (students[studentIndex].average < GRADE_D_MIN)
            continue;

        printf("Performance: ");
        printPerformance(students[studentIndex].grade);
        printf("\n");
    }
}

int main(void) {
    struct Student students[MAX_STUDENTS];
    int studentCount;

    printf("Enter number of students: ");

    if (scanf("%d", &studentCount) != 1) {
        printf("Error: Please enter a valid number of students.\n");
        return EXIT_FAILURE;
    }

    getchar();

    if (studentCount < MIN_STUDENTS || studentCount > MAX_STUDENTS) {
        printf("Invalid number of students. Enter a value between %d and %d.\n", MIN_STUDENTS, MAX_STUDENTS);
        return EXIT_FAILURE;
    }

    for (int studentIndex = 0; studentIndex < studentCount; studentIndex++) {
        while (!inputStudent(students, studentIndex))
            printf("Please enter the student details again.\n");
    }

    displayPerformance(students, studentCount);

    printf("\nList of Roll Numbers (via recursion): ");
    printRollNumbers(students, 0, studentCount);
    printf("\n");

    return EXIT_SUCCESS;
}
