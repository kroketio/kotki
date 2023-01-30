#include "kotki-py.h"

int scan() {
  if(kotki_ == nullptr) _init();
  return kotki_->scan();
}

int scan(const string& pathToJsonConfig) {
  if(kotki_ == nullptr) _init();
  return kotki_->scan(pathToJsonConfig);
}

string translate(const string& input, const string& language) {
  if(kotki_ == nullptr) _init();
  return kotki_->translate(input, language);
}

map<string, map<string, string>> listModels() {
  if(kotki_ == nullptr) _init();
  return kotki_->listModels();
}

void _init() {
  kotki_ = new Kotki();
}
