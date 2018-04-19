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
	return -1;
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
    for (int i = 0; i < sblock.num_inodes; i++) {
        if (strcmp(fileName, (char *) &(sblock.filenames[i])) == 0) {
            return -1;
        }
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
    bwrite(DEVICE_IMAGE, 3 + inode_index, (char*) &inode);
    // Update SuperBlock
    sblock.num_inodes++;
    memcpy(&sblock.filenames[inode_index], fileName, strlen(fileName));
    bwrite(DEVICE_IMAGE, 0, (char *) &sblock);
    return 0;
}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName)
{
	return -2;
}

/*
 * @brief	Opens an existing file.
 * @return	The file descriptor if possible, -1 if file does not exist, -2 in case of error..
 */
int openFile(char *fileName)
{
	return -2;
}

/*
 * @brief	Closes a file.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{
	return -1;
}

/*
 * @brief	Reads a number of bytes from a file and stores them in a buffer.
 * @return	Number of bytes properly read, -1 in case of error.
 */
int readFile(int fileDescriptor, void *buffer, int numBytes)
{
	return -1;
}

/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void *buffer, int numBytes)
{
	return -1;
}


/*
 * @brief	Modifies the position of the seek pointer of a file.
 * @return	0 if succes, -1 otherwise.
 */
int lseekFile(int fileDescriptor, long offset, int whence)
{
	return -1;
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
    long max_number_of_files = (num_blocks_on_disk - 3) / (max_file_blocks + 1);
    sblock->num_inodes = max_number_of_files;
    sblock->num_data_blocks = num_blocks_on_disk - 3 - max_number_of_files;
}

int allocate_inode() {
    char bitmap[BLOCK_SIZE];
    bread(DEVICE_IMAGE, 1, bitmap);
    int i;
    for (i = 0; i < BLOCK_SIZE; i++) {
        if(bitmap_getbit(bitmap, i) == 0) {
            return i;
        }
    }
    return -1;
}

SuperBlock load_superblock() {
    char buffer[BLOCK_SIZE] = {0};
    bread(DEVICE_IMAGE, 0, buffer);
    SuperBlock sblock = *(SuperBlock *) buffer;
    return sblock;
}
