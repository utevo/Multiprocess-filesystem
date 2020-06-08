#include <iostream>
#include <string>

#include "./../libraries/manager/lib.hpp"

const std::string path = "./mfs";

int main(int argc, char *argv[]) {
  if (argc != 2) {
    return -1;
  }

  char firs_letter = argv[1][0];
  if (firs_letter == 'c') {
    CreateFS(path, 13, 1);
  } else if (firs_letter == 'i') {
    InitSynchronization(path);
  } else if (firs_letter == 'r') {
    RemoveSynchronization(path);
  }
}
