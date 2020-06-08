#include <iostream>
#include <vector>
#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "../libraries/client/lib.hpp"
#include "../libraries/core/utils.hpp"

void printInode(const Inode&);

int main() {
    MFSClient client;
    client.mfs_mount("./mfs");

    client.mfs_creat("/plik1.txt", FileStatus::RDWR);
    client.mfs_creat("/plik2.txt", FileStatus::RDWR);
    client.mfs_creat("/plik3.txt", FileStatus::RDWR);
    client.mfs_creat("/plik4.txt", FileStatus::RDWR);
    client.mfs_creat("/plik5.txt", FileStatus::RDWR);
    client.mfs_unlink("/plik3.txt");
    client.mfs_creat("/plik6.txt", FileStatus::RDWR);
    client.mfs_mkdir("/folder");
    client.mfs_creat("/folder/plik2.txt", FileStatus::RDWR);
    client.mfs_creat("/folder/plik3.txt", FileStatus::RDWR);
    for(auto s : client.mfs_ls("/folder"))
        std::cout << s.first << "\t" << s.second << std::endl;
}
