#include <stdio.h>
#include <stdlib.h>
#include "vfs.h"

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
