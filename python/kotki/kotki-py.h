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

int scan();
int scan(const string& pathToJsonConfig);
string translate(const string& input, const string& language);
map<string, map<string, string>> listModels();
void _init();

PYBIND11_MODULE(kotki, m) {
  m.doc() = "Python binding for the kotki language translator";

  m.def("scan", pybind11::overload_cast<>(&scan), "Recursively search for 'registry.json' in various places. Returns amount of models loaded.");
  m.def("scan", pybind11::overload_cast<const std::string &>(&scan), "Load registry.json from a supplied path. Returns amount of models loaded.", pybind11::arg("path"));
  m.def("translate", &translate, "translate some text", pybind11::arg("text"), pybind11::arg("model"));
  m.def("listModels", &listModels, "list loaded translation models");
}
