#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_POOL_SIZE 1
#define MAX_POOL_SIZE 1000000
#define MIN_REQUEST_SIZE 1
#define INITIAL_CAPACITY 1
#define INPUT_BUFFER_SIZE 128
#define MAX_INTEGER_VALUE 2147483647
#define NO_BLOCK -1

typedef enum{
    FIRST_FIT = 1,
    BEST_FIT,
    WORST_FIT
}FitType;

typedef enum{
    FREE_BLOCK,
    ALLOCATED_BLOCK
}BlockStatus;

typedef struct{
    int startingOffset;
    int size;
    int allocationId;
    BlockStatus status;
}MemoryBlock;

typedef struct{
    unsigned char *memoryPool;
    MemoryBlock *blocks;
    int poolSize;
    int blockCount;
    int capacity;
    int nextId;
    FitType strategy;
    int idExhausted;
}MemoryManager;

void clearInputBuffer(void){
    int character;

    while((character = getchar()) != '\n' && character != EOF){
    }
}

int readInteger(const char *prompt, int *value){
    char input[INPUT_BUFFER_SIZE];
    int index = 0;
    int number = 0;
    int hasDigit = 0;

    printf("%s", prompt);

    if(fgets(input, sizeof(input), stdin) == NULL){
        return 0;
    }

    if(strchr(input, '\n') == NULL && !feof(stdin)){
        clearInputBuffer();
        return -1;
    }

    while(input[index] == ' ' || input[index] == '\t'){
        index++;
    }

    while(input[index] >= '0' && input[index] <= '9'){
        int digit = input[index] - '0';

        if(number > (MAX_INTEGER_VALUE - digit) / 10){
            return -1;
        }

        number = number * 10 + digit;
        hasDigit = 1;
        index++;
    }

    while(input[index] == ' ' || input[index] == '\t' ||
          input[index] == '\n' || input[index] == '\r'){
        index++;
    }

    if(!hasDigit || input[index] != '\0'){
        return -1;
    }

    *value = number;
    return 1;
}

int getValidInput(const char *prompt, int minimum, int maximum, int *value){
    int result;

    while(1){
        result = readInteger(prompt, value);

        if(result == 0){
            return 0;
        }

        if(result == 1 && *value >= minimum && *value <= maximum){
            return 1;
        }

        printf("Invalid input. Enter a value between %d and %d.\n", minimum, maximum);
    }
}

int initializeManager(MemoryManager *manager, int poolSize){
    manager->memoryPool = malloc(poolSize);
    manager->blocks = malloc(INITIAL_CAPACITY * sizeof(MemoryBlock));

    if(manager->memoryPool == NULL || manager->blocks == NULL){
        free(manager->memoryPool);
        free(manager->blocks);

        manager->memoryPool = NULL;
        manager->blocks = NULL;

        return 0;
    }

    manager->poolSize = poolSize;
    manager->blockCount = 1;
    manager->capacity = INITIAL_CAPACITY;
    manager->nextId = 1;
    manager->idExhausted = 0;

    manager->blocks[0] = (MemoryBlock){0, poolSize, 0, FREE_BLOCK};

    return 1;
}

void destroyManager(MemoryManager *manager){
    free(manager->memoryPool);
    free(manager->blocks);

    manager->memoryPool = NULL;
    manager->blocks = NULL;
    manager->blockCount = 0;
    manager->capacity = 0;
}

int ensureCapacity(MemoryManager *manager, int required){
    int newCapacity = manager->capacity;
    MemoryBlock *newBlocks;

    if(required <= manager->capacity){
        return 1;
    }

    if(required > manager->poolSize){
        return 0;
    }

    while(newCapacity < required){
        if(newCapacity > manager->poolSize / 2){
            newCapacity = manager->poolSize;
        }
        else{
            newCapacity *= 2;
        }
    }

    newBlocks = realloc(manager->blocks, newCapacity * sizeof(MemoryBlock));

    if(newBlocks == NULL){
        return 0;
    }

    manager->blocks = newBlocks;
    manager->capacity = newCapacity;

    return 1;
}

const char *getStrategyName(FitType strategy){
    switch(strategy){
        case FIRST_FIT:
            return "First Fit";

        case BEST_FIT:
            return "Best Fit";

        case WORST_FIT:
            return "Worst Fit";

        default:
            return "Unknown";
    }
}

int firstFit(const MemoryManager *manager, int requestedSize){
    for(int index = 0; index < manager->blockCount; index++){
        MemoryBlock *block = &manager->blocks[index];

        if(block->status == FREE_BLOCK && block->size >= requestedSize){
            return index;
        }
    }

    return NO_BLOCK;
}

