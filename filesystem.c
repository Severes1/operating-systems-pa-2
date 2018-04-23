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
    bwrite_with_crc(DEVICE_IMAGE, 1, buffer);
    bwrite_with_crc(DEVICE_IMAGE, 2, buffer);
   
    /* Write the SuperBlock to the disk */ 
    memcpy(buffer, (void *) &superblock, sizeof(SuperBlock));
    bwrite_with_crc(DEVICE_IMAGE, 0, buffer);

	return 0;
}

/*
 * @brief 	Mounts a file system in the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int mountFS(void)
{

    SuperBlock sblock = load_superblock();
    INODE_START = 3 + sblock.num_crc_blocks;
    DATA_BLOCK_START = 3 + sblock.max_inodes;

    return 0;
}

/*
 * @brief 	Unmounts the file system from the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int unmountFS(void)
{
    if (INODE_START == -1) {
        return -1;
    }
	INODE_START = -1;
    DATA_BLOCK_START = -1;
    return 0;
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
    bwrite_with_crc(DEVICE_IMAGE, INODE_START + inode_index, (char*) &inode);
    // Update SuperBlock
    sblock.num_inodes_in_use++;
    memcpy(&(sblock.filenames[inode_index].name), fileName, strlen(fileName));
    sblock.filenames[inode_index].index = inode_index;
    bwrite_with_crc(DEVICE_IMAGE, 0, (char *) &sblock);
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
    bwrite_with_crc(DEVICE_IMAGE, 2, bitmap); 
    // Update inode allocation
    bread(DEVICE_IMAGE, 1, bitmap);
    bitmap_setbit(bitmap, inode_index, 0);
    bwrite_with_crc(DEVICE_IMAGE, 1, bitmap);
    // Update superblock
    sblock.filenames[inode_index].index = -1;
    sblock.num_inodes_in_use--;
    bwrite_with_crc(DEVICE_IMAGE, 0, (char *) &sblock);
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
    
    oft.offset += numBytes;
    return numBytes;
}

/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void *buffer, int numBytes)
{
    // Check that file is open
    if (fileDescriptor < 0 || OPEN_FILE_TABLE[fileDescriptor] == NULL) {
        return -1;   
    }
    // Load entry 
    OFT_Entry * oft = OPEN_FILE_TABLE[fileDescriptor];
    INode inode;
    bread(DEVICE_IMAGE, INODE_START + oft->inode, (char *) &inode);
    // Determine which block to start writing to
    long current_block = oft->offset / BLOCK_SIZE;
    printf("Offset: %ld Current_block: %ld\n", oft->offset, current_block);
    // Write from buffer to file numBytes. If this is past the end of the file, extend the file. Update INode and Data
    int bytes_written = 0;
    char temp_buffer[BLOCK_SIZE] = {0};
    int block_index;
    while (bytes_written < numBytes) {
        int bytes_this_loop = numBytes - bytes_written < BLOCK_SIZE ? numBytes - bytes_written : BLOCK_SIZE;
        if (current_block >= inode.num_blocks) {
            // Allocate a new block
            block_index = allocate_data_block(); 
            inode.num_blocks++;
            inode.blocks[current_block] = block_index;
            inode.size += bytes_this_loop; 
            memset(temp_buffer, 0, BLOCK_SIZE); 
        } else {
            block_index = inode.blocks[current_block];
             if (numBytes - bytes_written < BLOCK_SIZE) {
                 bread(DEVICE_IMAGE, DATA_BLOCK_START + block_index, temp_buffer);
             }
        }
       
        memcpy(temp_buffer, buffer + bytes_written, bytes_this_loop);
        bwrite_with_crc(DEVICE_IMAGE, DATA_BLOCK_START + block_index, temp_buffer);
        bytes_written += BLOCK_SIZE;
        current_block++;
    }

    // Write INODE
    bwrite_with_crc(DEVICE_IMAGE, INODE_START + oft->inode, (char *) &inode);

    // Update CRC
    // TODO
    // Return number of bytes properly written.
    oft->offset += numBytes;
	printf("numBytes: %d, offset: %ld\n", numBytes, oft->offset);
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

    OFT_Entry * oft = OPEN_FILE_TABLE[fileDescriptor];

    long new_seek_pos;
    INode inode;
    switch (whence) {
        case FS_SEEK_CUR:
            new_seek_pos = oft->offset;
            break;
        case FS_SEEK_BEGIN:
            new_seek_pos = 0; 
            break;
        case FS_SEEK_END:
            bread(DEVICE_IMAGE, INODE_START + oft->inode, (char *) &inode);
            new_seek_pos = inode.size;
            break;
        default:
            return -1;
    }
    
    oft->offset = new_seek_pos + offset;
    return oft->offset;
}

/*
 * @brief 	Verifies the integrity of the file system metadata.
 * @return 	0 if the file system is correct, -1 if the file system is corrupted, -2 in case of error.
 */
