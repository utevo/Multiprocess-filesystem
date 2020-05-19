#include <iostream>
#include <vector>
#include <cmath>

#include "../libraries/client/lib.hpp"

int main(){
    std::cout << test_client_lib(1) << std::endl;
    MFSClient client;
    client.mfs_mount("./mfs");
    for (int i = 0; i < 8 * 4096 + 12; ++i) {
        std::cout << client.getAndReserveFirstFreeBlock() << std::endl;
    }


//    int blockSize = 4096;
//    int blocks = blockSize * 8 + 20;
//    std::vector<u_int64_t> bitmap(blockSize/sizeof(u_int64_t));
//    std::cout << blockSize/sizeof(u_int64_t) << std::endl;
//    std::cout << bitmap.size() * sizeof(u_int64_t) << std::endl;
//    for(int i = 0; i < 2; ++i) {
//        int unreadBlocksNumber = blocks - (i * blockSize * 8);
//        int readCount = (unreadBlocksNumber > blockSize * 8) ?
//                        bitmap.size() * sizeof(u_int64_t) :
//                        ceil((double) unreadBlocksNumber / sizeof(u_int64_t));
//        std::cout << unreadBlocksNumber << std::endl;
//        std::cout << readCount << std::endl;
//    }



}
