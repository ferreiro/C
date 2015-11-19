#ifndef _MYFS_H_
#define _MYFS_H_

#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#define false 0
#define true 1

#define BIT unsigned
#define BLOCK_SIZE_BYTES 4096
#define NUM_BITS (BLOCK_SIZE_BYTES/sizeof(BIT))
#define MAX_BLOCKS_WITH_NODES 5
#define MAX_BLOCKS_PER_FILE 100
#define MAX_FILES_PER_DIRECTORY 100
#define MAX_LEN_FILE_NAME 15
#define DISK_LBA int
#define BOOLEAN int

#define SUPERBLOCK_IDX 0
#define BITMAP_IDX 1
#define DIRECTORY_IDX 2
#define NODES_IDX 3

// STRUCTS
typedef struct FileStructure {
	int  nodeIdx;                         	// Associated i-node
	char fileName[MAX_LEN_FILE_NAME + 1];	// File name
	BOOLEAN freeFile;                       // Free file
} FileStruct;

typedef struct DirectoryStructure {
	int numFiles;                          		// Num files
	FileStruct files[MAX_FILES_PER_DIRECTORY];	// Files
} DirectoryStruct;

typedef struct NodeStructure {
	int numBlocks;                        		// Num blocks
	int fileSize;                        		// File size
	time_t modificationTime;              		// Modification time
	DISK_LBA blocks[MAX_BLOCKS_PER_FILE];		// Blocks
	BOOLEAN freeNode;                        	// If the node is available
} NodeStruct;

#define NODES_PER_BLOCK (BLOCK_SIZE_BYTES/sizeof(NodeStruct))
#define MAX_NODES (NODES_PER_BLOCK * MAX_BLOCKS_WITH_NODES)

typedef struct SuperBlockStructure {
	time_t creationTime;     	// Creation time
	int diskSizeInBlocks;    	// # blocks in disk
	int numOfFreeBlocks;     	// # of available blocks
	int blockSize;            	// Block size
	int maxLenFileName;  		// Max. length of a file name
	int maxBlocksPerFile; 		// Max. number of blocks per file
} SuperBlockStruct;

typedef struct MyFileSystemStructure {
	int fdVirtualDisk;             		// File descriptor where the whole filesystem is stored
	SuperBlockStruct superBlock;   		// Super block
	BIT bitMap[NUM_BITS];            	// Bit map
	DirectoryStruct directory;     		// Root directory
	NodeStruct* nodes[MAX_NODES];		// Array of inode pointers
	int numFreeNodes;                  // # of available inodes
} MyFileSystem;

/**
 * @brief Copies the content from one node to another
 *
 * @param dest destination node
 * @param src source node
 * @return void
 **/
void copyNode(NodeStruct *dest, NodeStruct *src);

/**
 * @brief Looks for the index a file has in the directory
 *
 * @param myFileSystem pointer to the FS
 * @param fileName name of the file we are looking for
 * @return index of the fil