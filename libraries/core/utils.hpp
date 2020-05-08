#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>

struct Superblock {
  const u_int32_t block_size = 4096;
  const u_int32_t inode_size = 32;
  u_int32_t blocks;
  u_int32_t inode_blocks;
  u_int32_t allocation_bitmap_blocks;
};

struct Inode {
  u_int8_t valid;
  u_int8_t type; // dir or normal file
  u_int32_t size;
  u_int32_t direct_ptrs[4];
  u_int32_t indirect_ptr;
  u_int32_t double_indirect_ptr;
};

#endif