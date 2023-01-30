// Sample C++ example that links against kotki-lib (see ../CMakeLists.txt)
// to build (from root project dir):
//    cmake -Bbuild -DBUILD_DEMO=1
//    make -Cbuild -j6
// the executable will be located at build/app/kotki-cli
#include <string>
#include "kotki/kotki.h"

using namespace std;


int main(int argc, char *argv[]) {
  if(argc != 3) {
    cout << "Usage: \n"
            "\t./kotki-cli <lang> <input>\n"
            "\t./kotki-cli 'enbg' 'My input text.'\n\n"
            "Download models here: https://github.com/kroketio/kotki/releases\n";
    return 1;
  }

  //string pathRegistry = argv[1];
  auto *kotki = new Kotki();
  kotki->scan();

  cout << kotki->translate(argv[2], argv[1]);

  return 0;
}
