#include "kotki-py.h"

void loadRegistry(const string& pathToJsonConfig) {
  if(kotki_ == nullptr) _init();
  kotki_->loadRegistryFromFile(pathToJsonConfig);
}

string translate(const string& input, const string& language) {
  if(kotki_ == nullptr) _init();
  return kotki_->translate(input, language);
}

vector<string> listModels() {
  if(kotki_ == nullptr) _init();
  return kotki_->listModels();
}

void _init() {
  kotki_ = new Kotki();
}
