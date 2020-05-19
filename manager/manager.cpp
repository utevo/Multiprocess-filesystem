#include <iostream>
#include <string>

#include "./../libraries/manager/lib.hpp"

std::string path = "./mfs";

int main(){
  CreateFS(path, 1, 8 * 4096 + 128);
}