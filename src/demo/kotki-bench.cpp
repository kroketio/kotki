// for testing
#include <string>
#include <chrono>
#include "kotki/kotki.h"

using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[]) {
  auto *kotki = new Kotki();
  kotki->scan();

  auto x = kotki->listModels();
  for(const auto z: x) {
    cout << z.first << endl;
  }

  vector<string> tests = {
      "Whenever I am at the office, I like to drink coffee.",
      "Car breaks after man steals gas in Breda, turns out to be diesel car.",
      "Investors are pursuing legal action against Virgin Galactic, claiming the company’s two spacecraft were not designed for regular space travel.",
      "Maemo Leste continues the legacy of Maemo. We aim to provide a free and open source Maemo experience on mobile phones and tablets like the Nokia N900.",
      "The modem support has been greatly improved by freemangordon, to the point where all known bugs seem to be fixed.",
      "Egg producers in Bulgaria expect their price to continue to rise even after the doubling of prices in the last year.",
      "Some technical failures or problems during flights were not properly disclosed."
  };

  cout << "=========================" << endl;
  for(int i = 0; i != tests.size(); i += 1) {
    cout << "(en->bg): " << tests[i] << endl;
    milliseconds then = duration_cast< milliseconds >(
        system_clock::now().time_since_epoch()
    );

    cout << kotki->translate(tests[i], "enbg") << endl;

    milliseconds now = duration_cast< milliseconds >(
        system_clock::now().time_since_epoch()
    );
    auto took = duration_cast<milliseconds>(now - then);
    auto ms = took.count();

    cout << "took: " << ms << "ms" << endl << endl;
  }

  return 0;
}
