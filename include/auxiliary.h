/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	auxiliary.h
 * @brief 	Headers for the auxiliary functions required by filesystem.c.
 * @date	01/03/2017
 */

/* Create an empty superblock */
void init_superblock(SuperBlock * sblock, long disk_size);
void init_inode(INode * inode);
int allocate_inode(); // Returns the index
int allocate_data_block(); // Returns the index
SuperBlock load_superblock();
INode load_inode(long i);
int get_inode_index(SuperBlock * sblock, char * fileName);

