#include <string>
#include <vector>

#include <translator/annotation.h>
#include <translator/parser.h>
#include <translator/project_version.h>
#include <translator/response.h>
#include <translator/response_options.h>
#include <translator/kotki.h>
#include <translator/translation_model.h>

#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

using namespace std;

static Kotki *kotki_ = nullptr;

void loadRegistry(const string& pathToJsonConfig);
string translate(const string& input, const string& language);
vector<string> listModels();
void _init();

PYBIND11_MODULE(kotki, m) {
  m.doc() = "Python binding for the kotki language translator";

  m.def("loadRegistry", &loadRegistry, "load registry.json");
  m.def("translate", &translate, "translate some text");
  m.def("listModels", &listModels, "list available translation models");
}
