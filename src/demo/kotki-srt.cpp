// Silly program that translates .srt subtitle files to a given language, for demo purposes.
//    cmake -Bbuild
//    make -Cbuild -j6
// the executable will be located at build/app/kotki-srt
#include <stdio.h>
#include <iostream>
#include <filesystem>

#include "kotki/byte_array_util.h"
#include "kotki/response_options.h"
#include "kotki/kotki.h"

using namespace marian::bergamot;
using namespace std;
namespace fs = std::filesystem;

bool is_digits(const std::string &str) {
  return str.find_first_not_of("0123456789") == std::string::npos;
}
bool allUpperCase(const std::string &val);
bool startswith(const string& haystack, const string& needle);
bool endswith(std::string const & value, std::string const &ending);
void nukeChar(string &val, const char _char);
int main(int argc, char *argv[]) {
  if(argc != 4) {
    cout << "Usage: \n"
            "\t./kotki-srt <path_to_registry.json> <lang> <srt_file>\n"
            "\t./kotki-srt '/home/user/example/models/registry.json' 'ennl' 'test.srt'\n\n"
            "Download models here: https://github.com/kroketio/kotki/releases\n";
    return 1;
  }

  const string regPath = argv[1];
  const string modelName = argv[2];
  const string subPath = argv[3];

  if(!std::filesystem::exists(regPath))
    throw std::runtime_error("could not read " + regPath);

  if(!std::filesystem::exists(subPath))
    throw std::runtime_error("could not read " + subPath);

  auto *kotki = new Kotki();
  kotki->scan(regPath);

  string line;
  ifstream srt;
  srt.open(subPath);

  if(!srt.is_open()) {
    perror("Error open subtitle");
    exit(EXIT_FAILURE);
  }

  std::stringstream buffer;
  buffer << srt.rdbuf();
  auto tmpBuf = buffer.str();

  // BOM removal
  for(const string &bom : {"\xEF\xBB\xBF", "\xFE\xFF", "\xFF\xFE", "\x00\x00\xFE\xFF", "\xFF\xFE\x00\x00"}) {
    if(startswith(tmpBuf, bom)) {
      tmpBuf.erase(0, bom.length());
      break;
    }
  }
  buffer.str(tmpBuf);

  string result;
  string tmpSub;
  while(getline(buffer, line)) {
    nukeChar(line, '\n');
    nukeChar(line, '\r');

    if(line.empty()) {
      if(tmpSub.empty()) {
        result += "\n";
        continue;
      }

      tmpSub.pop_back();
      if(tmpSub.empty()) {
        result += "\n";
        continue;
      }

      string translation;
      const auto wrapPos = tmpSub.find('\n');
      if(wrapPos != string::npos) {
        std::replace(tmpSub.begin(), tmpSub.end(), '\n', ' ');
      }

      // kotki doesn't handle *all* uppercase text well. let's lowercase it.
      // We assume ASCII here, and it is OK if this 'fix' only works for
      // latin, just a hack anyway to improve *some* specific subtitle situation
      if(allUpperCase(tmpSub)) {
        std::transform(tmpSub.begin(), tmpSub.end(), tmpSub.begin(),
                       ::tolower);
      }

      // translate
      translation = kotki->translate(tmpSub, modelName);

      // maintain word-wrap
      if(wrapPos != string::npos) {
        const auto wrapPosNew = translation.find(' ', wrapPos);
        if(wrapPosNew != string::npos) {
          translation[wrapPosNew] = '\n';
        }
      }

      result += translation + "\n\n";
      tmpSub = "";
      continue;
    }

    // index & timestamp can be skipped immediately
    if(line == "\n" || is_digits(line) || (
                                               line.find(" --> ") != string::npos &&
                                               std::count(line.begin(), line.end(), ',') == 2
                                               )
    ) {
      result += line + "\n";
      continue;
    }

    tmpSub += line + "\n";
  }

  cout << result;
  return 0;
}

bool startswith(const string& haystack, const string& needle) {
  return needle.length() <= haystack.length() && equal(needle.begin(), needle.end(), haystack.begin());
}

bool endswith(std::string const & value, std::string const & ending) {
  if(ending.size() > value.size()) { return false; }
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void nukeChar(string &val, const char _char) {
  val.erase(std::remove(val.begin(), val.end(), _char), val.cend());
}

bool allUpperCase(const std::string &val) {
  return std::all_of(val.begin(), val.end(), [](unsigned char c){
    const unsigned int _c = static_cast<int>(c);
    return (_c >= 65 && _c <= 90) || (_c <= 96 || _c >= 123);
  });
}