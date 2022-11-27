/**
* @file  kotki.h
* @brief A Bergamot-translator fork
*
* Copyright (C) 2021 Kroket Ltd. <info@kroket.io>
*
* This code is placed in the public domain.  You are free to use it for any purpose.
*
**/

#include "kotki.h"
#include "utils.h"
#include "cfgpath.h"

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

  LOG(info, "loaded {}", name);
  this->initialized = true;
}

string KotkiTranslationModel::findNBPrefixFile() {
  auto fileName = "nonbreaking_prefix." + this->langFrom;
  auto filePath = this->kotki_->kotkiCfgNbDir + fileName;
  if(fs::exists(filePath)) { return filePath; }
  if(this->langFrom == "bg") {
    return this->kotki_->kotkiCfgNbDir + "nonbreaking_prefix.ru";  // close 'nuff
  }
  return this->kotki_->kotkiCfgNbDir + "nonbreaking_prefix." + nb_prefix_default;
}

Kotki::Kotki() {
  this->ensureConfigDirectory();
  this->ensureNBPrefixes();
}

std::string Kotki::translate(string input, string language) {
  if(!m_models.count(language)) {
    LOG(error, "language \"{}\" not found", language);
    return "";
  }

  auto result = m_models[language]->translate(input);

  // bug fix when result starts with '- '
  if((result.rfind("- ", 0) == 0)) {
    result = result.erase(0, 2);
  }
  return result;
}

std::vector<string> Kotki::listModels() {
  vector<string> langs;
  for (auto const& [key, val]: m_models) {
    langs.emplace_back(key);
  }
  return langs;
}

void Kotki::loadRegistryFromFile(const fs::path& path) {
  string cwd = path.parent_path();
  if(!endsWith(cwd, "/")) { cwd += '/'; }
  if(!std::filesystem::exists(path))
    throw std::runtime_error("could not read " + path.string());

  std::ifstream fileStream(path);
  std::stringstream buffer;
  buffer << fileStream.rdbuf();

  return loadFromString(buffer.str(), cwd);
}

void Kotki::loadFromString(string config_json, const string &cwd) {
  m_doc.Parse(config_json.c_str());

  if(m_doc.HasParseError()) {
    throw std::runtime_error("could not parse JSON configuration.");
  }

  auto rootObj = m_doc.GetObject();
  for (auto const& group : rootObj){
    const auto *name = group.name.GetString();
    if(strlen(name) != 4) { continue; }
    auto obj = group.value.GetObject();

    string requiredErr;
    for(auto const& required: {"model", "lex", "vocab"}) {
      if(!obj.HasMember(required)) {
        requiredErr = required;
        break;
      }
    }

    if(!requiredErr.empty()) {
      LOG(warn, "Skipping model {} because \"{}\" was missing", name, requiredErr);
      continue;
    }

    auto modelObj = obj["model"].GetObject();
    auto lexObj = obj["lex"].GetObject();
    auto vocabObj = obj["vocab"].GetObject();

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
      LOG(warn, "Skipping model {} because path \"{}\" does not exist", name, pathErr);
      continue;
    }

    auto *kmodel = new KotkiTranslationModel(name, modelPath, lexPath, vocabPath, this);

    m_models[name] = kmodel;
  }
}

void Kotki::ensureConfigDirectory() {
  // create kotki data dir
  char cfgdir[512];
  get_user_data_folder(cfgdir, sizeof(cfgdir), "kotki");
  if(cfgdir[0] == 0) {
    throw std::runtime_error("could not determine user data folder");
  }

  kotkiCfgDir = string(cfgdir);
  fs::create_directories(cfgdir);

  kotkiCfgNbDir = kotkiCfgDir + "/nb_prefixes/";
  if(!fs::is_directory(kotkiCfgNbDir)) {
    fs::create_directory(kotkiCfgNbDir);
  }
}

void Kotki::ensureNBPrefixes() const {
  // write the 'nonbreaking_prefix files' to kotki cfg dir, marian needs them.
  for (auto const& [lang_name, data] : nb_prefix_lookup) {
    const auto pathDestination = kotkiCfgNbDir + "nonbreaking_prefix." + lang_name;
    std::ofstream out(pathDestination);
    out << data;
    out.close();
  }
}
