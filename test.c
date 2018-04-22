/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	test.c
 * @brief 	Implementation of the client test routines.
 * @date	01/03/2017
 */

#include <stdio.h>
#include <string.h>
#include "include/metadata.h"
#include "include/auxiliary.h"
#include "include/filesystem.h"

// Color definitions for asserts
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE   "\x1b[34m"

#define N_BLOCKS	2055           // Number of blocks in the device
#define DEV_SIZE 	N_BLOCKS * BLOCK_SIZE	// Device size, in bytes

int test_mkFS();
int test_mountFS();
int test_createFile();
int test_unmountFS();
int test_removeFile();

int main() {
	int ret;

	///////

	ret = test_mkFS();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mkFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mkFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////

	ret = test_mountFS();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mountFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////

	ret = test_createFile();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////

	ret = test_unmountFS();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////

	ret = test_removeFile();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST removeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST removeFile", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


   //////// 
    
    return 0;
}

int test_mkFS() {
    if (mkFS(DEV_SIZE) != 0) {
        return -1;
    }
    char buffer[BLOCK_SIZE] = {0};
    bread(DEVICE_IMAGE, 0, buffer);
   
    SuperBlock sblock = *(SuperBlock *) buffer;
    
    long necessary_space = sblock.max_inodes * (MAX_FILE_SIZE / BLOCK_SIZE);
    //printf("necessary_space = %ld\n", necessary_space);
    if (sblock.max_data_blocks < necessary_space) {
        return -1;
    }

    if (sblock.max_data_blocks + sblock.max_inodes + 3 > (DEV_SIZE / BLOCK_SIZE)) {
        return -1;
    }
    //printf("num_inodes: %ld \n", sblock.num_inodes);
    //printf("num_data_blocks: %ld \n", sblock.num_data_blocks);

    // Check that the allocation blocks are all 0
    for (int i = 1; i <= 2; i++) {
        bread(DEVICE_IMAGE, i, buffer);
        char buffer2[BLOCK_SIZE] = {0};
        if (memcmp(buffer, buffer2, BLOCK_SIZE) != 0) {
            return -1;
        }
    }
    
    return 0;
}


int test_mountFS() {
    printf("MOUNTFS TEST NOT IMPLEMENTED\n");
    return 0;
}


int test_unmountFS() {
    printf("UNMOUNTFS TEST NOT IMPLEMENTED\n");
    return 0;
}

int test_createFile() {
    int ret = createFile("test.txt");
    if (ret != 0) {
        return -1;
    }
  
    SuperBlock sblock = load_superblock();
    int inode_index = get_inode_index(&sblock, "test.txt");

    char bitmap[BLOCK_SIZE];
    bread(DEVICE_IMAGE, 1, bitmap);

    // Check that this inode has been correctly allocated
    if (bitmap_getbit(bitmap, inode_index) == 0) {
        return -1;
    }

    ret = createFile("test.txt");
    if (ret != -1) {
        // should already exist
        return -1;
    } 

    return 0;
}

// This test must be run after createFile
int test_removeFile() {
    SuperBlock sblock = load_superblock();
    int inode_index = get_inode_index(&sblock, "test.txt");

    char bitmap[BLOCK_SIZE];
    bread(DEVICE_IMAGE, 1, bitmap);
    if (bitmap_getbit(bitmap, inode_index) == 0) {
        return -1;    
    }
    int ret = removeFile("test.txt");
    if (ret != 0) {
        return -1;
    }
    bread(DEVICE_IMAGE, 1, bitmap);
    if (bitmap_getbit(bitmap, inode_index) == 1) {
        return -1;
    }

    ret = removeFile("test.txt");
    if (ret != -1) {
        return -1;    
    }

    sblock = load_superblock();

    inode_index = get_inode_index(&sblock, "test.txt");
    if (inode_index != -1) {
        return -1;    
    }
    return 0;
}