int checkFS(void)
{
    // Check superblock
    int ret;
    if ((ret = check_crc(0)) != 0) {
        return ret;    
    }
    
    // Check Allocation blocks
    if ((ret = check_crc(1)) != 0) {
        return ret;    
    }
    if ((ret = check_crc(2)) != 0) {
        return ret;    
    } 
   
    // Check INodes
    for (int i = INODE_START; i < DATA_BLOCK_START; i++) {
        if ((ret = check_crc(i)) != 0) {
            return ret;    
        } 
    }
	return 0;
}

/*
 * @brief 	Verifies the integrity of a file.
 * @return 	0 if the file is correct, -1 if the file is corrupted, -2 in case of error.
 */
int checkFile(char *fileName)
{
    // Read all of the blocks, and check that the CRC matches
    SuperBlock sblock = load_superblock();

    int inode_index = get_inode_index(&sblock, fileName);

    INode inode;
    bread(DEVICE_IMAGE, INODE_START + inode_index, (char *) &inode);

    int i;
   
    for (i = 0; i < inode.num_blocks; i++) {
        int blockNumber = DATA_BLOCK_START + inode.blocks[i];
        int ret;
        if ((ret = check_crc(blockNumber)) != 0) {
            return ret; 
        }
    } 
	return 0;
}

/* Calculates how many files can fit in  given disk_size,
   and initializes the superblock with the corresponding values */
void init_superblock(SuperBlock * sblock, long disk_size) {
    long max_file_blocks = MAX_FILE_SIZE / BLOCK_SIZE;
    long num_blocks_on_disk = disk_size / BLOCK_SIZE; 
    long num_crc_blocks = (2 * num_blocks_on_disk) / (BLOCK_SIZE + 2); // Assuming using CRC16
    long max_number_of_files = (num_blocks_on_disk - num_crc_blocks - 3) / (max_file_blocks + 1);
    sblock->num_crc_blocks = num_crc_blocks;
    sblock->max_inodes= max_number_of_files;
    sblock->max_data_blocks = num_blocks_on_disk - 3 - max_number_of_files;
}

/* Finds the First zero in a bitmap. Used by both allocate_ functions */
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
    bwrite_with_crc(DEVICE_IMAGE, 1, bitmap);
    return i;
}

/* Updates the data block allocation bitmap on the disk
   Returns the index of the first free data block */
int allocate_data_block() {
    char bitmap[BLOCK_SIZE];
    bread(DEVICE_IMAGE, 2, bitmap);
    int i = first_zero(bitmap, BLOCK_SIZE);
    bitmap_setbit(bitmap, i, 1);
    bwrite_with_crc(DEVICE_IMAGE, 2, bitmap);
    return i;
}

/* 
 * Helper function to load the superblock.
 * This could be used to cache the superblock in memory
 */
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

/* Returns 0 on success and -1 for failed write and -2 for failed CRC */
int bwrite_with_crc(char *deviceName, int blockNumber, char *buffer) {
    // Perform the write operation
    if (bwrite(deviceName, blockNumber, buffer) != 0) {
        return -1;
    }
    // Locate CRC hash
    long crc_block = (blockNumber * 2)/ BLOCK_SIZE;
    long index = (blockNumber * 2) % BLOCK_SIZE;

    // Read previous CRC hash
    uint16_t crc_buffer[BLOCK_SIZE / 2];
    bread(DEVICE_IMAGE, 3 + crc_block, (char *) crc_buffer);
    // Compute CRC hash
    uint16_t new_crc = CRC16((unsigned char *) buffer, BLOCK_SIZE, 0);
    
    printf("Writing block: %d\n", blockNumber);

    // Write CRC hash
    crc_buffer[index / 2] = new_crc;
    if (bwrite(DEVICE_IMAGE, 3 + crc_block, (char *) crc_buffer) != 0) {
        return -2;
    }
    return 0;
}

/* Returns 0 if data is ok, -1 if it is corrupt, -2 otherwise */
int check_crc(int blockNumber) {
    char data_buffer[BLOCK_SIZE];
    int loaded_crc = -1;
    uint16_t crc_buffer[BLOCK_SIZE / 2];
    if (bread(DEVICE_IMAGE, blockNumber, data_buffer) == -1) {
        return -2;   
    }

    uint16_t new_crc = CRC16((unsigned char *) data_buffer, BLOCK_SIZE, 0);

    long crc_block = (blockNumber * 2)/ BLOCK_SIZE;
    long index = (blockNumber * 2) % BLOCK_SIZE;
    if (crc_block != loaded_crc) {
        if (bread(DEVICE_IMAGE, 3 + crc_block, (char *) crc_buffer) == -1) {
            return -2;    
        }
        loaded_crc = crc_block;
    }

    uint16_t prev_crc = crc_buffer[index / 2];

    if (prev_crc != new_crc) {
        return -1;    
    }

    return 0;

}
