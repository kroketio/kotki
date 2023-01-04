#pragma once

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>

namespace marian::bergamot {

inline bool endsWith(std::string const & value, std::string const & ending) {
  if(ending.size() > value.size()) return false;
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::vector<std::filesystem::path> findFiles(const std::filesystem::path& root, const std::string& name) {
  std::vector<std::filesystem::path> paths;
  if(!std::filesystem::exists(root) || !std::filesystem::is_directory(root)) return paths;
  for(const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
    if(entry.path().filename() == name) {
      paths.push_back(entry.path());
    }
  }

  return paths;
}

Document* jsonLoads(const string &body) {
  auto *rtn = new Document();
  rtn->Parse(body.c_str());
  if(rtn->HasParseError())
    throw std::runtime_error("could not parse JSON.");
  return rtn;
}

std::stringstream readFile(const fs::path& path) {
  std::ifstream fileStream(path);
  std::stringstream buffer;
  buffer << fileStream.rdbuf();
  fileStream.close();
  return buffer;
}

inline std::string readFromStdin() {
  // Read a large input text blob from stdin
  std::ostringstream inputStream;
  inputStream << std::cin.rdbuf();
  std::string input = inputStream.str();
  return input;
}

}  // namespace marian::bergamot
