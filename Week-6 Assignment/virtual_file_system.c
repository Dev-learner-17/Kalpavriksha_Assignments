#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

enum {
    BLOCK_SIZE = 512,
    MAX_BLOCK_COUNT = 5000,
    MAX_NAME_LENGTH = 50,
    PERCENTAGE_SCALE = 100,
    INPUT_BUFFER_SIZE = 128
};

typedef enum {
    NODE_FILE,
    NODE_DIRECTORY
} NodeType;

typedef enum {
    STATUS_SUCCESS,
    STATUS_INVALID_INPUT,
    STATUS_NOT_FOUND,
    STATUS_ALREADY_EXISTS,
    STATUS_NOT_DIRECTORY,
    STATUS_IS_DIRECTORY,
    STATUS_DIRECTORY_NOT_EMPTY,
    STATUS_DISK_FULL,
    STATUS_MEMORY_ERROR
} Status;

typedef struct FileNode {
    char name[MAX_NAME_LENGTH + 1];
    NodeType type;
    struct FileNode *parent;
    struct FileNode *next;
    struct FileNode *previous;
    struct FileNode *children;
    int *blockPointers;
    size_t blockCount;
    size_t contentSize;
} FileNode;

typedef struct FreeBlock {
    int index;
    struct FreeBlock *next;
    struct FreeBlock *previous;
} FreeBlock;

typedef struct {
    unsigned char (*virtualDisk)[BLOCK_SIZE];
    FreeBlock *freeBlockNodes;
    FreeBlock *freeHead;
    FreeBlock *freeTail;
    FileNode *root;
    FileNode *currentDirectory;
    int totalBlocks;
    int freeBlockCount;
} VirtualFileSystem;

static void clearInputBuffer()
{
    int character;

    while ((character = getchar()) != '\n' && character != EOF) {
    }
}

static int readInteger(const char *prompt, int *value)
{
    char input[INPUT_BUFFER_SIZE];
    size_t index = 0;
    unsigned int number = 0;
    unsigned int maximumInteger = ((unsigned int)-1) >> 1;
    bool hasDigit = false;

    if (prompt == NULL || value == NULL) {
        return -1;
    }

    printf("%s", prompt);
    fflush(stdout);

    if (fgets(input, sizeof(input), stdin) == NULL) {
        return 0;
    }

    if (strchr(input, '\n') == NULL && !feof(stdin)) {
        clearInputBuffer();
        return -1;
    }

    while (input[index] == ' ' || input[index] == '\t') {
        index++;
    }

    while (input[index] >= '0' && input[index] <= '9') {
        unsigned int digit = (unsigned int)(input[index] - '0');

        if (number > (maximumInteger - digit) / 10U) {
            return -1;
        }

        number = number * 10U + digit;
        hasDigit = true;
        index++;
    }

    while (input[index] == ' ' || input[index] == '\t' ||
           input[index] == '\n' || input[index] == '\r') {
        index++;
    }

    if (!hasDigit || input[index] != '\0') {
        return -1;
    }

    *value = (int)number;
    return 1;
}

static int getValidInput(const char *prompt, int minimum, int maximum, int *value)
{
    int inputStatus;

    if (prompt == NULL || value == NULL || minimum > maximum) {
        return 0;
    }

    while (true) {
        inputStatus = readInteger(prompt, value);

        if (inputStatus == 0) {
            return 0;
        }

        if (inputStatus == 1 && *value >= minimum && *value <= maximum) {
            return 1;
        }

        printf("Invalid input. Enter a value between %d and %d.\n",
               minimum, maximum);
    }
}

static char *readCommandLine()
{
    size_t capacity = BLOCK_SIZE;
    size_t length = 0;
    char *line = malloc(capacity);
    int character;

    if (line == NULL) {
        return NULL;
    }

    while ((character = getchar()) != EOF && character != '\n') {
        if (length + 1 >= capacity) {
            size_t newCapacity = capacity * 2;
            char *resizedLine = realloc(line, newCapacity);

            if (resizedLine == NULL) {
                free(line);
                return NULL;
            }

            line = resizedLine;
            capacity = newCapacity;
        }

        line[length++] = (char)character;
    }

    if (character == EOF && length == 0) {
        free(line);
        return NULL;
    }

    line[length] = '\0';
    return line;
}

