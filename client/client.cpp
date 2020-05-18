#include <iostream>

#include "../libraries/client/lib.hpp"

int main(){
    std::cout << test_client_lib(1) << std::endl;
    MFSClient client;
    client.mfs_mount("./mfs");
}
