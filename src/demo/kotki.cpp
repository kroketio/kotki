// Sample C++ example that links against kotki-lib (see ../CMakeLists.txt)
// to build (from root project dir):
//    cmake -Bbuild
//    make -Cbuild -j6
// the executable will be located at build/app/kotki-cli
#include <string>
#include "kotki/kotki.h"

using namespace std;


int main(int argc, char *argv[]) {
  if(argc != 4) {
    cout << "Usage: \n"
            "\t./kotki-cli <path_to_registry.json> <lang> <input>\n"
            "\t./kotki-cli '/home/user/example/models/registry.json' 'ennl' 'My input text.'\n\n"
            "Download models here: https://github.com/kroketio/kotki/releases\n";
    return 1;
  }

  string pathRegistry = argv[1];
  auto *kotki = new Kotki();
  kotki->scan(pathRegistry);

  cout << kotki->translate(argv[3], argv[2]);

  return 0;
}
