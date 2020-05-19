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

    // ToDo

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

int MFSClient::getAndReserveFirstFreeBlock() {
    int disk = openAndSeek(allocationBitmapOffset);
    sync_client.AllocationBitmapLock();

    std::vector<u_int64_t> bitmap(blockSize/sizeof(u_int64_t));

    u_int64_t maxValue = 0xFFFFFFFFFFFFFFFF;
    int blockNumber = 0;
    bool stop = false;
    for(int i = 0; i < allocationBitmap; ++i) {

        int unreadBlocksNumber = blocks - (i * blockSize * 8);
        //read allocation bitmap
        //if need read whole block
        //otherwise only blocks number of bits
        int blocksToRead = (unreadBlocksNumber > blockSize * 8) ?
                bitmap.size() * sizeof(u_int64_t) * 8 :
                unreadBlocksNumber;

        int readCount = ceil((double)blocksToRead / sizeof(u_int64_t));
        if(readCount < 8)
            readCount = 8;

        if(read(disk, bitmap.data(), readCount) < 0)
            throw std::ios_base::failure("Cannot read allocation bitmap block");

        int bitsToRead = blocksToRead;
        for(int k = 0; k < ceil((double)readCount / sizeof(u_int64_t)); ++k) {
            if((bitmap[k] & maxValue) == maxValue)
                continue;
            u_int64_t tmp = 0x8000000000000000;
            int loopEnd = bitsToRead > sizeof(u_int64_t) * 8 ?
                      sizeof(u_int64_t) * 8 :
                      bitsToRead;
            for(int j = 0; i < loopEnd; ++j){
                if(blockNumber >= blocks)
                    throw std::out_of_range("All blocks is occupied");
                if((bitmap[k] & tmp) == 0) {
                    bitmap[k] |= tmp;
                    int seekN = lseek(disk, allocationBitmapOffset + i * blockSize, SEEK_SET);
                    if(seekN < 0)
                        throw std::ios_base::failure("Cannot seek on allocation bitmap");
                    int writeN = ;
                    if( write(disk, bitmap.data(), readCount) < 0)
                        throw std::ios_base::failure("Cannot seek on allocation bitmap");
                    stop = true;
                    break;
                }
                tmp >>= 1;
                blockNumber++;
            }
            if(stop)
                break;
            bitsToRead -= loopEnd;
        }
        if(stop)
            break;
    }
    sync_client.AllocationBitmapUnlock();
    close(disk);
    return blockNumber;
}


