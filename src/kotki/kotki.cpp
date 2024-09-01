/**
* @file  kotki.h
* @brief A Bergamot-translator fork
*
* Copyright (C) 2021 Kroket Ltd. <info@kroket.io>
*
* This code is placed in the public domain.  You are free to use it for any purpose.
*
**/

#include "kotki/kotki.h"
#include "kotki/utils.h"

string KotkiTranslationModel::translate(string input) {
  if(!initialized) { this->load(); }

  Batch batch;
  marian::Ptr<Request> request = model->makeRequest(std::move(input), m_cache);

  model->enqueueRequest(request);
  model->generateBatch(batch);
  model->translateBatch(0, batch);

  return request->response.target.text;
}

void KotkiTranslationModel::load() {
  auto config = parseOptionsFromString("", false);
  config->set("ssplit-mode", "paragraph");
  config->set("beam-size", "1");
  config->set("normalize", "1.0");
  config->set("word-penalty", "0");
  config->set("max-length-break", "128");
  config->set("mini-batch-words", "1024");
  config->set("workspace", "2000");
  config->set("max-length-factor", "2.5");
  config->set("cpu-threads", "0");
  config->set("quiet", "true");
  config->set("quiet-translation", "true");
  config->set("alignment", "soft");

  config->set("ssplit-prefix-file", this->findNBPrefixFile());
  auto models = std::vector<std::string>({
      pathModel_
  });
  config->set("models", models);
  config->set("gemm-precision", endsWith(models[0], "intgemm8.bin") ? "int8shiftAll" : "int8shiftAlphaAll");

  auto vocabs = std::vector<std::string>({pathVocab_, pathVocab_});
  config->set("vocabs", vocabs);

  auto shortlist = std::vector<std::string>({pathLex_,
      "false"
  });
  config->set("shortlist", shortlist);

  model = marian::New<TranslationModel>(config);

  this->initialized = true;
}

string KotkiTranslationModel::findNBPrefixFile() {
  auto fileName = "nonbreaking_prefix." + this->langFrom;
  auto filePath = this->kotki_->kotkiCfgNbDir.string() + fileName;
  if(fs::exists(filePath)) { return filePath; }
  if(this->langFrom == "bg") {
    return this->kotki_->kotkiCfgNbDir.string() + "nonbreaking_prefix.ru";  // close 'nuff
  }
  return this->kotki_->kotkiCfgNbDir.string() + "nonbreaking_prefix." + nb_prefix_default;
}

Kotki::Kotki() {
  this->ensureConfigDirectory();
  this->ensureNBPrefixes();
}

std::string Kotki::translate(string input, string language) {
  if(!m_models.count(language)) {
    std::cerr << "language << " << language << " not found\n";
    if(language.length()<4)return "";
    string firstlang = language.substr(0,2);
    string secondlang = language.substr(2,2);
    if(firstlang=="en"||secondlang=="en")return "";
    return translate(translate(input,firstlang+"en"),"en"+secondlang);
  }

  auto result = m_models[language]->translate(input);
  // bug fix when result starts with '- '
  if((result.rfind("- ", 0) == 0)) {
    result = result.erase(0, 2);
  }
  return result;
}

map<string, map<string, string>> Kotki::listModels() {
  map<string, map<string, string>> data;
  for (auto const& [name, kotkiTranslationModel]: m_models) {
    data[name] = kotkiTranslationModel->toJson();
  }
  return data;
}

// Recursively search for 'registry.json'
// - ~/.config/kotki/models/
// - /usr/share/kotki/
int Kotki::scan() {
  string kotkiUsrDir = "/usr/share/kotki/";
  vector<filesystem::path> paths;
  vector<filesystem::path> cfgPaths;
  vector<filesystem::path> usrPaths;

  if(std::filesystem::exists(kotkiUsrDir))
    usrPaths = findFiles(kotkiUsrDir, "registry.json");
  if(std::filesystem::exists(kotkiCfgModelDir))
    cfgPaths = findFiles(kotkiCfgModelDir, "registry.json");

  // for now, just grab the first occurrence of registry.json in the config dir
  // @TODO: latest version detection
  if(!cfgPaths.empty()) {
    cfgPaths = {cfgPaths[0]};
  }

  paths.insert(paths.end(), usrPaths.begin(), usrPaths.end());
  paths.insert(paths.end(), cfgPaths.begin(), cfgPaths.end());

  if(paths.empty()) {
    string msg = "Could not auto-find models. Search directories:\n";
    msg += "- " + kotkiCfgModelDir.string() + "\n";
    msg += "- /usr/share/kotki/\n";
    msg += "More info: https://github.com/kroketio/kotki/releases/";
    throw std::runtime_error(msg);
  }
  return this->scan(paths);
}

