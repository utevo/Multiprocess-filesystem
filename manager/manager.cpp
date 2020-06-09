#include <iostream>
#include <string>

#include "./../libraries/manager/lib.hpp"

const std::string path = "./mfs";

const std::string kHelpString =
    "h - help\nc - create file system\ni - init sync\nr - remove sync";

void handleHelp() { std::cout << kHelpString << std::endl; }

void handleCreate() { CreateFS(path, 32, 4096); }

void handleInitSync() { InitSynchronization(path); }

void handleRmSync() { RemoveSynchronization(path); }

int main(int argc, char *argv[]) {
  if (argc != 2) {
    handleHelp();
    return -1;
  }

  char firs_letter = argv[1][0];
  if (firs_letter == 'c') {
    handleCreate();
  } else if (firs_letter == 'i') {
    handleInitSync();
  } else if (firs_letter == 'r') {
    handleRmSync();
  } else if (firs_letter == 'h') {
    handleHelp();
  }
}
