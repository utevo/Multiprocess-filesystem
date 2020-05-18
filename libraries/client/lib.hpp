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
  int blockSize;
  int inodeSize;
  //offsets in number of blocks, not bytes!!!
  int inodeBitmapOffset;
  int inodeOffset;
  int blocksBitmapOffset;
  int blocksOffset;

  std::string disk_path;
  std::map<u_int32_t, u_int32_t> file_descriptions;
  std::map<u_int32_t, OpenFile> open_files;
  SyncClient sync_client;

  int openAndSeek(int offset);
  int getLowestDescriptor();
  int getFirstFreeBlock(); // returns offset to begin of block
};

#endif
