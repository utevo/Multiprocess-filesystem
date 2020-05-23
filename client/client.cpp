#include <iostream>
#include <vector>
#include <cmath>
#include <fcntl.h>
#include <unistd.h>

#include "../libraries/client/lib.hpp"

int main(){
    std::cout << test_client_lib(1) << std::endl;
    MFSClient client;
    client.mfs_mount("./mfs");
}