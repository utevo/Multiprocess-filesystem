#include "utils.hpp"
#include "sync.hpp"

#include <cmath>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include <iterator>
#include <sstream>

unsigned long myCeil(unsigned long first, unsigned long second) {
  return first / second + ((first % second != 0) ? 1 : 0);
}

std::vector<std::string> split(const std::string &str, char separator) {
  std::vector<std::string> cont;
  std::stringstream ss(str);
  std::string token;
  while (std::getline(ss, token, separator)) {
    if (token.empty())
      continue;
    cont.push_back(token);
  }
  return cont;
}

const std::string WHITESPACE = " \n\r\t\f\v";

std::string ltrim(const std::string &s) {
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string &s) {
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
std::string trim(const std::string &s) { return rtrim(ltrim(s)); }

Superblock ReadSuperblock(const std::string path) {
  int fd = open(path.c_str(), O_RDONLY);
  if (fd == -1)
    throw std::ios_base::failure("Cannot open virtual disk");
  Superblock superblock;
  read(fd, &superblock, sizeof(superblock));
  return superblock;
}

u_int32_t CalcInodes(u_int32_t inode_blocks) {
  return (kBlockSize / kInodeSize) * inode_blocks;
}
