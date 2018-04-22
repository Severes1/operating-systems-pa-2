/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	filesystem.c
 * @brief 	Implementation of the core file system funcionalities and auxiliary functions.
 * @date	01/03/2017
 */

#include "include/crc.h"			// Headers for the CRC functionality
#include "include/filesystem.h"		// Headers for the core functionality
#include "include/metadata.h"		// Type and structure declaration of the file system
#include "include/auxiliary.h"		// Headers for auxiliary functions
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#define MAX_NUMBER_OF_FILES 10
OFT_Entry * OPEN_FILE_TABLE[MAX_NUMBER_OF_FILES] = {0}; // Pointers to structures OFT_Entry

int INODE_START = -1;
int DATA_BLOCK_START = -1;

/*
 * @brief 	Generates the proper file system structure in a storage device, as designed by the student.
 * @return 	0 if success, -1 otherwise.
 */
int mkFS(long deviceSize)
{
    SuperBlock superblock;
    init_superblock(&superblock, deviceSize); 
    
    char buffer[BLOCK_SIZE] = {0};

    /* Write zeroes to the allocation bitmap blocks  */
    bwrite(DEVICE_IMAGE, 1, buffer);
    bwrite(DEVICE_IMAGE, 2, buffer);
   
    /* Write the SuperBlock to the disk */ 
    memcpy(buffer, (void *) &superblock, sizeof(SuperBlock));
    bwrite(DEVICE_IMAGE, 0, buffer);

	return 0;
}

/*
 * @brief 	Mounts a file system in the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int mountFS(void)
{

    SuperBlock sblock = load_superblock();

    INODE_START = 3;
    printf("Mounting the file system!\n");
    DATA_BLOCK_START = 3 + sblock.max_inodes;
    printf("%d\n", DATA_BLOCK_START);

    return 0;
}

/*
 * @brief 	Unmounts the file system from the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int unmountFS(void)
{
	return -1;
}

/*
 * @brief	Creates a new file, provided it it doesn't exist in the file system.
 * @return	0 if success, -1 if the file already exists, -2 in case of error.
 */
int createFile(char *fileName)
{
    // Check filename
    if (strlen(fileName) > MAX_FILENAME) {
        return -2;
    }
    SuperBlock sblock = load_superblock();
    if (get_inode_index(&sblock, fileName) != -1) {
        return -1;
    }
    // Update INode allocation
    int inode_index = allocate_inode();
    if (inode_index == -1) {
        return -1;
    } 
    // Create INode
    INode inode;
    // Init INode
    inode.size = 0;
    inode.num_blocks = 0;
    // Write INode to disk
    bwrite(DEVICE_IMAGE, INODE_START + inode_index, (char*) &inode);
    // Update SuperBlock
    sblock.num_inodes_in_use++;
    memcpy(&(sblock.filenames[inode_index].name), fileName, strlen(fileName));
    sblock.filenames[inode_index].index = inode_index;
    bwrite(DEVICE_IMAGE, 0, (char *) &sblock);
    return 0;
}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName)
{
    // Load Superblock
    SuperBlock sblock = load_superblock();
    // Check that file exists
    long inode_index = get_inode_index(&sblock, fileName);
    if (inode_index == -1) {
        return -1;
    }
    // Prepare bitmap
    char bitmap[BLOCK_SIZE];
    // Update data allocation
    bread(DEVICE_IMAGE, 2, bitmap);
    INode inode;
    bread(DEVICE_IMAGE, INODE_START + inode_index, (char *) &inode);
    for (int i = 0; i < inode.num_blocks; i++) {
        long data_index = inode.blocks[i];
        bitmap_setbit(bitmap, data_index, 0);
    }
    bwrite(DEVICE_IMAGE, 2, bitmap); 
    // Update inode allocation
    bread(DEVICE_IMAGE, 1, bitmap);
    bitmap_setbit(bitmap, inode_index, 0);
    bwrite(DEVICE_IMAGE, 1, bitmap);
    // Update superblock
    sblock.filenames[inode_index].index = -1;
    sblock.num_inodes_in_use--;
    bwrite(DEVICE_IMAGE, 0, (char *) &sblock);
    return 0;
}

/*
 * @brief	Opens an existing file.
 * @return	The file descriptor if possible, -1 if file does not exist, -2 in case of error..
 */
