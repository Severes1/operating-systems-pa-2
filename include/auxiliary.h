/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	auxiliary.h
 * @brief 	Headers for the auxiliary functions required by filesystem.c.
 * @date	01/03/2017
 */

typedef struct SuperBlock {
    int num_inodes;
    int num_data_blocks;
} SuperBlock;

typedef struct INode {
    int num_blocks;
    void * blocks[];
    int size;
} INode;

typedef char[256] AllocBlock;
