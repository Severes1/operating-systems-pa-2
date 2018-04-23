/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	auxiliary.h
 * @brief 	Headers for the auxiliary functions required by filesystem.c.
 * @date	01/03/2017
 */

/* Calculates how many files can fit in  given disk_size,
   and initializes the superblock with the corresponding values */
void init_superblock(SuperBlock * sblock, long disk_size);

/* Updates the inode allocation bitmap on the disk
   Returns the index of the first free inode block */
int allocate_inode(); // Returns the index

/* Updates the data block allocation bitmap on the disk
   Returns the index of the first free data block */
int allocate_data_block(); // Returns the index

/* Helper function to load the superblock. */
SuperBlock load_superblock();

/* Returns index of file if it exists, -1 otherwise */
int get_inode_index(SuperBlock * sblock, char * fileName);

/* Wrapper function that hashes every written block to check file integrity */
int bwrite_with_crc(char *deviceName, int blockNumber, char *buffer);

/* Checks the integrity of the given block */
int check_crc(int blockNumber);
