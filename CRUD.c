#include <stdio.h>
#include <stdlib.h>

#define FILE_NAME "users.txt"

struct User {
    int id;
    char name[50];
    int age;
};

void createUser() {
    struct User user;
    struct User tempUser;
    FILE *file;

    printf("Enter User ID: ");

    if (scanf("%d", &user.id) != 1) {
        printf("Invalid User ID.\n");
        while (getchar() != '\n') {
        }
        return;
    }

    file = fopen(FILE_NAME, "r");

    if (file != NULL) {
        while (fscanf(file, "%d %s %d", &tempUser.id, tempUser.name, &tempUser.age) == 3) {
            if (tempUser.id == user.id) {
                printf("User ID already exists.\n");
                fclose(file);
                return;
            }
        }

        fclose(file);
    }

    printf("Enter Name: ");
    scanf("%s", user.name);

    printf("Enter Age: ");

    if (scanf("%d", &user.age) != 1) {
        printf("Invalid Age.\n");
        while (getchar() != '\n') {
        }
        return;
    }

    file = fopen(FILE_NAME, "a");

    if (file == NULL) {
        printf("Error: Unable to open file.\n");
        return;
    }

    fprintf(file, "%d %s %d\n", user.id, user.name, user.age);

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

    while (fscanf(file, "%d %s %d", &user.id, user.name, &user.age) == 3) {
        printf("ID   : %d\n", user.id);
        printf("Name : %s\n", user.name);
        printf("Age  : %d\n", user.age);
        printf("\n");
    }

    fclose(file);
}

void updateUser() {
    struct User user;
    FILE *file;
    FILE *temp;
    int id;
    int found = 0;

    printf("Enter User ID to update: ");

    if (scanf("%d", &id) != 1) {
        printf("Invalid User ID.\n");
        while (getchar() != '\n') {
        }
        return;
    }

    file = fopen(FILE_NAME, "r");

    if (file == NULL) {
        printf("No users found.\n");
        return;
    }

    temp = fopen("temp.txt", "w");

    if (temp == NULL) {
        printf("Error: Unable to create temporary file.\n");
        fclose(file);
        return;
    }

    while (fscanf(file, "%d %s %d", &user.id, user.name, &user.age) == 3) {
        if (user.id == id) {
            found = 1;

            printf("Enter New Name: ");
            scanf("%s", user.name);

            printf("Enter New Age: ");

            if (scanf("%d", &user.age) != 1) {
                printf("Invalid Age.\n");
                while (getchar() != '\n') {
                }
                fclose(file);
                fclose(temp);
                remove("temp.txt");
                return;
            }
        }

        fprintf(temp, "%d %s %d\n", user.id, user.name, user.age);
    }

    fclose(file);
    fclose(temp);

    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);

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
    int id;
    int found = 0;

    printf("Enter User ID to delete: ");

    if (scanf("%d", &id) != 1) {
        printf("Invalid User ID.\n");
        while (getchar() != '\n') {
        }
        return;
    }

    file = fopen(FILE_NAME, "r");

    if (file == NULL) {
        printf("No users found.\n");
        return;
    }

    temp = fopen("temp.txt", "w");

    if (temp == NULL) {
        printf("Error: Unable to create temporary file.\n");
        fclose(file);
        return;
    }

    while (fscanf(file, "%d %s %d", &user.id, user.name, &user.age) == 3) {
        if (user.id == id) {
            found = 1;
            continue;
        }

        fprintf(temp, "%d %s %d\n", user.id, user.name, user.age);
    }

    fclose(file);
    fclose(temp);

    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);

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

            while (getchar() != '\n') {
            }

            continue;
        }

        switch (choice) {
            case 1: {
                createUser();
                break;
            }

            case 2: {
                readUsers();
                break;
            }

            case 3: {
                updateUser();
                break;
            }

            case 4: {
                deleteUser();
                break;
            }

            case 5: {
                printf("Program terminated.\n");
                return 0;
            }

            default: {
                printf("Invalid choice.\n");
                break;
            }
        }
    }

    return 0;
}