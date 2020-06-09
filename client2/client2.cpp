#include <cmath>
#include <fcntl.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "../libraries/client/lib.hpp"
#include "../libraries/core/utils.hpp"

const std::string path = "./mfs";

const std::string kHelpString = "create {path} - create file\n"
                                "write {path} {data} - write to file\n"
                                "read {path} - read file\n"
                                "mkdir {path} - create dir\n"
                                "ls {path} - list directory contents";

int openFile(MFSClient client, const std::string path) {
  int fd = client.mfs_open(path.c_str(), FileStatus::RDWR);
  if (fd == -1) {
    throw std::ios_base::failure("This file doesn't exist");
  }
  return fd;
}

void handleHelp() { std::cout << kHelpString << std::endl; }
void handleCreate(MFSClient client, const std::string path) {
  client.mfs_creat(path.c_str(), 0600);
}
void handleWrite(MFSClient client, const std::string path,
                 const std::string data) {
  int fd1 = client.mfs_open(path.c_str(), FileStatus::RDWR);
  char buff2[4100] = "abc";


  client.mfs_write(fd1, data.c_str(), data.length());
}
void handleRead(MFSClient client, const std::string path) {
  const u_int len = 4096;
  char buf[len];

  int fd1 = client.mfs_open(path.c_str(), FileStatus::RDWR);
  int result = client.mfs_read(fd1, buf, len);
  if (result == -1) {
    throw std::ios_base::failure("Couldn't read");
  }
  std::cout << "result: " << result << std::endl;
  std::cout << int(buf[0]) << std::endl;
}
void handleMkdir(MFSClient client, const std::string path) {
  client.mfs_mkdir(path.c_str());
}
void handleLs(MFSClient client, const std::string path) {
  std::vector<std::pair<uint32_t, std::string>> result =
      client.mfs_ls(path.c_str());

  for (std::pair<uint32_t, std::string> p : result) {
    std::cout << p.second << std::endl;
  }
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    handleHelp();
    return -1;
  }

  MFSClient client;
  client.mfs_mount(path.c_str());

  std::string firs_world(argv[1]);
  if (firs_world == "create") {
    if (argc != 3) {
      std::cout << "Wrong parameters" << std::endl;
      return -1;
    }
    handleCreate(client, argv[2]);
  } else if (firs_world == "write") {
    if (argc != 4) {
      std::cout << "Wrong parameters" << std::endl;
      return -1;
    }
    handleWrite(client, argv[2], argv[3]);
  } else if (firs_world == "read") {
    if (argc != 3) {
      std::cout << "Wrong parameters" << std::endl;
      return -1;
    }
    handleRead(client, argv[2]);
  } else if (firs_world == "mkdir") {
    if (argc != 3) {
      std::cout << "Wrong parameters" << std::endl;
      return -1;
    }
    handleMkdir(client, argv[2]);
  } else if (firs_world == "ls") {
    if (argc != 3) {
      std::cout << "Wrong parameters" << std::endl;
      return -1;
    }
    handleLs(client, argv[2]);
  } else {
    handleHelp();
  }
}
