#include "lib.hpp"

#include <cmath>
#include <iostream>

#include "../core/utils.hpp"

u_int CalcInodeBitmapBlocks(u_int inodes_blocks) {
  u_int inodes_in_one_block = kBlockSize / kInodeSize;
  u_int inodes = inodes_blocks * inodes_in_one_block;
  u_int inodes_in_one_inode_bitmap_block = kBlockSize * 8;
  u_int inode_bitmap_blocks =
      ceil((double)inodes / inodes_in_one_inode_bitmap_block);
  return inode_bitmap_blocks;
}

u_int CalcAllocationBitmapBlocks(u_int data_blocks) {
  u_int data_blocks_in_one_allocation_bitmap_block = kBlockSize * 8;
  u_int allocation_bitmap_blocks =
      ceil((double)data_blocks / data_blocks_in_one_allocation_bitmap_block);
  return allocation_bitmap_blocks;
}

extern int CreateFS(std::string path, u_int inodes_blocks, u_int data_blocks) {
  return CalcInodeBitmapBlocks(inodes_blocks);
};