int openFile(char *fileName)
{
    // TODO Check for file integrity
    SuperBlock sblock = load_superblock();
    int inode_index = get_inode_index(&sblock, fileName);
    if (inode_index == -1) {
        return -1;
    }
    
    // Make a file descriptor and return it
    int i;
    for (i = 0; i < MAX_NUMBER_OF_FILES; i++) {
        if (OPEN_FILE_TABLE[i] == NULL) {
            break;    
        }
    }

    if (i == MAX_NUMBER_OF_FILES) {
        return -1;    
    }

    OPEN_FILE_TABLE[i] = malloc(sizeof(OFT_Entry));
    
    OPEN_FILE_TABLE[i]->fd     = i;
    OPEN_FILE_TABLE[i]->inode  = inode_index;
    OPEN_FILE_TABLE[i]->offset = 0;

	return i;
}

/*
 * @brief	Closes a file.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{
    if (fileDescriptor < 0 || OPEN_FILE_TABLE[fileDescriptor] == NULL) {
        return -1;    
    }
    free(OPEN_FILE_TABLE[fileDescriptor]);
    OPEN_FILE_TABLE[fileDescriptor] = NULL;
	return 0;
}

/*
 * @brief	Reads a number of bytes from a file and stores them in a buffer.
 * @return	Number of bytes properly read, -1 in case of error.
 */
int readFile(int fileDescriptor, void *buffer, int numBytes)
{
    if (fileDescriptor < 0 || OPEN_FILE_TABLE[fileDescriptor] == NULL) {
        return -1;
    }
    OFT_Entry oft = *(OPEN_FILE_TABLE[fileDescriptor]);
   
    INode inode;
    bread(DEVICE_IMAGE, INODE_START + oft.inode, (char *) &inode);

    int bytes_read = 0;

	long current_block = oft.offset / BLOCK_SIZE;
    
    char temp_buffer[BLOCK_SIZE] = {0};
    
    if (numBytes + oft.offset > inode.size) {
        numBytes = inode.size - oft.offset;    
    }

    while (bytes_read < numBytes) {
        int block_index = inode.blocks[current_block];
        bread(DEVICE_IMAGE, DATA_BLOCK_START + block_index, temp_buffer);
        memcpy(buffer + bytes_read, temp_buffer, numBytes - bytes_read < BLOCK_SIZE ? numBytes - bytes_read : BLOCK_SIZE);
        bytes_read += BLOCK_SIZE;
        current_block++;
    }
     
    return numBytes;
}

/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void *buffer, int numBytes)
{
    printf("%d\n", DATA_BLOCK_START);
    // Check that file is open
    if (fileDescriptor < 0 || OPEN_FILE_TABLE[fileDescriptor] == NULL) {
        return -1;   
    }
    // Load entry 
    OFT_Entry oft = *(OPEN_FILE_TABLE[fileDescriptor]);
    INode inode;
    bread(DEVICE_IMAGE, INODE_START + oft.inode, (char *) &inode);
    // Determine which block to start writing to
    long current_block = oft.offset / BLOCK_SIZE;
    // Write from buffer to file numBytes. If this is past the end of the file, extend the file. Update INode and Data
    int bytes_written = 0;
    char temp_buffer[BLOCK_SIZE] = {0};
    int block_index;
    while (bytes_written < numBytes) {
        if (current_block >= inode.num_blocks) {
            // Allocate a new block
            block_index = allocate_data_block(); 
            inode.num_blocks++;
            inode.blocks[current_block] = block_index;
            inode.size += numBytes - bytes_written < BLOCK_SIZE ? numBytes - bytes_written : BLOCK_SIZE;
            char temp_temp_buffer[BLOCK_SIZE];
            bwrite(DEVICE_IMAGE, DATA_BLOCK_START + block_index, temp_temp_buffer);
        } else {
            block_index = inode.blocks[current_block];
        }
        if (numBytes - bytes_written < BLOCK_SIZE) {
            bread(DEVICE_IMAGE, DATA_BLOCK_START + block_index, temp_buffer);
        }
        memcpy(temp_buffer, buffer, numBytes - bytes_written < BLOCK_SIZE ? numBytes - bytes_written : BLOCK_SIZE);
        printf("Writing: %s in block: %d\n", temp_buffer, block_index);
        bwrite(DEVICE_IMAGE, DATA_BLOCK_START + block_index, temp_buffer);
        bytes_written += BLOCK_SIZE;
        current_block++;
    }

    // Write INODE
    bwrite(DEVICE_IMAGE, INODE_START + oft.inode, (char *) &inode);

    // Update CRC
    // TODO
    // Return number of bytes properly written.
	return numBytes;
}


