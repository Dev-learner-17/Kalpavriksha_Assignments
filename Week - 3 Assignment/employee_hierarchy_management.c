#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define EMPLOYEE_NAME_LENGTH 50
#define EMPLOYEE_DESIGNATION_LENGTH 50
#define INPUT_LINE_LENGTH 100
#define INITIAL_ARRAY_CAPACITY 4
#define CEO_DESIGNATION "CEO"
#define NO_MANAGER_ID 0

typedef enum {
    STATUS_SUCCESS,
    STATUS_INVALID_INPUT,
    STATUS_EMPLOYEE_NOT_FOUND,
    STATUS_DUPLICATE_EMPLOYEE_ID,
    STATUS_MANAGER_NOT_FOUND,
    STATUS_CEO_PROTECTED,
    STATUS_MEMORY_ERROR
} OperationStatus;

typedef struct Employee {
    int employeeId;
    char employeeName[EMPLOYEE_NAME_LENGTH];
    char employeeDesignation[EMPLOYEE_DESIGNATION_LENGTH];
    struct Employee *reportingManager;
    struct Employee **directReports;
    int directReportCount;
    int directReportCapacity;
} Employee;

Employee **employeeList = NULL;
int employeeCount = 0;
int employeeCapacity = 0;

void clearInputBuffer(void) {
    int inputCharacter;

    while ((inputCharacter = getchar()) != '\n' && inputCharacter != EOF) {}
}

int readInputLine(const char *prompt, char *inputBuffer, int bufferSize) {
    printf("%s", prompt);

    if (fgets(inputBuffer, bufferSize, stdin) == NULL) {
        return 0;
    }

    int inputLength = (int)strlen(inputBuffer);

    if (inputBuffer[inputLength - 1] != '\n' && !feof(stdin)) {
        clearInputBuffer();
        return 0;
    }

    while (inputLength > 0 && isspace((unsigned char)inputBuffer[inputLength - 1])) {
        inputBuffer[--inputLength] = '\0';
    }

    int leadingSpaceCount = 0;
    while (isspace((unsigned char)inputBuffer[leadingSpaceCount])) {
        leadingSpaceCount++;
    }
    memmove(inputBuffer, inputBuffer + leadingSpaceCount, strlen(inputBuffer + leadingSpaceCount) + 1);

    return inputBuffer[0] != '\0';
}

int readInteger(const char *prompt, int *enteredNumber) {
    char inputLine[INPUT_LINE_LENGTH];
    char *firstInvalidCharacter;

    if (!readInputLine(prompt, inputLine, sizeof(inputLine))) {
        return 0;
    }

    long parsedValue = strtol(inputLine, &firstInvalidCharacter, 10);

    if (*firstInvalidCharacter != '\0' || parsedValue < 0 || parsedValue > 1000000000L) {
        return 0;
    }

    *enteredNumber = (int)parsedValue;
    return 1;
}

Employee **growEmployeeArray(Employee **pointerArray, int *arrayCapacity, int requiredCapacity) {
    if (requiredCapacity <= *arrayCapacity) {
        return pointerArray;
    }

    int newArrayCapacity = (*arrayCapacity == 0) ? INITIAL_ARRAY_CAPACITY : *arrayCapacity;
    while (newArrayCapacity < requiredCapacity) {
        newArrayCapacity *= 2;
    }

    Employee **resizedArray = realloc(pointerArray, newArrayCapacity * sizeof(Employee *));
    if (resizedArray == NULL) {
        return NULL;
    }

    *arrayCapacity = newArrayCapacity;
    return resizedArray;
}

void removeEmployeeFromArray(Employee **pointerArray, int *arrayItemCount, Employee *employeeToRemove) {
    for (int employeeIndex = 0; employeeIndex < *arrayItemCount; employeeIndex++) {
        if (pointerArray[employeeIndex] == employeeToRemove) {
            pointerArray[employeeIndex] = pointerArray[*arrayItemCount - 1];
            (*arrayItemCount)--;
            return;
        }
    }
}

int isCeoDesignation(const char *employeeDesignation) {
    const char *ceoDesignation = CEO_DESIGNATION;
    int characterIndex = 0;

    while (employeeDesignation[characterIndex] != '\0' && ceoDesignation[characterIndex] != '\0') {
        if (tolower((unsigned char)employeeDesignation[characterIndex]) != tolower((unsigned char)ceoDesignation[characterIndex])) {
            return 0;
        }
        characterIndex++;
    }

    return employeeDesignation[characterIndex] == ceoDesignation[characterIndex];
}

