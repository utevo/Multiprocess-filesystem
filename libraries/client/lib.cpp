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
    disk_path = "./mfs";
}

int MFSClient::mfs_mount(char *path) {
    return 0;
}
int MFSClient::mfs_open(char *name, int mode) {
    int disk = openAndSkipSuperblock();
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
int MFSClient::openAndSkipSuperblock() {
    int fd = open(disk_path.c_str(), O_RDWR);
    if(fd == -1)
        throw std::ios_base::failure("Cannot open virtual disk");
    int seek = lseek(fd, kBlockSize, SEEK_DATA);
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


