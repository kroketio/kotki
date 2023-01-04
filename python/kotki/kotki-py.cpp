#include "kotki-py.h"

void scan() {
  if(kotki_ == nullptr) _init();
  kotki_->scan();
}

void scan(const string& pathToJsonConfig) {
  if(kotki_ == nullptr) _init();
  kotki_->scan(pathToJsonConfig);
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
