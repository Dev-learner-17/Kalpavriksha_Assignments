#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TEXT_SIZE 50
#define LINE_SIZE 100
#define START_CAPACITY 4
#define CEO_DESIGNATION "CEO"
#define NO_MANAGER 0

typedef enum {
    OK,
    ERR_INPUT,
    ERR_NOT_FOUND,
    ERR_DUPLICATE,
    ERR_NO_MANAGER,
    ERR_CEO,
    ERR_MEMORY
} Status;

typedef struct Employee {
    int id;
    char name[TEXT_SIZE];
    char designation[TEXT_SIZE];
    struct Employee *manager;
    struct Employee **reports;
    int reportCount;
    int reportCapacity;
} Employee;

Employee **allEmployees = NULL;
int totalEmployees = 0;
int totalCapacity = 0;

void clearInputBuffer(void) {
    int ch;

    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

int readLine(const char *prompt, char *buffer, int size) {
    printf("%s", prompt);

    if (fgets(buffer, size, stdin) == NULL) {
        return 0;
    }

    int length = (int)strlen(buffer);

    if (buffer[length - 1] != '\n' && !feof(stdin)) {
        clearInputBuffer();
        return 0;
    }

    while (length > 0 && isspace((unsigned char)buffer[length - 1])) {
        buffer[--length] = '\0';
    }

    int start = 0;
    while (isspace((unsigned char)buffer[start])) {
        start++;
    }
    memmove(buffer, buffer + start, strlen(buffer + start) + 1);

    return buffer[0] != '\0';
}

int readInt(const char *prompt, int *value) {
    char line[LINE_SIZE];
    char *end;

    if (!readLine(prompt, line, sizeof(line))) {
        return 0;
    }

    long number = strtol(line, &end, 10);

    if (*end != '\0' || number < 0 || number > 1000000000L) {
        return 0;
    }

    *value = (int)number;
    return 1;
}

Employee **growArray(Employee **array, int *capacity, int needed) {
    if (needed <= *capacity) {
        return array;
    }

    int newCapacity = (*capacity == 0) ? START_CAPACITY : *capacity;
    while (newCapacity < needed) {
        newCapacity *= 2;
    }

    Employee **temp = realloc(array, newCapacity * sizeof(Employee *));
    if (temp == NULL) {
        return NULL;
    }

    *capacity = newCapacity;
    return temp;
}

void removeFromArray(Employee **array, int *count, Employee *target) {
    for (int i = 0; i < *count; i++) {
        if (array[i] == target) {
            array[i] = array[*count - 1];
            (*count)--;
            return;
        }
    }
}

int isCeoDesignation(const char *designation) {
    const char *ceo = CEO_DESIGNATION;
    int i = 0;

    while (designation[i] != '\0' && ceo[i] != '\0') {
        if (tolower((unsigned char)designation[i]) != tolower((unsigned char)ceo[i])) {
            return 0;
        }
        i++;
    }

    return designation[i] == ceo[i];
}

Employee *findEmployee(int id) {
    for (int i = 0; i < totalEmployees; i++) {
        if (allEmployees[i]->id == id) {
            return allEmployees[i];
        }
    }
    return NULL;
}

Employee *createEmployee(int id, const char *name, const char *designation) {
    Employee *employee = calloc(1, sizeof(Employee));

    if (employee == NULL) {
        return NULL;
    }

    employee->id = id;
    strcpy(employee->name, name);
    strcpy(employee->designation, designation);
    return employee;
}

Status addEmployee(void) {
    int id, managerId;
    char name[TEXT_SIZE], designation[TEXT_SIZE];

    if (!readInt("Enter Employee ID: ", &id) || id == 0) {
        return ERR_INPUT;
    }
    if (findEmployee(id) != NULL) {
        return ERR_DUPLICATE;
    }
    if (!readLine("Enter Name: ", name, sizeof(name)) ||
        !readLine("Enter Designation: ", designation, sizeof(designation))) {
        return ERR_INPUT;
    }
    if (!readInt("Enter Manager ID (0 for top-level): ", &managerId)) {
        return ERR_INPUT;
    }

    if (managerId == id) {
        return ERR_INPUT;
    }

    Employee *manager = NULL;
    if (managerId != NO_MANAGER) {
        manager = findEmployee(managerId);
        if (manager == NULL) {
            return ERR_NO_MANAGER;
        }
    }

    Employee **newList = growArray(allEmployees, &totalCapacity, totalEmployees + 1);
    if (newList == NULL) {
        return ERR_MEMORY;
    }
    allEmployees = newList;

    Employee **newReports = NULL;
    if (manager != NULL) {
        newReports = growArray(manager->reports, &manager->reportCapacity, manager->reportCount + 1);
        if (newReports == NULL) {
            return ERR_MEMORY;
        }
    }

    Employee *employee = createEmployee(id, name, designation);
    if (employee == NULL) {
        return ERR_MEMORY;
    }

    if (manager != NULL) {
        manager->reports = newReports;
        manager->reports[manager->reportCount++] = employee;
        employee->manager = manager;
    }
    allEmployees[totalEmployees++] = employee;

    printf("Employee added successfully.\n");
    return OK;
}

Status moveReports(Employee *employee) {
    Employee *manager = employee->manager;

    if (manager != NULL) {
        Employee **newReports = growArray(manager->reports, &manager->reportCapacity,
                                          manager->reportCount + employee->reportCount);
        if (newReports == NULL) {
            return ERR_MEMORY;
        }
        manager->reports = newReports;
    }

    for (int i = 0; i < employee->reportCount; i++) {
        Employee *report = employee->reports[i];
        report->manager = manager;

        if (manager != NULL) {
            manager->reports[manager->reportCount++] = report;
        }
    }

    employee->reportCount = 0;
    return OK;
}

Status removeEmployee(int id) {
    Employee *employee = findEmployee(id);

    if (employee == NULL) {
        return ERR_NOT_FOUND;
    }
    if (isCeoDesignation(employee->designation)) {
        return ERR_CEO;
    }

    if (employee->manager != NULL) {
        removeFromArray(employee->manager->reports, &employee->manager->reportCount, employee);
    }

    Status status = moveReports(employee);
    if (status != OK) {
        return status;
    }

    removeFromArray(allEmployees, &totalEmployees, employee);
    free(employee->reports);
    free(employee);

    printf("Employee removed successfully.\n");
    return OK;
}

void printEmployeeTree(Employee *employee, int level) {
    for (int i = 0; i < level; i++) {
        printf("    ");
    }
    printf("%d - %s - %s\n", employee->id, employee->name, employee->designation);

    for (int i = 0; i < employee->reportCount; i++) {
        printEmployeeTree(employee->reports[i], level + 1);
    }
}

int countEmployeesUnder(Employee *employee) {
    int count = 0;

    for (int i = 0; i < employee->reportCount; i++) {
        count += 1 + countEmployeesUnder(employee->reports[i]);
    }
    return count;
}

void displayEmployeeHierarchy(int id) {
    Employee *employee = findEmployee(id);

    if (employee == NULL) {
        printf("Employee not found.\n");
        return;
    }

    printEmployeeTree(employee, 0);
    printf("Total Employees Under %d: %d\n", id, countEmployeesUnder(employee));
}

void displayCompleteHierarchy(void) {
    printf("\n===== ORGANIZATION HIERARCHY =====\n");

    for (int i = 0; i < totalEmployees; i++) {
        if (allEmployees[i]->manager == NULL) {
            printEmployeeTree(allEmployees[i], 0);
        }
    }
    printf("Total Employees: %d\n", totalEmployees);
}

void showStatus(Status status) {
    switch (status) {
        case OK:             break;
        case ERR_INPUT:      printf("Invalid input.\n"); break;
        case ERR_NOT_FOUND:  printf("Employee not found.\n"); break;
        case ERR_DUPLICATE:  printf("Employee ID already exists.\n"); break;
        case ERR_NO_MANAGER: printf("Reporting manager not found.\n"); break;
        case ERR_CEO:        printf("The CEO cannot be removed.\n"); break;
        case ERR_MEMORY:     printf("Memory allocation failed.\n"); break;
    }
}

void freeEverything(void) {
    for (int i = 0; i < totalEmployees; i++) {
        free(allEmployees[i]->reports);
        free(allEmployees[i]);
    }
    free(allEmployees);
}

void showMenu(void) {
    printf("\n===== EMPLOYEE HIERARCHY MANAGEMENT =====\n");
    printf("1. Add Employee\n");
    printf("2. Remove Employee\n");
    printf("3. Display Employee Hierarchy\n");
    printf("4. Display Complete Hierarchy\n");
    printf("5. Exit\n");
}

int main(void) {
    int choice, id;
    int running = 1;

    while (running) {
        showMenu();

        if (!readInt("Enter your choice: ", &choice)) {
            if (feof(stdin)) {
                break;
            }
            printf("Invalid menu choice.\n");
            continue;
        }

        switch (choice) {
            case 1:
                showStatus(addEmployee());
                break;

            case 2:
                if (readInt("Enter Employee ID to remove: ", &id)) {
                    showStatus(removeEmployee(id));
                } else {
                    printf("Invalid employee ID.\n");
                }
                break;

            case 3:
                if (readInt("Enter Employee ID: ", &id)) {
                    displayEmployeeHierarchy(id);
                } else {
                    printf("Invalid employee ID.\n");
                }
                break;

            case 4:
                displayCompleteHierarchy();
                break;

            case 5:
                running = 0;
                break;

            default:
                printf("Select a choice from 1 to 5.\n");
        }
    }

    freeEverything();
    printf("Program terminated successfully.\n");
    return 0;
}
