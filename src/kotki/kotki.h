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

#include "kotki/nb_prefix.h"
#include "kotki/translation_model.h"
#include "kotki/lang.h"

#include "rapidjson/document.h"
#include "rapidjson/schema.h"

using namespace std;
using namespace marian::bergamot;
using namespace rapidjson;
namespace fs = std::filesystem;

class Kotki;
struct KotkiTranslationModel {
  // name should be 4 chars, e.g: 'nlen' (Dutch to English)
  explicit KotkiTranslationModel(string name, string cwd, string pathModel, string pathLex, string pathVocab,string pathTrgVocab, Kotki* kotki)
      : name(name),
        cwd(cwd),
        pathModel_(std::move(pathModel)),
        pathLex_(std::move(pathLex)),
        pathVocab_(std::move(pathVocab)),
        pathTrgVocab_(std::move(pathTrgVocab)),
        kotki_(kotki) {
    langFrom = name.substr(0, 2);
    langTo = name.erase(0, 2);
  }

  string name;
  string cwd;
  string langFrom;
  string langTo;
  bool initialized = false;
  void load();
  string translate(string input);
  shared_ptr<TranslationModel> model;
  map<string, string> toJson() {
    map<string, string> rtn;
    rtn["name"] = this->name;
    rtn["cwd"] = this->cwd;
    rtn["version"] = "";  // @TODO: support versions
    rtn["model"] = this->pathModel_;
    rtn["lex"] = this->pathLex_;
    if(this->pathVocab_==this->pathTrgVocab_)rtn["vocab"] = this->pathVocab_;
    else {
        rtn["srcvocab"] = this->pathVocab_;
        rtn["trgvocab"] = this->pathTrgVocab_;
    }
    rtn["description"] = "";
    if(lang::countryCodes.count(langFrom) &&
       lang::countryCodes.count(langTo)) {
      rtn["description"] = lang::countryCodes[langFrom] +
                           " -> " +
                           lang::countryCodes[langTo];
    }
    return rtn;
  }
 private:
  string pathModel_;
  string pathLex_;
  string pathVocab_;
  string pathTrgVocab_;
  Kotki* kotki_;
  string findNBPrefixFile();
  std::optional<TranslationCache> m_cache = std::nullopt;
};

class Kotki {
 public:
  Kotki();

  int scan();
  int scan(const fs::path& path);
  int scan(vector<filesystem::path> paths);
  int clear();

  bool modelExists(string name);
  vector<KotkiTranslationModel*> loadRegistry(const fs::path &regPath);

  string translate(string input, string language);
  map<string, map<string, string>> listModels();
  void ensureConfigDirectory();
  void ensureNBPrefixes() const;
  static string find_config_directory();

  std::filesystem::path kotkiCfgDir;
  std::filesystem::path kotkiCfgNbDir;
  std::filesystem::path kotkiCfgModelDir;

 private:
  map<string, KotkiTranslationModel*> m_models;
};

#endif // KroketTranslation_H