Employee *findEmployeeById(int employeeId) {
    for (int employeeIndex = 0; employeeIndex < employeeCount; employeeIndex++) {
        if (employeeList[employeeIndex]->employeeId == employeeId) {
            return employeeList[employeeIndex];
        }
    }
    return NULL;
}

Employee *createEmployeeRecord(int employeeId, const char *employeeName, const char *employeeDesignation) {
    Employee *employee = calloc(1, sizeof(Employee));

    if (employee == NULL) {
        return NULL;
    }

    employee->employeeId = employeeId;
    strcpy(employee->employeeName, employeeName);
    strcpy(employee->employeeDesignation, employeeDesignation);
    return employee;
}

OperationStatus addEmployee(void) {
    int employeeId, managerEmployeeId;
    char employeeName[EMPLOYEE_NAME_LENGTH], employeeDesignation[EMPLOYEE_DESIGNATION_LENGTH];

    if (!readInteger("Enter Employee ID: ", &employeeId) || employeeId == 0) {
        return STATUS_INVALID_INPUT;
    }
    if (findEmployeeById(employeeId) != NULL) {
        return STATUS_DUPLICATE_EMPLOYEE_ID;
    }
    if (!readInputLine("Enter Name: ", employeeName, sizeof(employeeName)) ||
        !readInputLine("Enter Designation: ", employeeDesignation, sizeof(employeeDesignation))) {
        return STATUS_INVALID_INPUT;
    }
    if (!readInteger("Enter Manager ID (0 for top-level): ", &managerEmployeeId)) {
        return STATUS_INVALID_INPUT;
    }

    if (managerEmployeeId == employeeId) {
        return STATUS_INVALID_INPUT;
    }

    Employee *reportingManager = NULL;
    if (managerEmployeeId != NO_MANAGER_ID) {
        reportingManager = findEmployeeById(managerEmployeeId);
        if (reportingManager == NULL) {
            return STATUS_MANAGER_NOT_FOUND;
        }
    }

    Employee **resizedEmployeeList = growEmployeeArray(employeeList, &employeeCapacity, employeeCount + 1);
    if (resizedEmployeeList == NULL) {
        return STATUS_MEMORY_ERROR;
    }
    employeeList = resizedEmployeeList;

    Employee **resizedDirectReports = NULL;
    if (reportingManager != NULL) {
        resizedDirectReports = growEmployeeArray(reportingManager->directReports, &reportingManager->directReportCapacity, reportingManager->directReportCount + 1);
        if (resizedDirectReports == NULL) {
            return STATUS_MEMORY_ERROR;
        }
    }

    Employee *employee = createEmployeeRecord(employeeId, employeeName, employeeDesignation);
    if (employee == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    if (reportingManager != NULL) {
        reportingManager->directReports = resizedDirectReports;
        reportingManager->directReports[reportingManager->directReportCount++] = employee;
        employee->reportingManager = reportingManager;
    }
    employeeList[employeeCount++] = employee;

    printf("Employee added successfully.\n");
    return STATUS_SUCCESS;
}

OperationStatus transferReportsToManager(Employee *employee) {
    Employee *reportingManager = employee->reportingManager;

    if (reportingManager != NULL) {
        Employee **resizedDirectReports = growEmployeeArray(reportingManager->directReports, &reportingManager->directReportCapacity,
                                          reportingManager->directReportCount + employee->directReportCount);
        if (resizedDirectReports == NULL) {
            return STATUS_MEMORY_ERROR;
        }
        reportingManager->directReports = resizedDirectReports;
    }

    for (int reportIndex = 0; reportIndex < employee->directReportCount; reportIndex++) {
        Employee *directReport = employee->directReports[reportIndex];
        directReport->reportingManager = reportingManager;

        if (reportingManager != NULL) {
            reportingManager->directReports[reportingManager->directReportCount++] = directReport;
        }
    }

    employee->directReportCount = 0;
    return STATUS_SUCCESS;
}

OperationStatus removeEmployee(int employeeId) {
    Employee *employee = findEmployeeById(employeeId);

    if (employee == NULL) {
        return STATUS_EMPLOYEE_NOT_FOUND;
    }
    if (isCeoDesignation(employee->employeeDesignation)) {
        return STATUS_CEO_PROTECTED;
    }

    if (employee->reportingManager != NULL) {
        removeEmployeeFromArray(employee->reportingManager->directReports, &employee->reportingManager->directReportCount, employee);
    }

    OperationStatus operationStatus = transferReportsToManager(employee);
    if (operationStatus != STATUS_SUCCESS) {
        return operationStatus;
    }

    removeEmployeeFromArray(employeeList, &employeeCount, employee);
    free(employee->directReports);
    free(employee);

    printf("Employee removed successfully.\n");
    return STATUS_SUCCESS;
}

void displayEmployeeTree(Employee *employee, int indentationLevel) {
    for (int indentationIndex = 0; indentationIndex < indentationLevel; indentationIndex++) {
        printf("    ");
    }
    printf("%d - %s - %s\n", employee->employeeId, employee->employeeName, employee->employeeDesignation);
    
    for (int reportIndex = 0; reportIndex < employee->directReportCount; reportIndex++) {
        displayEmployeeTree(employee->directReports[reportIndex], indentationLevel + 1);
    }
}

int countEmployeesUnder(Employee *employee) {
    int totalEmployeesBelow = 0;

    for (int reportIndex = 0; reportIndex < employee->directReportCount; reportIndex++) {
        totalEmployeesBelow += 1 + countEmployeesUnder(employee->directReports[reportIndex]);
    }
    return totalEmployeesBelow;
}

void displayEmployeeHierarchy(int employeeId) {
    Employee *employee = findEmployeeById(employeeId);

    if (employee == NULL) {
        printf("Employee not found.\n");
        return;
    }

    displayEmployeeTree(employee, 0);
    printf("Total Employees Under %d: %d\n", employeeId, countEmployeesUnder(employee));
}

void displayCompleteHierarchy(void) {
    printf("\n===== ORGANIZATION HIERARCHY =====\n");

    for (int employeeIndex = 0; employeeIndex < employeeCount; employeeIndex++) {
        if (employeeList[employeeIndex]->reportingManager == NULL) {
            displayEmployeeTree(employeeList[employeeIndex], 0);
        }
    }
    printf("Total Employees: %d\n", employeeCount);
}

void displayOperationStatus(OperationStatus operationStatus) {
    switch (operationStatus) {
        case STATUS_SUCCESS:             break;
        case STATUS_INVALID_INPUT:      printf("Invalid input.\n"); break;
        case STATUS_EMPLOYEE_NOT_FOUND:  printf("Employee not found.\n"); break;
        case STATUS_DUPLICATE_EMPLOYEE_ID:  printf("Employee ID already exists.\n"); break;
        case STATUS_MANAGER_NOT_FOUND: printf("Reporting manager not found.\n"); break;
        case STATUS_CEO_PROTECTED:        printf("The CEO cannot be removed.\n"); break;
        case STATUS_MEMORY_ERROR:     printf("Memory allocation failed.\n"); break;
    }
}

void freeAllEmployees(void) {
    for (int employeeIndex = 0; employeeIndex < employeeCount; employeeIndex++) {
        free(employeeList[employeeIndex]->directReports);
        free(employeeList[employeeIndex]);
    }
    free(employeeList);
}

void displayMenu(void) {
    printf("\n===== EMPLOYEE HIERARCHY MANAGEMENT =====\n");
    printf("1. Add Employee\n");
    printf("2. Remove Employee\n");
    printf("3. Display Employee Hierarchy\n");
    printf("4. Display Complete Hierarchy\n");
    printf("5. Exit\n");
}

int main(void) {
    int menuChoice, employeeId;
    int isRunning = 1;

    while (isRunning) {
        displayMenu();

        if (!readInteger("Enter your choice: ", &menuChoice)) {
            if (feof(stdin)) {
                break;
            }
            printf("Invalid menu choice.\n");
            continue;
        }

        switch (menuChoice) {
            case 1:
                displayOperationStatus(addEmployee());
                break;

            case 2:
                if (readInteger("Enter Employee ID to remove: ", &employeeId)) {
                    displayOperationStatus(removeEmployee(employeeId));
                } else {
                    printf("Invalid employee ID.\n");
                }
                break;

            case 3:
                if (readInteger("Enter Employee ID: ", &employeeId)) {
                    displayEmployeeHierarchy(employeeId);
                } else {
                    printf("Invalid employee ID.\n");
                }
                break;

            case 4:
                displayCompleteHierarchy();
                break;

            case 5:
                isRunning = 0;
                break;

            default:
                printf("Select a choice from 1 to 5.\n");
        }
    }

    freeAllEmployees();
    printf("Program terminated successfully.\n");
    return 0;
}