int bestFit(const MemoryManager *manager, int requestedSize){
    int selectedIndex = NO_BLOCK;

    for(int index = 0; index < manager->blockCount; index++){
        MemoryBlock *block = &manager->blocks[index];

        if(block->status == FREE_BLOCK && block->size >= requestedSize &&
           (selectedIndex == NO_BLOCK || block->size < manager->blocks[selectedIndex].size)){
            selectedIndex = index;
        }
    }

    return selectedIndex;
}

int worstFit(const MemoryManager *manager, int requestedSize){
    int selectedIndex = NO_BLOCK;

    for(int index = 0; index < manager->blockCount; index++){
        MemoryBlock *block = &manager->blocks[index];

        if(block->status == FREE_BLOCK && block->size >= requestedSize &&
           (selectedIndex == NO_BLOCK || block->size > manager->blocks[selectedIndex].size)){
            selectedIndex = index;
        }
    }

    return selectedIndex;
}

int findFreeBlock(const MemoryManager *manager, int requestedSize){
    switch(manager->strategy){
        case FIRST_FIT:
            return firstFit(manager, requestedSize);

        case BEST_FIT:
            return bestFit(manager, requestedSize);

        case WORST_FIT:
            return worstFit(manager, requestedSize);

        default:
            return NO_BLOCK;
    }
}

int findAllocation(const MemoryManager *manager, int allocationId){
    for(int index = 0; index < manager->blockCount; index++){
        MemoryBlock *block = &manager->blocks[index];

        if(block->status == ALLOCATED_BLOCK && block->allocationId == allocationId){
            return index;
        }
    }

    return NO_BLOCK;
}

void mergeFreeBlocks(MemoryManager *manager){
    int index = 0;

    while(index + 1 < manager->blockCount){
        MemoryBlock *current = &manager->blocks[index];
        MemoryBlock *next = &manager->blocks[index + 1];

        if(current->status == FREE_BLOCK && next->status == FREE_BLOCK){
            current->size += next->size;

            memmove(&manager->blocks[index + 1],
                    &manager->blocks[index + 2],
                    (manager->blockCount - index - 2) * sizeof(MemoryBlock));

            manager->blockCount--;
        }
        else{
            index++;
        }
    }
}

void insertAllocation(MemoryManager *manager, int index, int requestedSize, int allocationId){
    int originalSize = manager->blocks[index].size;
    int originalOffset = manager->blocks[index].startingOffset;

    if(originalSize > requestedSize){
        memmove(&manager->blocks[index + 2],
                &manager->blocks[index + 1],
                (manager->blockCount - index - 1) * sizeof(MemoryBlock));

        manager->blocks[index + 1] = (MemoryBlock){
            originalOffset + requestedSize,
            originalSize - requestedSize,
            0,
            FREE_BLOCK
        };

        manager->blockCount++;
    }

    manager->blocks[index] = (MemoryBlock){
        originalOffset,
        requestedSize,
        allocationId,
        ALLOCATED_BLOCK
    };
}

int detectFragmentation(const MemoryManager *manager, int requestedSize){
    int totalFree = 0;
    int largestFree = 0;

    for(int index = 0; index < manager->blockCount; index++){
        MemoryBlock *block = &manager->blocks[index];

        if(block->status == FREE_BLOCK){
            totalFree += block->size;

            if(block->size > largestFree){
                largestFree = block->size;
            }
        }
    }

    if(totalFree >= requestedSize && largestFree < requestedSize){
        printf("External fragmentation detected.\n");
        printf("Requested size: %d bytes\n", requestedSize);
        printf("Total free memory: %d bytes\n", totalFree);
        printf("Individual free blocks:\n");

        for(int index = 0; index < manager->blockCount; index++){
            if(manager->blocks[index].status == FREE_BLOCK){
                printf("%d bytes\n", manager->blocks[index].size);
            }
        }

        return 1;
    }

    return 0;
}

void reportAllocationFailure(const MemoryManager *manager, int requestedSize){
    printf("Allocation failed: no contiguous free block is large enough.\n");
    detectFragmentation(manager, requestedSize);
}

void allocateMemory(MemoryManager *manager, int requestedSize){
    int index;
    int allocationId;

    if(requestedSize < MIN_REQUEST_SIZE || requestedSize > manager->poolSize){
        printf("Invalid allocation size.\n");
        return;
    }

    if(manager->idExhausted){
        printf("Allocation ID limit reached.\n");
        return;
    }

    index = findFreeBlock(manager, requestedSize);

    if(index == NO_BLOCK){
        reportAllocationFailure(manager, requestedSize);
        return;
    }

    if(manager->blocks[index].size > requestedSize &&
       !ensureCapacity(manager, manager->blockCount + 1)){
        printf("Metadata allocation failed.\n");
        return;
    }

    allocationId = manager->nextId;

    insertAllocation(manager, index, requestedSize, allocationId);

    if(manager->nextId == MAX_INTEGER_VALUE){
        manager->idExhausted = 1;
    }
    else{
        manager->nextId++;
    }

    printf("Allocation successful. ID: %d, Start: %d, Size: %d\n",
           allocationId, manager->blocks[index].startingOffset, requestedSize);
}

