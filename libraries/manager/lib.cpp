#include "lib.hpp"

#include <cmath>
#include <fcntl.h>
#include <ios>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../core/utils.hpp"

u_int CalcInodeBitmapBlocks(u_int inodes_blocks) noexcept {
  u_int inodes_in_one_block = kBlockSize / kInodeSize;
  u_int inodes = inodes_blocks * inodes_in_one_block;
  u_int inodes_in_one_inode_bitmap_block = kBlockSize * 8;
  u_int inode_bitmap_blocks =
      ceil((double)inodes / inodes_in_one_inode_bitmap_block);
  return inode_bitmap_blocks;
}

u_int CalcAllocationBitmapBlocks(u_int data_blocks) noexcept {
  u_int data_blocks_in_one_allocation_bitmap_block = kBlockSize * 8;
  u_int allocation_bitmap_blocks =
      ceil((double)data_blocks / data_blocks_in_one_allocation_bitmap_block);
  return allocation_bitmap_blocks;
}

void AppendSuperblock(int fd, Superblock superblock) {
  int result = write(fd, &superblock, kBlockSize);
  if (result != kBlockSize)
    throw std::iostream::failure("Couldn't add superblock");
}

void AppendEmptyBlocks(int fd, u_int blocks) {
  u_int64_t bytes = blocks * kBlockSize;
  const int before = lseek(fd, 0, SEEK_CUR);
  const int after = lseek(fd, bytes - 1, SEEK_CUR);
  if (after - before != bytes - 1) {
    throw std::iostream::failure("Couldn't add blocks");
  }
}

void AppendInodeBlocks(int fd, u_int inode_blocks) {
  try {
    AppendEmptyBlocks(fd, inode_blocks);
  } catch (std::iostream::failure e) {
    throw std::iostream::failure("Couldn't add inode blocks");
  }
}

void AppendInodeBitmapBlocks(int fd, u_int inode_bitmap_blocks) {
  try {
    AppendEmptyBlocks(fd, inode_bitmap_blocks);
  } catch (std::iostream::failure e) {
    throw std::iostream::failure("Couldn't add inode bitmap blocks");
  }
}

void AppendAllocationBitmapBlocks(int fd, u_int allocation_bitmap_blocks) {
  try {
    AppendEmptyBlocks(fd, allocation_bitmap_blocks);
  } catch (std::iostream::failure e) {
    throw std::iostream::failure("Couldn't add allocation bitmap blocks");
  }
}

void AppendDataBlocks(int fd, u_int data_blocks) {
  try {
    AppendEmptyBlocks(fd, data_blocks);
  } catch (std::iostream::failure e) {
    throw std::iostream::failure("Couldn't add data blocks");
  }
}

extern void CreateFS(std::string path, u_int inodes_blocks, u_int data_blocks) {
  u_int inode_bitmap_blocks = CalcInodeBitmapBlocks(inodes_blocks);
  u_int allocation_bitmap_blocks = CalcAllocationBitmapBlocks(data_blocks);

  int fd = creat(path.c_str(), S_IRUSR | S_IRGRP | S_IROTH);
  if (fd < 0)
    throw std::iostream::failure("Couldn't open file");

  Superblock superblock;
  superblock.inode_blocks = inodes_blocks;
  superblock.inode_bitmap_blocks = inode_bitmap_blocks;
  superblock.allocation_bitmap_blocks = allocation_bitmap_blocks;
  superblock.data_blocks = data_blocks;
  AppendSuperblock(fd, superblock);

  AppendInodeBlocks(fd, inodes_blocks);
  AppendInodeBitmapBlocks(fd, inode_bitmap_blocks);
  AppendAllocationBitmapBlocks(fd, allocation_bitmap_blocks);
  AppendDataBlocks(fd, data_blocks);
};