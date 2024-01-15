#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
// we need to make these file first
    //#include "storage_mgr.h"
    //#include "dberror.h"
#define PAGE_SIZE 4096

typedef struct {
    char *fileName; // Pointer to a string that holds the name of the file.
    int totalNumPages; // The total number of pages in the file.
    int curPagePos;  // The current page position in the file.
    FILE *filePointer;   // A pointer to a FILE object that represents the file in the filesystem.
} SM_FileHandle;

typedef char* SM_PageHandle; // A pointer to a File object that represents byte data of a page.

// Print name of Teammates
void initStorageManager(void) {
    printf("\n <---------------Initializing Storage Manager----------------->\n "
           "BY\n"
           "Prachi Kotadia (A20549927), "
           "Rishabh Rao (A205)\n,"
           "Ayush (A205)\n");
}

// Creates a file of PAGE_SIZE.
void createFile(char *fileName) {
    FILE *file = fopen(fileName, "w");
    if (file == NULL) {
        perror("Error creating file"); // Check if file opening failed.
        return;
    }
    // Set file size to PAGE_SIZE by writing a null character at PAGE_SIZE - 1.
    if (fseek(file, PAGE_SIZE - 1, SEEK_SET) != 0 || fputc('\0', file) == EOF) {
        perror("Error initializing file size");
        fclose(file); // Close file on error.
        return;
    }
    fclose(file); // Close file after successful creation.
}


// Opens a file and initializes its SM_FileHandle structure.
int openFile(char *fileName, SM_FileHandle *fileHandle) {
    fileHandle->filePointer = fopen(fileName, "r+");
    if (fileHandle->filePointer == NULL) {
        perror("Error opening file");
        return -1;
    }
    // Duplicate file name and store it in the file handle.
    fileHandle->fileName = strdup(fileName);
    if (fileHandle->fileName == NULL) { // Check memory allocation for file name.
        fclose(fileHandle->filePointer);
        perror("Error allocating memory");
        return -1;
    }

    // Set current page position to the beginning and calculate total number of pages.
    fileHandle->curPagePos = 0;
    if (fseek(fileHandle->filePointer, 0, SEEK_END) != 0 || (fileHandle->totalNumPages = ftell(fileHandle->filePointer) / PAGE_SIZE) < 0) {
        free(fileHandle->fileName);
        fclose(fileHandle->filePointer);
        perror("Error finding file size");
        return -1;
    }

    return 0;
}

// Closes the file and frees associated resources.
int closeFile(SM_FileHandle *fileHandle) {
    int status = 0;
    if (fclose(fileHandle->filePointer) != 0) {
        perror("Error closing file");
        status = -1;
    }
    free(fileHandle->fileName); // Free the dynamically allocated file name.
    return status;
}

// Deletes a file from the file system.
int destroyFile(char *fileName) {
    if (remove(fileName) == 0) {
        printf("File successfully deleted.\n");
        return 0;
    } else {
        perror("Error deleting file");
        return -1;
    }
}

// Writes data to a file.
int writeFile(SM_FileHandle *fileHandle, const char *data, size_t size) {
    fseek(fileHandle->filePointer, 0, SEEK_SET); // Set file pointer to the start of the file.
    // Write data to the file and check if the write operation was successful.
    return fwrite(data, sizeof(char), size, fileHandle->filePointer) == size ? 0 : -1;
}

// Reads data from a file.
int readFile(SM_FileHandle *fileHandle, char *buffer, size_t size) {
    fseek(fileHandle->filePointer, 0, SEEK_SET); // Set file pointer to the start of the file.
// Read data from the file into the buffer and check if the read operation was successful.
    return fread(buffer, sizeof(char), size, fileHandle->filePointer) == size ? 0 : -1;
}

int main() {
    initStorageManager(); // Initialize the storage manager.
    char *fileName = "assign1.bin";

    createFile(fileName); // Create a new file.


    SM_FileHandle fh;
    if (openFile(fileName, &fh) == 0) { // Open the file and check for success.
        printf("Opened file %s successfully.\n", fh.fileName);
        // Data to be written to the file.
        const char *dataToWrite = "Hello, World!";
        if (writeFile(&fh, dataToWrite, strlen(dataToWrite)) == 0) {
            printf("Data written successfully.\n");
        } else {
            printf("Failed to write data.\n");
        }

        char buffer[PAGE_SIZE];// Buffer to store read data.
        // Read data from the file into the buffer and check for success.
        if (readFile(&fh, buffer, strlen(dataToWrite)) == 0) {
            printf("Data read successfully: %s\n", buffer);
        } else {
            printf("Failed to read data.\n");
       
    }

    closeFile(&fh);
} else {
    printf("Failed to open file %s.\n", fileName);
}

destroyFile(fileName);

return 0;
}






// I guess we may write with or without the main and add the code below to the part of assignment_1 (2.3).

// Returns the current page position in the file.
int getBlockPos(SM_FileHandle *fileHandle) {
    return fileHandle->curPagePos;
}

// Reads the first block of the file.
int readFirstBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    return readBlock(0, fileHandle, memPage); // Calls readBlock for the first page (page number 0)
}

// Reads the last block of the file.
int readLastBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    return readBlock(fileHandle->totalNumPages - 1, fileHandle, memPage); // Calls readBlock for the last page.
}

// Reads the previous block relative to the current position.
int readPreviousBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    if (fileHandle->curPagePos <= 0) return -1;
    return readBlock(fileHandle->curPagePos - 1, fileHandle, memPage);
}

// Reads the current block.
int readCurrentBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    return readBlock(fileHandle->curPagePos, fileHandle, memPage);
}

// Reads the next block relative to the current position.
int readNextBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    if (fileHandle->curPagePos >= fileHandle->totalNumPages - 1) return -1;
    return readBlock(fileHandle->curPagePos + 1, fileHandle, memPage);
}

// Writes the current block to the file.
int writeCurrentBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    return writeBlock(fileHandle->curPagePos, fileHandle, memPage);
}

// Appends an empty block to the end of the file.
int appendEmptyBlock(SM_FileHandle *fileHandle) {
    fseek(fileHandle->filePointer, 0, SEEK_END); // Move file pointer to the end of the file.
    SM_PageHandle emptyBlock = (SM_PageHandle) calloc(PAGE_SIZE, sizeof(char)); // Allocate memory for an empty page.
    fwrite(emptyBlock, sizeof(char), PAGE_SIZE, fileHandle->filePointer); // Write the empty page to the file.
    free(emptyBlock); // Free the allocated memory.
    fileHandle->totalNumPages++; // Increment the total number of pages.
    return 0;
}

// Ensures the file has at least a specified number of pages.
int ensureCapacity(int numberOfPages, SM_FileHandle *fileHandle) {
    while (fileHandle->totalNumPages < numberOfPages) {
        appendEmptyBlock(fileHandle);
    }
    return 0;
}
