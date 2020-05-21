#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <cmath>
#include "lib.hpp"

#include "../core/sync.hpp"
#include "../core/utils.hpp"

extern int test_client_lib(int x) { return 1 * x; }

extern const u_int32_t kBlockSize;
extern const u_int32_t kInodeSize;

MFSClient::MFSClient() {
    error = 0;
}

void MFSClient::mfs_mount(char *path) {
    disk_path = path;
    int fd = openAndSeek();
    if(read(fd, &blockSize, sizeof(u_int32_t)) <= 0)
        throw std::ios_base::failure("Cannot read block size from super block");
    if(read(fd, &inodeSize, sizeof(u_int32_t)) <= 0)
        throw std::ios_base::failure("Cannot read inode size from super block");
    if(read(fd, &inodes, sizeof(u_int32_t)) <= 0)
        throw std::ios_base::failure("Cannot read number of inodes blocks from super block");
    inodes *= blockSize / inodeSize;
    inodesOffset = blockSize;
    if(read(fd, &inodesBitmap, sizeof(u_int32_t)) <= 0)
        throw std::ios_base::failure("Cannot read number of inodes bitmap blocks from super block");
    inodesBitmapOffset = inodesOffset + inodes * blockSize;
    if(read(fd, &allocationBitmap, sizeof(u_int32_t)) <= 0)
        throw std::ios_base::failure("Cannot read number of allocations bitmap from super block");
    allocationBitmapOffset = inodesBitmapOffset + inodesBitmap * blockSize;
    if(read(fd, &blocks, sizeof(u_int32_t)) <= 0)
        throw std::ios_base::failure("Cannot read block size from super block");
    blocksOffset = allocationBitmapOffset + allocationBitmap * blockSize;
    close(fd);
}

int MFSClient::mfs_open(char *name, int mode) {
    return 0;
}

int MFSClient::mfs_creat(char *name, int mode) {
    return 0;
}

int MFSClient::mfs_read(int fd, char *buf, int len) {
    u_int32_t open_file_idx = file_descriptions.at(fd);
    OpenFile open_file = open_files.at(open_file_idx);
    u_int32_t inode_idx = open_file.inode_idx;

    sync_client.ReadLock(inode_idx);

    // TODO

    sync_client.ReadUnlock(inode_idx);
}

int MFSClient::mfs_write(int fd, char *buf, int len) {
    return 0;
}

int MFSClient::mfs_lseek(int fd, int whence, int offset) {
    return 0;
}

int MFSClient::mfs_unlink(char *name) {
    return 0;
}

int MFSClient::mfs_mkdir(char *name) {
    return 0;
}

int MFSClient::mfs_rmdir(char *name) {
    return 0;
}

int MFSClient::openAndSeek(int offset) {
    if(offset < 0)
        throw std::invalid_argument("Offset cannot be lower than zero");

    int fd = open(disk_path.c_str(), O_RDWR);
    if(fd == -1)
        throw std::ios_base::failure("Cannot open virtual disk");
    int seek = lseek(fd, offset, SEEK_CUR);
    if(seek == -1)
        throw std::ios_base::failure("Error during lseek on virtual disk");
    return fd;
}

int MFSClient::getLowestDescriptor() {
    int lowestFd = 1;
    while (file_descriptions.find(lowestFd) != file_descriptions.end())
        ++lowestFd;
    return lowestFd;
}

int MFSClient::getAndTakeUpFirstFreeBlock() {
    sync_client.AllocationBitmapLock();
    int disk = openAndSeek(allocationBitmapOffset);
    int blockNumber;
    try {
        blockNumber = getFirstFreeBitmapIndex(disk, allocationBitmapOffset, allocationBitmap, blocks);
    }
    catch (std::out_of_range&) {
        throw std::out_of_range("All blocks is occupied");
    }
    close(disk);
    sync_client.AllocationBitmapUnlock();
    return blockNumber;
}

int MFSClient::getAndTakeUpFirstFreeInode() {
    sync_client.InodeBitmapLock();
    int disk = openAndSeek(inodesBitmapOffset);
    int inodeNumber;
    try {
        inodeNumber = getFirstFreeBitmapIndex(disk, inodesBitmapOffset, inodesBitmap, inodes);
    }
    catch (std::out_of_range&) {
        throw std::out_of_range("All inodes is occupied");
    }
    close(disk);
    sync_client.InodeBitmapUnlock();
    return inodeNumber;
}

