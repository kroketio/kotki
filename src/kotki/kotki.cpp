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
    return "";
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

// Recursively search for 'registry.json' in '~/.config/kotki/models/', auto-detect translation models
// @TODO: try to find the most recent `registry.json`
void Kotki::scan() {
  if(kotkiCfgModelDir.empty())
    throw std::runtime_error("kotkiCfgModelDir was empty");

  std::map<filesystem::path, Document*> results;
  std::vector<filesystem::path> paths = findFiles(kotkiCfgModelDir, "registry.json");

  for(const auto& path : paths) {
    try {
      auto doc = jsonLoads(readFile(path).str());
      results[path] = doc;
    } catch(const std::exception&) {
      std::cerr << "skipping path " + path.string();
    }
  }

  if(results.empty()) {
    throw std::runtime_error("Could not auto-find models in '" + kotkiCfgModelDir.string() +
                             "'. Read github.com/kroketio/kotki/releases/ for instructions.");
  }

  // @TODO: implement latest version detection
//  for (auto const& [key, val] : results) {
//    auto rootObj = m_doc.GetObject();
//    if(rootObj.HasMember("version")){
//
//    }
//  }

  // @TODO: we just pick the first occurrence for now
  for (auto const& [_path, jsonDoc] : results) {
    auto cwd = _path.parent_path().string();
    scan(jsonDoc, cwd);
    break;
  }
}

void Kotki::scan(const fs::path& path) {
  if(kotkiCfgModelDir.empty())
    throw std::runtime_error("kotkiCfgModelDir was empty");
  if(!std::filesystem::exists(path))
    throw std::runtime_error("could not read " + path.string());
  if(fs::is_directory(path))
    throw std::runtime_error("path is not a file: " + path.string());

  string cwd = path.parent_path().string() + "/";
  Document *cfg;

  try {
    auto buf = readFile(path);
    cfg = jsonLoads(buf.str());
  } catch(const std::exception&) {
    throw std::runtime_error("error parsing JSON: " + path.string());
  }
  return scan(cfg, cwd);
}

void Kotki::scan(Document *config_json, const fs::path &cwd) {
  if(kotkiCfgModelDir.empty())
    throw std::runtime_error("kotkiCfgModelDir was empty");
  if(!fs::is_directory(cwd))
    throw std::runtime_error("cwd must be a directory: " + cwd.string());
  if(config_json == nullptr)
    throw std::runtime_error("config may not be empty.");

  const auto cwdStr = cwd.string() + "/";
  const auto rootObj = config_json->GetObject();
  for (auto const& group : rootObj){
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

    auto modelPath = cwdStr + modelObj["name"].GetString();
    auto lexPath = cwdStr + lexObj["name"].GetString();
    auto vocabPath = cwdStr + vocabObj["name"].GetString();

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

    auto *kmodel = new KotkiTranslationModel(name, cwd, modelPath, lexPath, vocabPath, this);

    if(m_models.count(name)) {
      delete m_models[name];
    }

    m_models[name] = kmodel;
  }

  delete config_json;
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
