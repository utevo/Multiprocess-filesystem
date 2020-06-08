// #include <iostream>
// #include <vector>
// #include <cmath>
// #include <fcntl.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <unistd.h>
// #include <fcntl.h>

#include "../libraries/client/lib.hpp"
// #include "../libraries/core/utils.hpp"

// void printInode(const Inode&);

int main() {
    MFSClient client;
    client.mfs_mount("./mfs");
}