int MFSClient::getFirstFreeBitmapIndex(int disk_fd, u_int32_t offset, u_int32_t sizeInBlocks, u_int32_t amount) {
    u_int64_t maxValue = 0xFFFFFFFFFFFFFFFF;
    unsigned long indexNumber = 0;

    for(int i = 0; i < sizeInBlocks; ++i) {
        std::vector<u_int64_t> bitmap(blockSize/sizeof(u_int64_t));
        unsigned long unreadIndexes = amount - (i * blockSize * 8);
        bool stop = false;
        unsigned long uints64ToRead = unreadIndexes > blockSize * 8 ?
                                      bitmap.size() :
                                      myCeil(unreadIndexes, (sizeof(u_int64_t) * 8));

        unsigned long readCount = uints64ToRead * sizeof(u_int64_t);
        if(read(disk_fd, bitmap.data(), readCount) < 0)
            throw std::ios_base::failure("Cannot read bitmap block");

        unsigned long endLoop = myCeil(readCount, sizeof(u_int64_t));
        if((unreadIndexes % 64 == 0) && (endLoop < bitmap.size()))
            endLoop++;

        for(int k = 0; k < endLoop; ++k) {
            if(bitmap[k] == maxValue){
                indexNumber += 64;
                continue;
            }
            u_int64_t tmp = 0x0000000000000001;
            while (tmp <= maxValue) {
                if(indexNumber >= amount)
                    throw std::out_of_range("");
                if((bitmap[k] & tmp) == 0) {
                    bitmap[k] |= tmp;
                    if(lseek(disk_fd, offset + i * blockSize, SEEK_SET) < 0)
                        throw std::ios_base::failure("Cannot seek on bitmap");
                    if(write(disk_fd, bitmap.data(), readCount) < 0)
                        throw std::ios_base::failure("Cannot seek on bitmap");
                    stop = true;
                    break;
                }
                tmp <<= 1;
                indexNumber++;
            }
            if(stop)
                break;
        }
        if(stop)
            break;
    }
    return indexNumber;
}

void MFSClient::freeInode(unsigned long index) {
    if(index > inodes)
        throw std::invalid_argument("Inode index is greater than number of inodes");
    sync_client.InodeBitmapLock();
    int disk = openAndSeek(inodesBitmapOffset);
    try {
        freeBitmapIndex(disk, inodesBitmapOffset, index);

        //TODO set inode valid as false in inode table

    }
    catch (std::out_of_range&) {
        throw std::out_of_range("Inode has already freed");
    }
    close(disk);
    sync_client.InodeBitmapUnlock();

}

void MFSClient::freeBlock(unsigned long index)  {
    if(index > blocks)
        throw std::invalid_argument("Block index is greater than number of blocks");
    sync_client.AllocationBitmapLock();
    int disk = openAndSeek(allocationBitmapOffset);
    try {
        freeBitmapIndex(disk, allocationBitmapOffset, index);
        //TODO clear block

    }
    catch (std::out_of_range&) {
        throw std::out_of_range("Block has already freed");
    }
    close(disk);
    sync_client.AllocationBitmapUnlock();
}

void MFSClient::freeBitmapIndex(int disk_fd, u_int32_t offset, unsigned long index) {
    unsigned int blockNumber = index / (blockSize * 8);
    unsigned int indexInBlock = index - (blockNumber * blockSize * 8);
    unsigned int offsetToReadUInt8 = indexInBlock / (sizeof(uint8_t) * 8);
    u_int8_t bitmap;

    if(lseek(disk_fd, offset + blockNumber * blockSize + offsetToReadUInt8, SEEK_SET) < 0)
        throw std::ios_base::failure("Cannot seek bitmap block");
    if(read(disk_fd, &bitmap, sizeof(u_int8_t)) < 0)
        throw std::ios_base::failure("Cannot read bitmap block");

    unsigned int positionInBitmap = index % 8;
    u_int8_t tmp = 0x01;
    tmp <<= positionInBitmap;
    tmp = ~tmp;

    bitmap &= tmp;
    //TODO throw if index has already freed
    if(lseek(disk_fd, offset + blockNumber * blockSize + offsetToReadUInt8, SEEK_SET) < 0)
        throw std::ios_base::failure("Cannot seek bitmap block");
    if(write(disk_fd, &bitmap, sizeof(u_int8_t)) < 0)
        throw std::ios_base::failure("Cannot write bitmap block");
}



