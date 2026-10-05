#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "users.txt"
#define TEMP_FILE_NAME "temp.txt"

struct User {
    unsigned int id;
    char name[50];
    unsigned short age;
};

void clearInputBuffer() {
    int ch;

    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

void readName(char name[], size_t size) {
    if (fgets(name, size, stdin) != NULL) {
        name[strcspn(name, "\n")] = '\0';
    }
}

int replaceFileWithTemp() {
    if (remove(FILE_NAME) != 0) {
        printf("Error: Unable to remove the original file.\n");
        return 0;
    }

    if (rename(TEMP_FILE_NAME, FILE_NAME) != 0) {
        printf("Error: Unable to rename temporary file.\n");
        return 0;
    }

    return 1;
}

void createUser() {
    struct User user;
    struct User tempUser;
    FILE *file;

    printf("Enter User ID: ");

    if (scanf("%u", &user.id) != 1) {
        printf("Invalid User ID.\n");
        clearInputBuffer();
        return;
    }

    file = fopen(FILE_NAME, "r");

    if (file != NULL) {
        while (fscanf(file, "%u|%49[^|]|%hu", &tempUser.id, tempUser.name, &tempUser.age) == 3) {

            if (tempUser.id == user.id) {
                printf("User ID already exists.\n");
                fclose(file);
                return;
            }
        }

        fclose(file);
    }

    clearInputBuffer();

    printf("Enter Name: ");
    readName(user.name, sizeof(user.name));

    printf("Enter Age: ");

    if (scanf("%hu", &user.age) != 1) {
        printf("Invalid Age.\n");
        clearInputBuffer();
        return;
    }

    file = fopen(FILE_NAME, "a");

    if (file == NULL) {
        printf("Error: Unable to open file.\n");
        return;
    }

    fprintf(file, "%u|%s|%hu\n", user.id, user.name, user.age);

    fclose(file);

    printf("User added successfully.\n");
}

void readUsers() {
    struct User user;
    FILE *file;

    file = fopen(FILE_NAME, "r");

    if (file == NULL) {
        printf("No users found.\n");
        return;
    }

    printf("\nUser Records\n");

    while (fscanf(file, "%u|%49[^|]|%hu", &user.id, user.name, &user.age) == 3) {

        printf("ID   : %u\n", user.id);
        printf("Name : %s\n", user.name);
        printf("Age  : %hu\n", user.age);
        printf("\n");
    }

    fclose(file);
}

void updateUser() {
    struct User user;
    FILE *file;
    FILE *temp;
    unsigned int id;
    int found = 0;

    printf("Enter User ID to update: ");

    if (scanf("%u", &id) != 1) {
        printf("Invalid User ID.\n");
        clearInputBuffer();
        return;
    }

    file = fopen(FILE_NAME, "r");

    if (file == NULL) {
        printf("No users found.\n");
        return;
    }

    temp = fopen(TEMP_FILE_NAME, "w");

    if (temp == NULL) {
        printf("Error: Unable to create temporary file.\n");
        fclose(file);
        return;
    }

    while (fscanf(file, "%u|%49[^|]|%hu", &user.id, user.name, &user.age) == 3) {

        if (user.id == id) {
            found = 1;

            clearInputBuffer();

            printf("Enter New Name: ");
            readName(user.name, sizeof(user.name));

            printf("Enter New Age: ");

            if (scanf("%hu", &user.age) != 1) {
                printf("Invalid Age.\n");
                clearInputBuffer();
                fclose(file);
                fclose(temp);
                remove(TEMP_FILE_NAME);
                return;
            }
        }

        fprintf(temp, "%u|%s|%hu\n", user.id, user.name, user.age);
    }

    fclose(file);
    fclose(temp);

    if (!replaceFileWithTemp()) {
        return;
    }

    if (found) {
        printf("User updated successfully.\n");
    }
    else {
        printf("User ID not found.\n");
    }
}

void deleteUser() {
    struct User user;
    FILE *file;
    FILE *temp;
    unsigned int id;
    int found = 0;

    printf("Enter User ID to delete: ");

    if (scanf("%u", &id) != 1) {
        printf("Invalid User ID.\n");
        clearInputBuffer();
        return;
    }

    file = fopen(FILE_NAME, "r");

    if (file == NULL) {
        printf("No users found.\n");
        return;
    }

    temp = fopen(TEMP_FILE_NAME, "w");

    if (temp == NULL) {
        printf("Error: Unable to create temporary file.\n");
        fclose(file);
        return;
    }

    while (fscanf(file, "%u|%49[^|]|%hu", &user.id, user.name, &user.age) == 3) {

        if (user.id == id) {
            found = 1;
            continue;
        }

        fprintf(temp, "%u|%s|%hu\n", user.id,user.name, user.age);
    }

    fclose(file);
    fclose(temp);

    if (!replaceFileWithTemp()) {
        return;
    }

    if (found) {
        printf("User deleted successfully.\n");
    }
    else {
        printf("User ID not found.\n");
    }
}

int main() {
    int choice;
    FILE *file;

    file = fopen(FILE_NAME, "a");

    if (file == NULL) {
        printf("Error: Unable to create users.txt\n");
        return 1;
    }

    fclose(file);

    while (1) {
        printf("\nCRUD SYSTEM\n");
        printf("1. Create User\n");
        printf("2. Read Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid choice.\n");
            clearInputBuffer();
            continue;
        }

        switch (choice) {
            case 1:
                createUser();
                break;

            case 2:
                readUsers();
                break;

            case 3:
                updateUser();
                break;

            case 4:
                deleteUser();
                break;

            case 5:
                printf("Program terminated.\n");
                return 0;

            default:
                printf("Invalid choice.\n");
                break;
        }
    }

    return 0;
}