void releaseMemory(MemoryManager *manager, int allocationId){
    int index = findAllocation(manager, allocationId);

    if(index == NO_BLOCK){
        printf("Invalid or already released allocation ID.\n");
        return;
    }

    manager->blocks[index].status = FREE_BLOCK;
    manager->blocks[index].allocationId = 0;

    mergeFreeBlocks(manager);

    printf("Allocation released successfully.\n");
}

int resizeInPlace(MemoryManager *manager, int index, int newSize){
    int oldSize = manager->blocks[index].size;
    int oldOffset = manager->blocks[index].startingOffset;

    if(newSize < oldSize){
        int releasedSize = oldSize - newSize;

        if(index + 1 < manager->blockCount &&
           manager->blocks[index + 1].status == FREE_BLOCK){
            manager->blocks[index].size = newSize;
            manager->blocks[index + 1].startingOffset = oldOffset + newSize;
            manager->blocks[index + 1].size += releasedSize;
        }
        else{
            if(!ensureCapacity(manager, manager->blockCount + 1)){
                return -1;
            }

            memmove(&manager->blocks[index + 2],
                    &manager->blocks[index + 1],
                    (manager->blockCount - index - 1) * sizeof(MemoryBlock));

            manager->blocks[index].size = newSize;

            manager->blocks[index + 1] = (MemoryBlock){
                oldOffset + newSize,
                releasedSize,
                0,
                FREE_BLOCK
            };

            manager->blockCount++;
        }

        mergeFreeBlocks(manager);
        return 1;
    }

    if(newSize > oldSize){
        int growth = newSize - oldSize;

        if(index + 1 >= manager->blockCount ||
           manager->blocks[index + 1].status != FREE_BLOCK ||
           manager->blocks[index + 1].size < growth){
            return 0;
        }

        if(manager->blocks[index + 1].size == growth){
            memmove(&manager->blocks[index + 1],
                    &manager->blocks[index + 2],
                    (manager->blockCount - index - 2) * sizeof(MemoryBlock));

            manager->blockCount--;
        }
        else{
            manager->blocks[index + 1].startingOffset += growth;
            manager->blocks[index + 1].size -= growth;
        }

        manager->blocks[index].size = newSize;
    }

    return 1;
}

int resizeByMoving(MemoryManager *manager, int index, int newSize, int allocationId){
    int destination = findFreeBlock(manager, newSize);
    int oldOffset = manager->blocks[index].startingOffset;
    int oldSize = manager->blocks[index].size;
    int newOffset;

    if(destination == NO_BLOCK){
        reportAllocationFailure(manager, newSize);
        printf("Original allocation remains unchanged.\n");
        return 0;
    }

    if(manager->blocks[destination].size > newSize &&
       !ensureCapacity(manager, manager->blockCount + 1)){
        printf("Metadata allocation failed. Original allocation unchanged.\n");
        return 0;
    }

    newOffset = manager->blocks[destination].startingOffset;

    memmove(manager->memoryPool + newOffset,
            manager->memoryPool + oldOffset,
            oldSize);

    insertAllocation(manager, destination, newSize, allocationId);

    for(int current = 0; current < manager->blockCount; current++){
        MemoryBlock *block = &manager->blocks[current];

        if(block->status == ALLOCATED_BLOCK &&
           block->allocationId == allocationId &&
           block->startingOffset == oldOffset){
            block->status = FREE_BLOCK;
            block->allocationId = 0;
            break;
        }
    }

    mergeFreeBlocks(manager);

    return 1;
}

void resizeMemory(MemoryManager *manager, int allocationId, int newSize){
    int index = findAllocation(manager, allocationId);
    int result;

    if(index == NO_BLOCK){
        printf("Invalid allocation ID.\n");
        return;
    }

    if(newSize < MIN_REQUEST_SIZE || newSize > manager->poolSize){
        printf("Invalid resize size.\n");
        return;
    }

    if(newSize == manager->blocks[index].size){
        printf("Allocation size is unchanged.\n");
        return;
    }

    result = resizeInPlace(manager, index, newSize);

    if(result == 1){
        printf("Allocation resized in place successfully.\n");
        return;
    }

    if(result == -1){
        printf("Metadata allocation failed. Original allocation unchanged.\n");
        return;
    }

    if(resizeByMoving(manager, index, newSize, allocationId)){
        printf("Allocation moved and resized successfully.\n");
    }
}

