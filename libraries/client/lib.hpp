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
  int mount(char *path);

  int open(char *name, int mode);
  int creat(char *name, int mode);
  int read(int fd, char *buf, int len);
  int write(int fd, char *buf, int len);
  int lseek(int fd, int whence, int offset);
  int unlink(char *name);

  int mkdir(char *name);
  int rmdir(char *name);

private:
  std::map<u_int32_t, u_int32_t> file_descriptions;
  std::map<u_int32_t, OpenFile> open_files;
  SyncClient sync_client;
};

#endif
