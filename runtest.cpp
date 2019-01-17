
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char * argv[]) {
  for (int i = 0; i < argc; i++) {
    std::cout << "argv[" << i << "]"
              << " = " << argv[i] << std::endl;
  }
  return 0;
}