int Kotki::scan(const fs::path& path) {
  if(!endsWith(path.string(), ".json"))
    throw std::runtime_error("kotkiCfgModelDir was empty");

  vector<filesystem::path> paths = {path};
  return this->scan(paths);
}

int Kotki::scan(vector<filesystem::path> paths) {
  if(paths.empty())
    throw std::runtime_error("scan(paths) was empty");

  int loaded = 0;
  for(const auto &path: paths) {
    vector<KotkiTranslationModel*> _models = this->loadRegistry(path);
    for(const auto &_model: _models) {
      if(m_models.count(_model->name)) {
        delete m_models[_model->name];
      }

      m_models[_model->name] = _model;
      loaded += 1;
    }
  }

  return loaded;
}

vector<KotkiTranslationModel*> Kotki::loadRegistry(const fs::path &regPath) {
  vector<KotkiTranslationModel*> results;
  string cwd = regPath.parent_path().string() + "/";
  auto buf = readFile(regPath);
  auto doc = jsonLoads(buf.str());
  const auto rootObj = doc->GetObject();

  for (auto const& group : rootObj) {
    const auto *name = group.name.GetString();
    if(strlen(name) != 4) { continue; }
    const auto obj = group.value.GetObject();

    string requiredErr;
    for(auto const& required: {"model", "lex", "vocab"}) {
      if(!obj.HasMember(required)) {
        requiredErr = required;
        break;
      }
    }

    if(!requiredErr.empty()) {
      std::cerr << "Skipping model " << name << " because " << requiredErr << " was missing\n";
      continue;
    }

    const auto modelObj = obj["model"].GetObject();
    const auto lexObj = obj["lex"].GetObject();
    const auto vocabObj = obj["vocab"].GetObject();

    auto modelPath = cwd + modelObj["name"].GetString();
    auto lexPath = cwd + lexObj["name"].GetString();
    auto vocabPath = cwd + vocabObj["name"].GetString();

    string pathErr;
    for(const auto &path: {modelPath, lexPath, vocabPath}) {
      if(!fs::exists(path)) {
        pathErr = path;
        break;
      }
    }

    if(!pathErr.empty()) {
      std::cerr << "Skipping model " << name << " because path " << pathErr << " does not exist\n";
      continue;
    }

    auto *model = new KotkiTranslationModel(name, cwd, modelPath, lexPath, vocabPath, this);
    results.emplace_back(model);
  }

  return results;
}

void Kotki::ensureConfigDirectory() {
  // create kotki data dir in ~/.config/kotki/
  kotkiCfgDir = find_config_directory() + "/kotki";
  kotkiCfgNbDir = kotkiCfgDir.string() + "/nb_prefixes/";
  kotkiCfgModelDir = kotkiCfgDir.string() + "/models/";

  if(!fs::is_directory(kotkiCfgDir)) {
    fs::create_directories(kotkiCfgDir);
  }
  if(!fs::is_directory(kotkiCfgNbDir)) {
    fs::create_directory(kotkiCfgNbDir);
  }
  if(!fs::is_directory(kotkiCfgModelDir)) {
    fs::create_directories(kotkiCfgModelDir);
  }
}

void Kotki::ensureNBPrefixes() const {
  // write the 'nonbreaking_prefix files' to kotki cfg dir, marian needs them.
  for (auto const& [lang_name, data] : nb_prefix_lookup) {
    const auto pathDestination = kotkiCfgNbDir.string() + "nonbreaking_prefix." + lang_name;
    std::ofstream out(pathDestination);
    out << data;
    out.close();
  }
}

string Kotki::find_config_directory() {
  std::string config_directory;

  // Check the XDG_CONFIG_HOME environment variable
  const char* xdg_config_home = std::getenv("XDG_CONFIG_HOME");
  if (xdg_config_home) {
    config_directory = xdg_config_home;
  } else {
    // If XDG_CONFIG_HOME is not set, use the default value of "$HOME/.config"
    const char* home = std::getenv("HOME");
    if (home) {
      config_directory = std::string(home) + "/.config";
    } else {
      // If HOME is not set, use the current working directory
      config_directory = std::filesystem::current_path().string();
    }
  }

  return config_directory;
}
