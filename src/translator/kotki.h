#ifndef K_H
#define K_H

#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <map>
#include <regex>

#include "nb_prefix.h"
#include "translation_model.h"
#include "logging.h"

#include "rapidjson/document.h"
#include "rapidjson/schema.h"

using namespace std;
using namespace marian::bergamot;
using namespace rapidjson;
namespace fs = std::filesystem;

class Kotki;
struct KotkiTranslationModel {
  // name should be 4 chars, e.g: 'nlen' (Dutch to English)
  explicit KotkiTranslationModel(string name, string pathModel, string pathLex, string pathVocab, Kotki* kotki)
      : name(name),
        pathModel_(std::move(pathModel)),
        pathLex_(std::move(pathLex)),
        pathVocab_(std::move(pathVocab)),
        kotki_(kotki) {
    langFrom = name.substr(0, 2);
    langTo = name.erase(0, 2);
  }

  string name;
  string langFrom;
  string langTo;
  bool initialized = false;
  void load();
  string translate(string input);

  shared_ptr<TranslationModel> model;
 private:
  string pathModel_;
  string pathLex_;
  string pathVocab_;
  Kotki* kotki_;
  string findNBPrefixFile();
  std::optional<TranslationCache> m_cache = std::nullopt;
};

class Kotki {
 public:
  Kotki();

  void loadFromString(string config_json, const string &cwd);
  void loadRegistryFromFile(const fs::path& path);
  string translate(string input, string language);
  vector<string> listModels();
  void ensureConfigDirectory();
  void ensureNBPrefixes() const;

  string kotkiCfgDir;
  string kotkiCfgNbDir;

 private:
  Document m_doc;
  map<string, KotkiTranslationModel*> m_models;
};

#endif // KroketTranslation_H