static FileNode *createFileNode(const char *name, NodeType type, FileNode *parent)
{
    FileNode *newNode = calloc(1, sizeof(*newNode));

    if (newNode == NULL) {
        return NULL;
    }

    strcpy(newNode->name, name);
    newNode->type = type;
    newNode->parent = parent;
    newNode->next = newNode;
    newNode->previous = newNode;

    return newNode;
}

static void insertChild(FileNode *parent, FileNode *newNode)
{
    FileNode *lastChild;

    if (parent->children == NULL) {
        parent->children = newNode;
    } else {
        lastChild = parent->children->previous;
        newNode->next = parent->children;
        newNode->previous = lastChild;
        lastChild->next = newNode;
        parent->children->previous = newNode;
    }

    newNode->parent = parent;
}

static void detachChild(FileNode *node)
{
    FileNode *parent = node->parent;

    if (node->next == node) {
        parent->children = NULL;
    } else {
        node->previous->next = node->next;
        node->next->previous = node->previous;

        if (parent->children == node) {
            parent->children = node->next;
        }
    }

    node->next = node;
    node->previous = node;
    node->parent = NULL;
}

static FileNode *findChild(FileNode *directory, const char *name)
{
    FileNode *currentChild;

    if (directory->children == NULL) {
        return NULL;
    }

    currentChild = directory->children;

    do {
        if (strcmp(currentChild->name, name) == 0) {
            return currentChild;
        }

        currentChild = currentChild->next;
    } while (currentChild != directory->children);

    return NULL;
}

static bool initializeFileSystem(VirtualFileSystem *fileSystem, int blockCount)
{
    int blockIndex;

    memset(fileSystem, 0, sizeof(*fileSystem));

    fileSystem->virtualDisk = calloc((size_t)blockCount, sizeof(*fileSystem->virtualDisk));
    fileSystem->freeBlockNodes = calloc((size_t)blockCount, sizeof(*fileSystem->freeBlockNodes));

    if (fileSystem->virtualDisk == NULL || fileSystem->freeBlockNodes == NULL) {
        free(fileSystem->virtualDisk);
        free(fileSystem->freeBlockNodes);
        memset(fileSystem, 0, sizeof(*fileSystem));
        return false;
    }

    fileSystem->totalBlocks = blockCount;
    fileSystem->freeBlockCount = blockCount;

    for (blockIndex = 0; blockIndex < blockCount; blockIndex++) {
        FreeBlock *blockNode = &fileSystem->freeBlockNodes[blockIndex];

        blockNode->index = blockIndex;
        blockNode->previous = blockIndex == 0 ? NULL : &fileSystem->freeBlockNodes[blockIndex - 1];
        blockNode->next = blockIndex == blockCount - 1 ? NULL : &fileSystem->freeBlockNodes[blockIndex + 1];
    }

    fileSystem->freeHead = &fileSystem->freeBlockNodes[0];
    fileSystem->freeTail = &fileSystem->freeBlockNodes[blockCount - 1];

    fileSystem->root = createFileNode("/", NODE_DIRECTORY, NULL);

    if (fileSystem->root == NULL) {
        free(fileSystem->virtualDisk);
        free(fileSystem->freeBlockNodes);
        memset(fileSystem, 0, sizeof(*fileSystem));
        return false;
    }

    fileSystem->currentDirectory = fileSystem->root;
    return true;
}

static bool validateName(const char *name)
{
    size_t nameLength;

    if (name == NULL || name[0] == '\0' || strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        return false;
    }

    nameLength = strlen(name);

    if (nameLength > MAX_NAME_LENGTH || strchr(name, '/') != NULL) {
        return false;
    }

    return true;
}

