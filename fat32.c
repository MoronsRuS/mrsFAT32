#include "fat32.h"

#define BPB_BytsPerSec_Offset	0x0B
#define BPB_SecPerClus_Offset	0x0D
#define BPB_RsvdSecCnt_Offset	0x0E
#define BPB_NumFATs_Offset	0x10
#define BPB_FATSz32_Offset	0x24
#define BPB_RootClus_Offset	0x2C
#define BPB_Sanity_Offset	0x1FE

void byteCopy(void* dst, void* src, size_t num);
void extricateShort(uint16_t* dst, uint8_t* src);
void extricateWord(uint32_t* dst, uint8_t* src);

uint8_t buffer[512];

fat32Error fat32OpenDisk(fat32Disk* disk,fat32ReadBlock readBlock) {
	disk->readBlock = readBlock;
	readBlock(0,buffer);
	byteCopy(disk->partitions,&(buffer[446]),4*16);
	extricateShort(&(disk->sanity),&(buffer[510]));
	if (disk->sanity != 0xAA55) {
		return FAT32_DISK_INSANE;
	}
	return FAT32_NO_ERROR;
}

fat32Error fat32OpenPartition(
	fat32Partition* partition, fat32Disk* disk, size_t index
) {
	fat32ReadBlock* readBlock = disk->readBlock;
	if (disk->sanity != 0xAA55) {
		return FAT32_DISK_INSANE;
	}
	partition->readBlock = readBlock;
	partition->typeCode = disk->partitions[index][4];
	if (partition->typeCode != 0x0C && partition->typeCode != 0x0B) {
		return FAT32_PART_BAD_TYPE;
	}
	extricateWord(&(partition->lbaBegin),&(disk->partitions[index][8]));
	extricateWord(&(partition->numSectors),&(disk->partitions[index][12]));

	readBlock(partition->lbaBegin,buffer);
	extricateShort(
		&(partition->sanity),
		&(buffer[BPB_Sanity_Offset])
	);
	if (partition->sanity != 0xAA55) {
		return FAT32_PART_INSANE;
	}
	extricateShort(
		&(partition->bytesPerSector),
		&(buffer[BPB_BytsPerSec_Offset])
	);
	if (partition->bytesPerSector != 512) {
		return FAT32_PART_BAD_SECTSIZE;
	}
	partition->sectorsPerCluster = buffer[BPB_SecPerClus_Offset];
	extricateShort(
		&(partition->numReservedSectors),
		&(buffer[BPB_RsvdSecCnt_Offset])
	);
	partition->numFATs = buffer[BPB_NumFATs_Offset];
	if (partition->numFATs != 2) {
		return FAT32_PART_BAD_FATCOUNT;
	}
	extricateWord(
		&(partition->sectorsPerFAT),
		&(buffer[BPB_FATSz32_Offset])
	);
	extricateWord(
		&(partition->rootDirectoryCluster),
		&(buffer[BPB_RootClus_Offset])
	);
	partition->lbaFAT= partition->lbaBegin + partition->numReservedSectors;
	partition->lbaCluster= partition->lbaFAT + 
		(partition->numFATs * partition->sectorsPerFAT);
}

void byteCopy(void* dst, void* src, size_t num) {
	size_t i;
	uint8_t* _dst=(uint8_t*)dst;
	uint8_t* _src=(uint8_t*)src;
	for (i=0;i<num;++i) {
		*_dst = *_src;
		++_dst;
		++_src;
	}
}
void extricateShort(uint16_t* dst, uint8_t* src) {
	(*dst) = src[0];
	(*dst) += src[1] << 8;
}
void extricateWord(uint32_t* dst, uint8_t* src) {
	(*dst) = src[0];
	(*dst) += src[1] << 8;
	(*dst) += src[2] << 16;
	(*dst) += src[3] << 24;
}

