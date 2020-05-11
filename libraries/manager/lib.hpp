#ifndef MANAGER_LIB_HPP
#define MANAGER_LIB_HPP

#include <cstdint>
#include <string>

extern int test_manager_lib(int x);

extern int CreateFS(std::string path,
                    unsigned int inodes_blocks,
                    unsigned int data_blocks);

#endif