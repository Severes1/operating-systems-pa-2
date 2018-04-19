/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	metadata.h
 * @brief 	Definition of the structures and data types of the file system.
 * @date	01/03/2017
 */

#include "blocks_cache.h"
#define MAX_FILENAME 20

#define bitmap_getbit(bitmap_, i_) (bitmap_[i_ >> 3] & (1 << (i_ & 0x07)))
static inline void bitmap_setbit(char *bitmap_, int i_, int val_) {
  if (val_)
    bitmap_[(i_ >> 3)] |= (1 << (i_ & 0x07));
  else
    bitmap_[(i_ >> 3)] &= ~(1 << (i_ & 0x07));
}

typedef struct filename { char name[MAX_FILENAME]; } filename_t;

typedef struct SuperBlock {
    long num_inodes;
    long num_data_blocks;
    filename_t filenames[(BLOCK_SIZE - 2 * sizeof(long)) / MAX_FILENAME];  
} SuperBlock;

typedef struct INode {
    long size;
    long num_blocks;
    void * blocks[BLOCK_SIZE - 2 * sizeof(long)];
} INode;

typedef char * AllocBlock;


