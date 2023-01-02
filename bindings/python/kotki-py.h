#include <string>
#include <vector>

#include <kotki/annotation.h>
#include <kotki/parser.h>
#include <kotki/project_version.h>
#include <kotki/response.h>
#include <kotki/response_options.h>
#include <kotki/kotki.h>
#include <kotki/translation_model.h>

#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

using namespace std;

static Kotki *kotki_ = nullptr;

void load();
void load(const string& pathToJsonConfig);
string translate(const string& input, const string& language);
map<string, map<string, string>> listModels();
void _init();

PYBIND11_MODULE(kotki, m) {
  m.doc() = "Python binding for the kotki language translator";

  m.def("loadRegistry", pybind11::overload_cast<>(&load), "Recursively search for 'registry.json' in '~/.config/kotki/models/', auto-detect translation models");
  m.def("loadRegistry", pybind11::overload_cast<const std::string &>(&load), "Load registry.json from a supplied path");
  m.def("translate", &translate, "translate some text");
  m.def("listModels", &listModels, "list loaded translation models");
}
