#ifndef MANAGER_LIB_HPP
#define MANAGER_LIB_HPP

#include <cstdint>
#include <string>

u_int CalcInodeBitmapBlocks(u_int inodes_blocks) noexcept;
u_int CalcAllocationBitmapBlocks(u_int data_blocks) noexcept;

extern void CreateFS(std::string path, u_int inodes_blocks, u_int data_blocks);

#endif