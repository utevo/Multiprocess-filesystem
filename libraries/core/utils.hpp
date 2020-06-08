#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <vector>

const u_int32_t kBlockSize = 4096;
const u_int32_t kInodeSize = 32;

struct Superblock {
  u_int32_t block_size = kBlockSize;
  u_int32_t inode_size = kInodeSize;
  u_int32_t inode_blocks;
  u_int32_t inode_bitmap_blocks;
  u_int32_t allocation_bitmap_blocks;
  u_int32_t data_blocks;
  
  u_int32_t free[1018]; //why not kBlockSize/4 - 6? kBlockSize may vary
};

struct Inode {
  u_int16_t valid = 0;
  u_int16_t type = 0; // dir or normal file, 0 - file, 1 dir
  u_int32_t size = 0;
  u_int32_t direct_idxs[4] = {0,0,0,0};
  u_int32_t indirect_idx = 0;
  u_int32_t double_indirect_idx = 0;
};

enum FileStatus: u_int8_t {
  RDONLY = 0x1,
  WRONLY = 0x2,
  RDWR = 0x4,
};

struct OpenFile {
  FileStatus status;
  u_int32_t offset;
  u_int32_t inode_idx;
};

unsigned long myCeil(unsigned long first, unsigned long second);
std::vector<std::string> split(const std::string& str, char separator);
std::string trim(const std::string& s);

#endif