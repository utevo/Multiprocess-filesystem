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

int main(){
    MFSClient client;
    client.mfs_mount("./mfs");
    int disk = client.openAndSeek();
    lseek(disk, client.blocksOffset + kBlockSize, SEEK_SET);
    char buffer2[28];
    u_int32_t dir1, dir2;
    read(disk, &dir1, 4);
    read(disk, buffer2, 28);
    std::cout << dir1 << std::endl;
    std::cout << buffer2 << std::endl;
    read(disk, &dir1, 4);
    read(disk, buffer2, 28);
    std::cout << dir1 << std::endl;
    std::cout << buffer2 << std::endl;
    close(disk);

}

void printInode(const Inode &inode) {
    std::cout << "Inode: " << std::endl;
    std::cout << inode.type << std::endl;
    std::cout << inode.size << std::endl;
    std::cout << inode.valid << std::endl;
    for(auto b : inode.direct_idxs)
        std::cout << b << "\t";
    std::cout << "\n";
    std::cout << inode.indirect_idx << std::endl;
    std::cout << inode.double_indirect_idx << std::endl;
}
