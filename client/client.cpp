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

// void printInode(const Inode&);

int main() {
    MFSClient client;
    client.mfs_mount("./mfs");

    client.mfs_creat("/plik1.txt", FileStatus::RDWR);
    client.mfs_creat("/plik2.txt", FileStatus::RDWR);
    client.mfs_creat("/plik3.txt", FileStatus::RDWR);
    client.mfs_creat("/plik4.txt", FileStatus::RDWR);
//    client.mfs_mkdir("/folder");
//    client.mfs_mkdir("/folder2");
//    client.mfs_mkdir("/folder2");
//
//    client.mfs_creat("/folder/plik4.txt", FileStatus::RDWR);
    client.mfs_creat("/plik5.txt", FileStatus::RDWR);
    client.mfs_unlink("/plik3.txt");
    int fd = client.mfs_creat("/plik6.txt", FileStatus::RDWR);
    int fd1 = client.mfs_open("/plik6.txt", FileStatus::RDWR);
    client.mfs_creat("/plik6.txt", FileStatus::RDWR);
    char buff[17000];
    for(int i = 0; i < 17000; ++i)
        buff[i] = 'a';
    client.mfs_write(fd, buff, 17000);
    char buff2[4100];
    for(int i = 0; i < 4100; ++i)
        buff2[i] = 'b';
    client.mfs_write(fd1, buff2, 4100);


    int filefd = client.openAndSeek(client.blocksOffset + 2 * client.blockSize);
    char tmp[4096];
    char tmp2[4096];
    read(filefd, &tmp, 4096);
    lseek(filefd, client.blocksOffset + 6 * client.blockSize, SEEK_SET);
    read(filefd, &tmp2, 4096);
    for(auto s : client.mfs_ls("/"))
        std::cout << s.first << "\t" << s.second << std::endl;
    std::cout << " " << std::endl;
//    for(auto s : client.mfs_ls("/folder"))
//        std::cout << s.first << "\t" << s.second << std::endl;
}
