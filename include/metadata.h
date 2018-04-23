/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	metadata.h
 * @brief 	Definition of the structures and data types of the file system.
 * @date	01/03/2017
 */

#include "blocks_cache.h"
#define MAX_FILENAME 32 

#define bitmap_getbit(bitmap_, i_) (bitmap_[i_ >> 3] & (1 << (i_ & 0x07)))
static inline void bitmap_setbit(char *bitmap_, int i_, int val_) {
  if (val_)
    bitmap_[(i_ >> 3)] |= (1 << (i_ & 0x07));
  else
    bitmap_[(i_ >> 3)] &= ~(1 << (i_ & 0x07));
}

/* Maps a filename to its INode */
typedef struct filename { 
    char name[MAX_FILENAME]; 
    long index;
} filename_t;

/* Contains information about the structure of the disk */
typedef struct SuperBlock {
    long num_crc_blocks; // The number of blocks to be used for CRC hashes
    long num_inodes_in_use; // The number of files that are stored on this disk
    long max_inodes; // The maximum number of files that can be stored on this disk
    long max_data_blocks; // The maximum amount of data blocks that are available

    /* A mapping of a filename to its index in the INode blocks */
    filename_t filenames[(BLOCK_SIZE - 2 * sizeof(long)) / MAX_FILENAME];  
} SuperBlock;

/* Keeps track of the locations of the data blocks associated with this file */
typedef struct INode {
    long size;
    long num_blocks;
    long blocks[BLOCK_SIZE - 2 * sizeof(long)];
} INode;

/* Contains in-memory data to process a file */
typedef struct OFT_Entry {
    int fd;
    long inode;
    long offset;
} OFT_Entry;
