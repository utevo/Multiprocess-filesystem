#ifndef LIB_HPP
#define LIB_HPP

#include <cstdint>
#include <string>

extern int test_manager_lib(int x);

extern int createFS(std::string path,
             unsigned int inodes_blocks,
             unsigned int allocation_bitmap_blocks,
             unsigned int data_blocks);

#endif