static Status createEntry(VirtualFileSystem *fileSystem, const char *name, NodeType type)
{
    FileNode *newNode;

    if (!validateName(name)) {
        return STATUS_INVALID_INPUT;
    }

    if (findChild(fileSystem->currentDirectory, name) != NULL) {
        return STATUS_ALREADY_EXISTS;
    }

    newNode = createFileNode(name, type, fileSystem->currentDirectory);

    if (newNode == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    insertChild(fileSystem->currentDirectory, newNode);
    return STATUS_SUCCESS;
}

static void removeFreeBlockFromHead(VirtualFileSystem *fileSystem)
{
    FreeBlock *removedBlock = fileSystem->freeHead;

    fileSystem->freeHead = removedBlock->next;

    if (fileSystem->freeHead == NULL) {
        fileSystem->freeTail = NULL;
    } else {
        fileSystem->freeHead->previous = NULL;
    }

    removedBlock->next = NULL;
    removedBlock->previous = NULL;
    fileSystem->freeBlockCount--;
}

static void appendFreeBlock(VirtualFileSystem *fileSystem, int blockIndex)
{
    FreeBlock *newFreeBlock = &fileSystem->freeBlockNodes[blockIndex];

    newFreeBlock->next = NULL;
    newFreeBlock->previous = fileSystem->freeTail;

    if (fileSystem->freeTail == NULL) {
        fileSystem->freeHead = newFreeBlock;
    } else {
        fileSystem->freeTail->next = newFreeBlock;
    }

    fileSystem->freeTail = newFreeBlock;
    fileSystem->freeBlockCount++;
}

static void releaseFileBlocks(VirtualFileSystem *fileSystem, FileNode *file)
{
    size_t blockPosition;

    for (blockPosition = 0; blockPosition < file->blockCount; blockPosition++) {
        int blockIndex = file->blockPointers[blockPosition];

        memset(fileSystem->virtualDisk[blockIndex], 0, BLOCK_SIZE);
        appendFreeBlock(fileSystem, blockIndex);
    }

    free(file->blockPointers);
    file->blockPointers = NULL;
    file->blockCount = 0;
    file->contentSize = 0;
}

static Status writeFile(VirtualFileSystem *fileSystem, const char *fileName, const char *content)
{
    FileNode *file = findChild(fileSystem->currentDirectory, fileName);
    size_t contentSize = strlen(content);
    size_t requiredBlocks = contentSize / BLOCK_SIZE;
    size_t blockPosition;
    size_t bytesCopied = 0;
    int *newBlockPointers = NULL;

    if (file == NULL) {
        return STATUS_NOT_FOUND;
    }

    if (file->type != NODE_FILE) {
        return STATUS_IS_DIRECTORY;
    }

    if (contentSize % BLOCK_SIZE != 0) {
        requiredBlocks++;
    }

    if (requiredBlocks > (size_t)fileSystem->freeBlockCount + file->blockCount) {
        return STATUS_DISK_FULL;
    }

    if (requiredBlocks > 0) {
        newBlockPointers = malloc(requiredBlocks * sizeof(*newBlockPointers));

        if (newBlockPointers == NULL) {
            return STATUS_MEMORY_ERROR;
        }
    }

    releaseFileBlocks(fileSystem, file);

    for (blockPosition = 0; blockPosition < requiredBlocks; blockPosition++) {
        FreeBlock *allocatedBlock = fileSystem->freeHead;
        size_t bytesRemaining = contentSize - bytesCopied;
        size_t bytesToCopy = bytesRemaining < BLOCK_SIZE ? bytesRemaining : BLOCK_SIZE;

        newBlockPointers[blockPosition] = allocatedBlock->index;
        removeFreeBlockFromHead(fileSystem);
        memcpy(fileSystem->virtualDisk[newBlockPointers[blockPosition]], content + bytesCopied, bytesToCopy);
        bytesCopied += bytesToCopy;
    }

    file->blockPointers = newBlockPointers;
    file->blockCount = requiredBlocks;
    file->contentSize = contentSize;

    return STATUS_SUCCESS;
}

static Status readFile(VirtualFileSystem *fileSystem, const char *fileName)
{
    FileNode *file = findChild(fileSystem->currentDirectory, fileName);
    size_t bytesRemaining;
    size_t blockPosition;

    if (file == NULL) {
        return STATUS_NOT_FOUND;
    }

    if (file->type != NODE_FILE) {
        return STATUS_IS_DIRECTORY;
    }

    bytesRemaining = file->contentSize;

    for (blockPosition = 0; blockPosition < file->blockCount; blockPosition++) {
        size_t bytesToPrint = bytesRemaining < BLOCK_SIZE ? bytesRemaining : BLOCK_SIZE;

        fwrite(fileSystem->virtualDisk[file->blockPointers[blockPosition]], sizeof(unsigned char), bytesToPrint, stdout);
        bytesRemaining -= bytesToPrint;
    }

    putchar('\n');
    return STATUS_SUCCESS;
}

static Status deleteFile(VirtualFileSystem *fileSystem, const char *fileName)
{
    FileNode *file = findChild(fileSystem->currentDirectory, fileName);

    if (file == NULL) {
        return STATUS_NOT_FOUND;
    }

    if (file->type != NODE_FILE) {
        return STATUS_IS_DIRECTORY;
    }

    detachChild(file);
    releaseFileBlocks(fileSystem, file);
    free(file);
    return STATUS_SUCCESS;
}

static Status removeDirectory(VirtualFileSystem *fileSystem, const char *directoryName)
{
    FileNode *directory = findChild(fileSystem->currentDirectory, directoryName);

    if (directory == NULL) {
        return STATUS_NOT_FOUND;
    }

    if (directory->type != NODE_DIRECTORY) {
        return STATUS_NOT_DIRECTORY;
    }

    if (directory->children != NULL) {
        return STATUS_DIRECTORY_NOT_EMPTY;
    }

    detachChild(directory);
    free(directory);
    return STATUS_SUCCESS;
}

static Status changeDirectory(VirtualFileSystem *fileSystem, const char *directoryName)
{
    FileNode *targetDirectory;

    if (strcmp(directoryName, "/") == 0) {
        fileSystem->currentDirectory = fileSystem->root;
        return STATUS_SUCCESS;
    }

    if (strcmp(directoryName, "..") == 0) {
        if (fileSystem->currentDirectory->parent != NULL) {
            fileSystem->currentDirectory = fileSystem->currentDirectory->parent;
        }

        return STATUS_SUCCESS;
    }

    if (strcmp(directoryName, ".") == 0) {
        return STATUS_SUCCESS;
    }

    targetDirectory = findChild(fileSystem->currentDirectory, directoryName);

    if (targetDirectory == NULL) {
        return STATUS_NOT_FOUND;
    }

    if (targetDirectory->type != NODE_DIRECTORY) {
        return STATUS_NOT_DIRECTORY;
    }

    fileSystem->currentDirectory = targetDirectory;
    return STATUS_SUCCESS;
}

static void listDirectory(const FileNode *directory)
{
    const FileNode *currentChild;

    if (directory->children == NULL) {
        printf("(empty)\n");
        return;
    }

    currentChild = directory->children;

    do {
        if (currentChild->type == NODE_DIRECTORY) {
            printf("%s/\n", currentChild->name);
        } else {
            printf("%s\n", currentChild->name);
        }

        currentChild = currentChild->next;
    } while (currentChild != directory->children);
}

static void printDirectoryPath(const FileNode *directory)
{
    if (directory->parent == NULL) {
        putchar('/');
        return;
    }

    printDirectoryPath(directory->parent);

    if (directory->parent->parent != NULL) {
        putchar('/');
    }

    printf("%s", directory->name);
}

static void printWorkingDirectory(const FileNode *directory)
{
    printDirectoryPath(directory);
}

static void printCommandPrompt(const FileNode *currentDirectory)
{
    if (currentDirectory->parent == NULL) {
        printf("/ > ");
    } else {
        printf("%s > ", currentDirectory->name);
    }
}

static void displayDiskUsage(const VirtualFileSystem *fileSystem)
{
    int usedBlocks = fileSystem->totalBlocks - fileSystem->freeBlockCount;
    double diskUsage = (double)usedBlocks * PERCENTAGE_SCALE / fileSystem->totalBlocks;

    printf("Total Blocks: %d\n", fileSystem->totalBlocks);
    printf("Used Blocks: %d\n", usedBlocks);
    printf("Free Blocks: %d\n", fileSystem->freeBlockCount);
    printf("Disk Usage: %.2f%%\n", diskUsage);
}

static void destroyDirectoryTree(FileNode *directory)
{
    FileNode *firstChild = directory->children;

    while (firstChild != NULL) {
        FileNode *childToDelete = firstChild;

        if (childToDelete->next == childToDelete) {
            firstChild = NULL;
        } else {
            firstChild = childToDelete->next;
            childToDelete->previous->next = childToDelete->next;
            childToDelete->next->previous = childToDelete->previous;
        }

        if (childToDelete->type == NODE_DIRECTORY) {
            destroyDirectoryTree(childToDelete);
        } else {
            free(childToDelete->blockPointers);
        }

        free(childToDelete);
    }

    directory->children = NULL;
}

static void destroyFileSystem(VirtualFileSystem *fileSystem)
{
    if (fileSystem->root != NULL) {
        destroyDirectoryTree(fileSystem->root);
        free(fileSystem->root);
    }

    free(fileSystem->freeBlockNodes);
    free(fileSystem->virtualDisk);
    memset(fileSystem, 0, sizeof(*fileSystem));
}

static char *skipWhitespace(char *text)
{
    while (*text != '\0' && isspace((unsigned char)*text)) {
        text++;
    }

    return text;
}

static char *extractToken(char **text)
{
    char *tokenStart;
    size_t tokenLength;

    *text = skipWhitespace(*text);

    if (**text == '\0') {
        return NULL;
    }

    tokenStart = *text;

    while (**text != '\0' && !isspace((unsigned char)**text)) {
        (*text)++;
    }

    tokenLength = (size_t)(*text - tokenStart);

    if (**text != '\0') {
        **text = '\0';
        (*text)++;
    }

    if (tokenLength == 0) {
        return NULL;
    }

    return tokenStart;
}

static char *extractWriteContent(char *text)
{
    char *contentStart;
    char *contentEnd;

    text = skipWhitespace(text);

    if (*text == '"') {
        contentStart = text + 1;
        contentEnd = strrchr(contentStart, '"');

        if (contentEnd == NULL) {
            return NULL;
        }

        *contentEnd = '\0';
        return contentStart;
    }

    return text;
}

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

static bool processCommand(VirtualFileSystem *fileSystem, char *commandLine)
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

int main()
{
    VirtualFileSystem fileSystem;
    int blockCount;
    char blockCountPrompt[INPUT_BUFFER_SIZE];

    snprintf(blockCountPrompt, sizeof(blockCountPrompt),
             "Enter number of disk blocks (1-%d): ", MAX_BLOCK_COUNT);

    if (!getValidInput(blockCountPrompt, 1, MAX_BLOCK_COUNT, &blockCount)) {
        printf("Input ended. Exiting program.\n");
        return EXIT_FAILURE;
    }

    if (!initializeFileSystem(&fileSystem, blockCount)) {
        printf("Unable to initialize virtual file system.\n");
        return EXIT_FAILURE;
    }

    printf("Compact VFS - ready. Type 'exit' to quit.\n");

    while (1) {
        char *commandLine;

        printCommandPrompt(fileSystem.currentDirectory);
        fflush(stdout);

        commandLine = readCommandLine();

        if (commandLine == NULL) {
            break;
        }

        if (!processCommand(&fileSystem, commandLine)) {
            free(commandLine);
            break;
        }

        free(commandLine);
    }

    destroyFileSystem(&fileSystem);
    printf("Memory released. Exiting program...\n");

    return EXIT_SUCCESS;
}
