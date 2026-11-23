#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vfs.h"

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

bool initializeFileSystem(VirtualFileSystem *fileSystem, int blockCount)
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

Status createEntry(VirtualFileSystem *fileSystem, const char *name, NodeType type)
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

Status writeFile(VirtualFileSystem *fileSystem, const char *fileName, const char *content)
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

Status readFile(VirtualFileSystem *fileSystem, const char *fileName)
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

Status deleteFile(VirtualFileSystem *fileSystem, const char *fileName)
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

Status removeDirectory(VirtualFileSystem *fileSystem, const char *directoryName)
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

Status changeDirectory(VirtualFileSystem *fileSystem, const char *directoryName)
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

void listDirectory(const FileNode *directory)
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

void printWorkingDirectory(const FileNode *directory)
{
    printDirectoryPath(directory);
}

void printCommandPrompt(const FileNode *currentDirectory)
{
    if (currentDirectory->parent == NULL) {
        printf("/ > ");
    } else {
        printf("%s > ", currentDirectory->name);
    }
}

void displayDiskUsage(const VirtualFileSystem *fileSystem)
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

void destroyFileSystem(VirtualFileSystem *fileSystem)
{
    if (fileSystem->root != NULL) {
        destroyDirectoryTree(fileSystem->root);
        free(fileSystem->root);
    }

    free(fileSystem->freeBlockNodes);
    free(fileSystem->virtualDisk);
    memset(fileSystem, 0, sizeof(*fileSystem));
}
