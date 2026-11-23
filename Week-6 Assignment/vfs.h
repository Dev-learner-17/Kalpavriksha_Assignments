#ifndef VFS_H
#define VFS_H
#include <stddef.h>
#include <stdbool.h>

enum { 
BLOCK_SIZE=512,
MAX_BLOCK_COUNT=5000,
MAX_NAME_LENGTH=50,
PERCENTAGE_SCALE=100,
INPUT_BUFFER_SIZE=128
};

typedef enum { 
NODE_FILE,
NODE_DIRECTORY
} NodeType;

typedef enum { STATUS_SUCCESS,
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
char name[MAX_NAME_LENGTH+1];
NodeType type;
struct FileNode *parent,*next,*previous,*children;
int *blockPointers;
size_t blockCount,contentSize;
} FileNode;

typedef struct FreeBlock { 
int index;
struct FreeBlock *next,*previous;
} FreeBlock;

typedef struct { 
unsigned char (*virtualDisk)[BLOCK_SIZE];
FreeBlock *freeBlockNodes,*freeHead,*freeTail;
FileNode *root,*currentDirectory;
int totalBlocks,freeBlockCount;
} VirtualFileSystem;


bool initializeFileSystem(VirtualFileSystem *, int);
void destroyFileSystem(VirtualFileSystem *);
bool processCommand(VirtualFileSystem *, char *);
char *readCommandLine(void);
int getValidInput(const char *, int, int, int *);
void printCommandPrompt(const FileNode *);
Status createEntry(VirtualFileSystem *, const char *, NodeType);
Status writeFile(VirtualFileSystem *, const char *, const char *);
Status readFile(VirtualFileSystem *, const char *);
Status deleteFile(VirtualFileSystem *, const char *);
Status removeDirectory(VirtualFileSystem *, const char *);
Status changeDirectory(VirtualFileSystem *, const char *);
void listDirectory(const FileNode *);
void printWorkingDirectory(const FileNode *);
void displayDiskUsage(const VirtualFileSystem *);
char *extractToken(char **);
char *extractWriteContent(char *);
#endif
