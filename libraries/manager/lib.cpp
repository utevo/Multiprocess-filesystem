#include <iostream>

#include "../core/sync.hpp"
#include "../core/utils.hpp"

extern int test_manager_lib(int x) { return 2 * x * test_utils_lib(x) * test_sync_lib(x); }

extern int createFS(std::string path, unsigned int inodes_blocks,
                    unsigned int allocation_bitmap_blocks,
                    unsigned int data_blocks){};