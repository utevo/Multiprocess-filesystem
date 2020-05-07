#ifndef LIB_HPP
#define LIB_HPP

#include <dirent.h>
#include <sys/types.h>
#include <vector>


extern int test_client_lib(int x);

class MFSClient {

public:
  int error;

  MFSClient();
  int mount(char* path);

  int open(char *name, int mode);
  int creat(char *name, int mode);
  int read(int fd, char *buf, int len);
  int write(int fd, char *buf, int len);
  int lseek(int fd, int whence, int offset);
  int unlink(char *name);

  int mkdir(char *name);
  int rmdir(char *name); 
};

#endif
