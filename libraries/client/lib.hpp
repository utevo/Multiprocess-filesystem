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
  int mfs_mount(char *path);

  int mfs_open(char *name, int mode);
  int mfs_creat(char *name, int mode);
  int mfs_read(int fd, char *buf, int len);
  int mfs_write(int fd, char *buf, int len);
  int mfs_lseek(int fd, int whence, int offset);
  int mfs_unlink(char *name);

  int mfs_mkdir(char *name);
  int mfs_rmdir(char *name);

private:
  std::string disk_path;
  std::map<u_int32_t, u_int32_t> file_descriptions;
  std::map<u_int32_t, OpenFile> open_files;
  SyncClient sync_client;

  int openAndSkipSuperblock();
  int getLowestDescriptor();
};

#endif
