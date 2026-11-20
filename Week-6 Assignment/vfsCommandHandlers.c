#include <stdio.h>
#include <string.h>
#include "vfs.h"

static void displayStatus(Status status, const char *entryName, NodeType entryType)
{
    switch (status) {
        case STATUS_SUCCESS:
            if (entryType == NODE_DIRECTORY) {
                printf("Directory '%s' created successfully.\n", entryName);
            } else {
                printf("File '%s' created successfully.\n", entryName);
            }
            break;
        case STATUS_INVALID_INPUT:
            printf("Invalid name or command arguments.\n");
            break;
        case STATUS_NOT_FOUND:
            printf("Entry '%s' not found.\n", entryName);
            break;
        case STATUS_ALREADY_EXISTS:
            printf("An entry named '%s' already exists.\n", entryName);
            break;
        case STATUS_NOT_DIRECTORY:
            printf("'%s' is not a directory.\n", entryName);
            break;
        case STATUS_IS_DIRECTORY:
            printf("'%s' is a directory, not a file.\n", entryName);
            break;
        case STATUS_DIRECTORY_NOT_EMPTY:
            printf("Directory '%s' is not empty.\n", entryName);
            break;
        case STATUS_DISK_FULL:
            printf("Write failed: insufficient free disk blocks.\n");
            break;
        case STATUS_MEMORY_ERROR:
            printf("Operation failed: memory allocation error.\n");
            break;
    }
}

bool processCommand(VirtualFileSystem *fileSystem, char *commandLine)
{
    char *remainingText = commandLine;
    char *command = extractToken(&remainingText);
    char *argument;
    Status operationStatus;

    if (command == NULL) {
        return true;
    }

    if (strcmp(command, "cd..") == 0) {
        command = "cd";
        remainingText = "..";
    } else if (strcmp(command, "cd../") == 0) {
        command = "cd";
        remainingText = "..";
    }

    if (strcmp(command, "exit") == 0) {
        return false;
    }

    if (strcmp(command, "mkdir") == 0 || strcmp(command, "create") == 0) {
        NodeType requestedType = strcmp(command, "mkdir") == 0 ? NODE_DIRECTORY : NODE_FILE;

        argument = extractToken(&remainingText);

        if (argument == NULL || extractToken(&remainingText) != NULL) {
            displayStatus(STATUS_INVALID_INPUT, "", requestedType);
        } else {
            operationStatus = createEntry(fileSystem, argument, requestedType);

            if (operationStatus == STATUS_SUCCESS) {
                displayStatus(operationStatus, argument, requestedType);
            } else {
                displayStatus(operationStatus, argument, requestedType);
            }
        }

        return true;
    }

    if (strcmp(command, "write") == 0) {
        char *fileName = extractToken(&remainingText);
        char *content;

        if (fileName == NULL) {
            displayStatus(STATUS_INVALID_INPUT, "", NODE_FILE);
            return true;
        }

        content = extractWriteContent(remainingText);

        if (content == NULL) {
            displayStatus(STATUS_INVALID_INPUT, fileName, NODE_FILE);
            return true;
        }

        operationStatus = writeFile(fileSystem, fileName, content);

        if (operationStatus == STATUS_SUCCESS) {
            printf("Data written successfully (size=%zu bytes).\n", strlen(content));
        } else {
            displayStatus(operationStatus, fileName, NODE_FILE);
        }

        return true;
    }

    if (strcmp(command, "read") == 0 || strcmp(command, "delete") == 0 ||
        strcmp(command, "rmdir") == 0 || strcmp(command, "cd") == 0) {
        argument = extractToken(&remainingText);

        if (argument == NULL || extractToken(&remainingText) != NULL) {
            displayStatus(STATUS_INVALID_INPUT, "", NODE_FILE);
            return true;
        }

        if (strcmp(command, "read") == 0) {
            operationStatus = readFile(fileSystem, argument);

            if (operationStatus != STATUS_SUCCESS) {
                displayStatus(operationStatus, argument, NODE_FILE);
            }
        } else if (strcmp(command, "delete") == 0) {
            operationStatus = deleteFile(fileSystem, argument);

            if (operationStatus == STATUS_SUCCESS) {
                printf("File deleted successfully.\n");
            } else {
                displayStatus(operationStatus, argument, NODE_FILE);
            }
        } else if (strcmp(command, "rmdir") == 0) {
            operationStatus = removeDirectory(fileSystem, argument);

            if (operationStatus == STATUS_SUCCESS) {
                printf("Directory removed successfully.\n");
            } else {
                displayStatus(operationStatus, argument, NODE_DIRECTORY);
            }
        } else {
            operationStatus = changeDirectory(fileSystem, argument);

            if (operationStatus == STATUS_SUCCESS) {
                printf("Moved to ");
                printWorkingDirectory(fileSystem->currentDirectory);
            } else {
                displayStatus(operationStatus, argument, NODE_DIRECTORY);
            }

            if (operationStatus == STATUS_SUCCESS) {
                putchar('\n');
            }
        }

        return true;
    }

    if (strcmp(command, "ls") == 0 || strcmp(command, "pwd") == 0 || strcmp(command, "df") == 0) {
        if (extractToken(&remainingText) != NULL) {
            displayStatus(STATUS_INVALID_INPUT, "", NODE_FILE);
        } else if (strcmp(command, "ls") == 0) {
            listDirectory(fileSystem->currentDirectory);
        } else if (strcmp(command, "pwd") == 0) {
            printWorkingDirectory(fileSystem->currentDirectory);
            putchar('\n');
        } else {
            displayDiskUsage(fileSystem);
        }

        return true;
    }

    printf("Unknown command: %s\n", command);
    return true;
}