/*
 * @brief	Modifies the position of the seek pointer of a file.
 * @return	0 if success, -1 otherwise.
 */
int lseekFile(int fileDescriptor, long offset, int whence)
{

    if (fileDescriptor < 0 || OPEN_FILE_TABLE[fileDescriptor] == NULL) {
        return -1;
    }

    OFT_Entry oft = *(OPEN_FILE_TABLE[fileDescriptor]);

    long new_seek_pos;
    INode inode;
    switch (whence) {
        case FS_SEEK_CUR:
            new_seek_pos = oft.offset;
            break;
        case FS_SEEK_BEGIN:
            new_seek_pos = 0; 
            break;
        case FS_SEEK_END:
            bread(DEVICE_IMAGE, INODE_START + oft.inode, (char *) &inode);
            new_seek_pos = inode.size;
            break;
        default:
            return -1;
    }

    return new_seek_pos + offset;
}

/*
 * @brief 	Verifies the integrity of the file system metadata.
 * @return 	0 if the file system is correct, -1 if the file system is corrupted, -2 in case of error.
 */
int checkFS(void)
{
	return -2;
}

/*
 * @brief 	Verifies the integrity of a file.
 * @return 	0 if the file is correct, -1 if the file is corrupted, -2 in case of error.
 */
int checkFile(char *fileName)
{
	return -2;
}

void init_superblock(SuperBlock * sblock, long disk_size) {
    long max_file_blocks = MAX_FILE_SIZE / BLOCK_SIZE;
    long num_blocks_on_disk = disk_size / BLOCK_SIZE; 
    long num_crc_blocks = (2 * num_blocks_on_disk) / (BLOCK_SIZE + 2); // Assuming using CRC16
    long max_number_of_files = (num_blocks_on_disk - num_crc_blocks - 3) / (max_file_blocks + 1);
    sblock->num_crc_blocks = num_crc_blocks;
    sblock->max_inodes= max_number_of_files;
    sblock->max_data_blocks = num_blocks_on_disk - 3 - max_number_of_files;
}

int first_zero(char * bitmap, int length) {
    int i;
    for (i = 0; i < length; i++) {
        if(bitmap_getbit(bitmap, i) == 0) {
            break;
        }
    }
    if (i == length) {
        return -1;
    }
    return i;
}

/* Updates the inode allocation bitmap on the disk
   Returns the index of the first free inode block */
int allocate_inode() {
    char bitmap[BLOCK_SIZE];
    bread(DEVICE_IMAGE, 1, bitmap);
    int i = first_zero(bitmap, BLOCK_SIZE);
    bitmap_setbit(bitmap, i, 1);
    bwrite(DEVICE_IMAGE, 1, bitmap);
    return i;
}

int allocate_data_block() {
    char bitmap[BLOCK_SIZE];
    bread(DEVICE_IMAGE, 2, bitmap);
    int i = first_zero(bitmap, BLOCK_SIZE);
    bitmap_setbit(bitmap, i, 1);
    bwrite(DEVICE_IMAGE, 2, bitmap);
    return i;
}

SuperBlock load_superblock() {
    char buffer[BLOCK_SIZE] = {0};
    bread(DEVICE_IMAGE, 0, buffer);
    SuperBlock sblock = *(SuperBlock *) buffer;
    return sblock;
}

/* Returns index of file if it exists, -1 otherwise */
int get_inode_index(SuperBlock * sblock, char * fileName) {
    int i, count;
    while (count < sblock->num_inodes_in_use) {
        if (sblock->filenames[i].index == -1) {
            i++;
            continue; // Skip deleted files
        } else if (strcmp(fileName, (char *) &(sblock->filenames[i].name)) == 0) {
            return sblock->filenames[i].index;
        }
        count++;
        i++;
    }
    return -1;
}
