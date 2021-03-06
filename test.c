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
#include <stdlib.h>

// Color definitions for asserts
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE   "\x1b[34m"

#define N_BLOCKS	2055                    // Number of blocks in the device
#define DEV_SIZE 	N_BLOCKS * BLOCK_SIZE	// Device size, in bytes

#define TEST_FILE "paella.jpg"

int test_mkFS();
int test_mountFS();
int test_createFile();
int test_unmountFS();
int test_removeFile();
int test_write();
int test_big_write();
int test_checkFile();
int test_checkFS();
int realtest(char * in_file);

int test_interleave();

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
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST removeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


   //////// 
    
    ret = test_write();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST write ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST write ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


   //////// 
    
    ret = test_big_write();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST big write ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST big write ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


   //////// 
 
   ret = test_checkFile();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST checkFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST checkFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


   //////// 
 
  ret = realtest(TEST_FILE);
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST realTest ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST realTest ", ANSI_COLOR_GREEN, "SUCCESS\n",
    ANSI_COLOR_RESET);


   //////// 
   ret = test_interleave();

	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST interleave ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}


	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST interleave ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


   //////// 
  
   ret = test_checkFS();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST checkFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST checkFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


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
    return mountFS();
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

// Assume mount happened
int test_write() {
    createFile("test.txt");
    int fd = openFile("test.txt");
    if (fd == -1) {
        return -1;    
    }
    char * buffer = "Hello world";
    int ret = writeFile(fd, buffer, 12);
    if (ret < 0) {
        return -1;    
    }

    lseekFile(fd, 0, FS_SEEK_BEGIN);
    char buffer2[12];
    ret = readFile(fd, buffer2, 12);
    if (ret < 0) {
        return -1;    
    }
    closeFile(fd);
    if (strcmp(buffer, buffer2) != 0) {
        return -1;    
    }
    removeFile("test.txt");
    return 0;
}

int test_big_write() {
    createFile("test.txt");
    int fd = openFile("test.txt");
    if (fd == -1) {
        return -1;    
    }
    char buffer[2 * BLOCK_SIZE];
    for (int i = 0; i < 2 * BLOCK_SIZE / 12; i++) {
        strcpy(&buffer[i * 12], "Hello world");    
    }
    int ret = writeFile(fd, buffer, 2 * BLOCK_SIZE);
    if (ret < 0) {
        return -1;    
    }
    
    lseekFile(fd, 0, FS_SEEK_BEGIN);
    char buffer2[2 * BLOCK_SIZE];
    ret = readFile(fd, buffer2, 2 * BLOCK_SIZE);
    if (ret < 0) {
        return -1;    
    }
    closeFile(fd);
    int i;
    for (i = 0; i < BLOCK_SIZE * 2; i++) {
        if (buffer[i] != buffer2[i]) {
            break;       
        }
    }
    if (i != 2 * BLOCK_SIZE) {
        buffer[i + 10] = '\0';
        buffer2[i + 10] = '\0';
        printf("At index %d: %s != %s\n", i, &buffer[i], &buffer2[i]);
        return -1;
    }

    ret = memcmp(buffer, buffer2, 2 * BLOCK_SIZE);
    if (ret != 0) {
        return -1;    
    }
    return 0;

}

// Perform directly after test_big_write
int test_checkFile() {
    int ret = checkFile("test.txt");    
    if (ret != 0) {
        return -1;    
    }
    return 0;
}


// Perform directly after test_big_write
int test_checkFS() {
    int ret = checkFS(); 
    if (ret != 0) {
        return -1;    
    }
    return 0;
}



/* Write a large file to disk */
int realtest(char * in_file) {
    createFile("realtest.txt");
    int fd = openFile("realtest.txt");

    FILE *f = fopen(in_file, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  //same as rewind(f);

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;
    writeFile(fd, string, fsize);
    free(string);
    
    close(fd);

    return 0;
}

int test_interleave() {
    createFile("F1");
    createFile("F2");
    int fd1 = openFile("F1");
    int fd2 = openFile("F2");

    char buff1[BLOCK_SIZE];
    memset(buff1, 'a', BLOCK_SIZE);

    char buff2[BLOCK_SIZE];
    memset(buff2, 'b', BLOCK_SIZE);

    writeFile(fd1, buff1, BLOCK_SIZE);


    writeFile(fd2, buff2, BLOCK_SIZE);
    memset(buff1, 'a', BLOCK_SIZE);
    writeFile(fd1, buff1, BLOCK_SIZE);
    writeFile(fd2, buff2, BLOCK_SIZE);
    closeFile(fd1);
    closeFile(fd2);

    removeFile("F1");
    createFile("F3");

    int fd3 = openFile("F3");

    char buff3[BLOCK_SIZE];
    memset(buff3, 'c', BLOCK_SIZE);
    
    writeFile(fd3, buff3, BLOCK_SIZE);

    closeFile(fd3);
    return 0;
}
