#ifndef CLIENT_LIB_HPP
#define CLIENT_LIB_HPP

#include <map>

#include "../core/utils.hpp"
#include "../core/sync.hpp"

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
  int openAndSeek(int offset = 0);
  int getLowestDescriptor();
  int getAndReserveFirstFreeBlock(); // returns offset to begin of block


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
  std::map<u_int32_t, u_int32_t> file_descriptions;
  std::map<u_int32_t, OpenFile> open_files;
  SyncClient sync_client;
};

#endif
