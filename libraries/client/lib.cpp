#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
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
    int seek = lseek(fd, offset * blockSize, SEEK_DATA);
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

    sync_client.AllocationBitmapUnlock();
    return 0;
}