void displayMemoryMap(const MemoryManager *manager){
    printf("\nMemory Map (%d bytes):\n", manager->poolSize);

    for(int index = 0; index < manager->blockCount; index++){
        const MemoryBlock *block = &manager->blocks[index];

        if(block->status == ALLOCATED_BLOCK){
            printf("[Allocated #%d: %d bytes, Start: %d]\n",
                   block->allocationId, block->size, block->startingOffset);
        }
        else{
            printf("[Free: %d bytes, Start: %d]\n",
                   block->size, block->startingOffset);
        }
    }
}

void displayStatistics(const MemoryManager *manager){
    int allocatedBytes = 0;
    int freeBytes = 0;
    int allocatedCount = 0;
    int freeCount = 0;
    int largestFree = 0;

    for(int index = 0; index < manager->blockCount; index++){
        const MemoryBlock *block = &manager->blocks[index];

        if(block->status == ALLOCATED_BLOCK){
            allocatedBytes += block->size;
            allocatedCount++;
        }
        else{
            freeBytes += block->size;
            freeCount++;

            if(block->size > largestFree){
                largestFree = block->size;
            }
        }
    }

    printf("Total memory: %d bytes\n", manager->poolSize);
    printf("Allocated: %d bytes\n", allocatedBytes);
    printf("Free: %d bytes\n", freeBytes);
    printf("Allocated blocks: %d\n", allocatedCount);
    printf("Free blocks: %d\n", freeCount);
    printf("Largest free block: %d bytes\n", largestFree);
    printf("Strategy: %s\n", getStrategyName(manager->strategy));
}

void changeStrategy(MemoryManager *manager){
    int choice;

    printf("\n1. First Fit\n");
    printf("2. Best Fit\n");
    printf("3. Worst Fit\n");

    if(!getValidInput("Select strategy: ", FIRST_FIT, WORST_FIT, &choice)){
        return;
    }

    manager->strategy = (FitType)choice;

    printf("Strategy changed to %s.\n", getStrategyName(manager->strategy));
}

void displayMenu(void){
    printf("\nMenu:\n");
    printf("1. Allocate memory\n");
    printf("2. Release memory\n");
    printf("3. Resize allocation\n");
    printf("4. Display memory map\n");
    printf("5. Display memory statistics\n");
    printf("6. Change allocation strategy\n");
    printf("7. Exit\n");
}

void processMenu(MemoryManager *manager){
    int choice;
    int requestedSize;
    int allocationId;
    int running = 1;

    displayMenu();
    while(running){

        if(!getValidInput("\nChoice: ", 1, 7, &choice)){
            break;
        }

        switch(choice){
            case 1:
                if(!getValidInput("Requested size: ", MIN_REQUEST_SIZE, manager->poolSize, &requestedSize)){
                    return;
                }

                allocateMemory(manager, requestedSize);
                break;

            case 2:
                if(!getValidInput("Allocation ID: ", MIN_REQUEST_SIZE, MAX_INTEGER_VALUE, &allocationId)){
                    return;
                }

                releaseMemory(manager, allocationId);
                break;

            case 3:
                if(!getValidInput("Allocation ID: ", MIN_REQUEST_SIZE, MAX_INTEGER_VALUE, &allocationId)){
                    return;
                }

                if(!getValidInput("New allocation size: ", MIN_REQUEST_SIZE, manager->poolSize, &requestedSize)){
                    return;
                }

                resizeMemory(manager, allocationId, requestedSize);
                break;

            case 4:
                displayMemoryMap(manager);
                break;

            case 5:
                displayStatistics(manager);
                break;

            case 6:
                changeStrategy(manager);
                break;

            case 7:
                running = 0;
                printf("Exiting Memory Manager.\n");
                break;
        }
    }
}

int main(void){
    MemoryManager manager = {0};
    int poolSize;
    int strategy;

    if(!getValidInput("Enter memory pool size: ", MIN_POOL_SIZE, MAX_POOL_SIZE, &poolSize)){
        return 0;
    }

    if(!initializeManager(&manager, poolSize)){
        printf("Memory initialization failed.\n");
        return 1;
    }

    if(!getValidInput("Select strategy (1. First Fit, 2. Best Fit, 3. Worst Fit): ", FIRST_FIT, WORST_FIT, &strategy)){
        destroyManager(&manager);
        return 0;
    }

    manager.strategy = (FitType)strategy;

    printf("Strategy set to %s.\n", getStrategyName(manager.strategy));

    processMenu(&manager);
    destroyManager(&manager);

    return 0;
}
