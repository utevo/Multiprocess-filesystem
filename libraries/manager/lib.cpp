#include <iostream>

extern int test_manager_lib(int x) { return 2 * x; }

extern int createFS(std::string path,
                    unsigned int inodes_blocks,
                    unsigned int data_blocks) {};