#ifndef CLIENT_LIB_HPP
#define CLIENT_LIB_HPP

#include <map>
#include "../core/utils.hpp"
#include "../core/sync.hpp"
#include "Handler.h"

extern int test_client_lib(int x);

class MFSClient {
public:
  int error;

  MFSClient();
  void mfs_mount(char *path);

  int mfs_open(char *name, int mode);
  int mfs_creat(char *name, int mode);
  int mfs_read(int fd, char *buf, int len);
  int mfs_write(int fd, char *buf, int len);
  int mfs_lseek(int fd, int whence, int offset);
  int mfs_unlink(char *name);

  int mfs_mkdir(char *name);
  int mfs_rmdir(char *name);

private:
  int openAndSeek(const int& offset = 0) const;
  int getLowestDescriptor() const;

  u_int32_t getInode(std::string path);
  u_int32_t getParentInode(std::string path);
  u_int32_t getInodeFromDirectoryByName(const int& disk_fd, const std::string& filename, const u_int32_t& directoryInode);

  void addInodeToDirectory(const u_int32_t& directoryInode, const u_int32_t& inode, const std::string& name);

  u_int32_t getAndTakeUpFirstFreeInode(); //return inode number
    //TODO think about: get n blocks by one call and return vector?
  u_int32_t getAndTakeUpFirstFreeBlock(); //returns block number
  u_int32_t getFirstFreeBitmapIndex(int disk_fd, u_int32_t offset, u_int32_t sizeInBlocks, u_int32_t amount) const;

  void freeInode(unsigned long index);
  void freeBlock(unsigned long index);
  void freeBitmapIndex(int disk_fd, u_int32_t offset, unsigned long index) const;


  u_int32_t blockSize;
  u_int32_t inodeSize;

  //number of blocks for each objects
  u_int32_t inodes;
  u_int32_t inodesBitmap;
  u_int32_t blocks;
  u_int32_t allocationBitmap;

  //offsets in bytes
  u_int32_t inodesBitmapOffset;
  u_int32_t inodesOffset;
  u_int32_t allocationBitmapOffset;
  u_int32_t blocksOffset;

  std::string disk_path;
  //key - file descriptor, value - open file structure
  std::map<u_int32_t, OpenFile> open_files;
  SyncClient sync_client;
};

#endif
