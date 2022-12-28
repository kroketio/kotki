#pragma once

#include <iostream>

namespace marian::bergamot {

inline bool endsWith(std::string const & value, std::string const & ending) {
  if(ending.size() > value.size()) return false;
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

inline std::string readFromStdin() {
  // Read a large input text blob from stdin
  std::ostringstream inputStream;
  inputStream << std::cin.rdbuf();
  std::string input = inputStream.str();
  return input;
}

}  // namespace marian::bergamot
