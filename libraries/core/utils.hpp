#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>

struct Superblock {
  const u_int32_t block_size = 4096;
  const u_int32_t inode_size = 32;
  u_int32_t inode_blocks;
  u_int32_t inode_bitmap_blocks;
  u_int32_t allocation_bitmap_blocks;
  u_int32_t data_blocks;
};

struct Inode {
  u_int8_t valid;
  u_int8_t type; // dir or normal file
  u_int32_t size;
  u_int32_t direct_idxs[4];
  u_int32_t indirect_idx;
  u_int32_t double_indirect_idx;
};

enum FileStatus: u_int8_t {
  RDONLY,
  WRONLY,
  RDWR
};

struct OpenFile {
  FileStatus status;
  u_int32_t offset;
  u_int32_t inode_idx;
};

#endif