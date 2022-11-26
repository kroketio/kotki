// Sample C++ example that links against Kotki (see CMakeLists.txt)
// to build (from root project dir):
//    cmake -Bbuild
//    make -Cbuild -j6
// the executable will be located at build/app/kotki-cli
#include <string>
#include "translator/byte_array_util.h"
#include "translator/response_options.h"
#include "translator/utils.h"
#include "translator/kotki.h"
#include "translator/nb_prefix.h"

using namespace marian::bergamot;
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
  kotki->loadRegistryFromFile(pathRegistry);

  // list languages
  // cout << "Available languages: ";
  // std::vector<string> langs = kotki->listModels();
  // for(const auto &lang: kotki->listModels())
  //   cout << lang << ", ";
  // cout << endl << endl;

  cout << kotki->translate(argv[3], argv[2]);

  return 0;
}
