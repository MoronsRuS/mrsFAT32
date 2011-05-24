#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <stddef.h>

typedef void (*fat32ReadBlock(uint32_t lba, uint8_t* buff));

typedef uint32_t fat32Error;
#define FAT32_NO_ERROR				0x00000000
#define FAT32_DISK_INSANE			0x00000008
#define FAT32_PART_BAD_TYPE			0x00000100
#define FAT32_PART_BAD_SECTSIZE			0x00000200
#define FAT32_PART_BAD_FATCOUNT			0x00000400
#define FAT32_PART_INSANE			0x00000800

typedef struct {
	fat32ReadBlock*	readBlock;
	uint8_t		typeCode;
	uint32_t	lbaBegin;
	uint32_t	numSectors;
	uint16_t	bytesPerSector;		//Always 512 Bytes
	uint8_t		sectorsPerCluster;	//1,2,4,8,16,32,64,128
	uint16_t	numReservedSectors;	//Usually 0x20
	uint8_t		numFATs;		//Always 2
	uint32_t	sectorsPerFAT;		//Depends on disk size
	uint32_t	rootDirectoryCluster;	//Usually 0x00000002
	uint16_t	sanity;			//Always 0xAA55
	uint64_t	lbaFAT;
	uint64_t	lbaCluster;
} fat32Partition;

typedef struct {
	fat32ReadBlock*	readBlock;
	uint8_t		partitions[4][16];
	uint16_t	sanity;			//Should be 0xAA55
} fat32Disk;

typedef struct {
	fat32Partition*	partition;
	//Context Info
} fat32FileContext;

typedef struct {
	fat32Partition*	partition;
	//Context Info
} fat32BrowseContext;

fat32Error fat32OpenDisk(fat32Disk* disk,fat32ReadBlock readBlock);
fat32Error fat32OpenPartition(
	fat32Partition* partition, fat32Disk* disk, size_t index
);

#endif
