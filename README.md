# kotki

[![License: MPL v2](https://img.shields.io/badge/License-MPL%20v2-blue.svg)](https://www.mozilla.org/en-US/MPL/2.0/)

Fast language translations without using the cloud using Neural Machine techniques. Written in C++. Based on [Bergamot](https://browser.mt/).

Uses language models borrowed from the Mozilla extension ['Firefox Translations'](https://addons.mozilla.org/en-US/firefox/addon/firefox-translations/).

100% FOSS, **Linux only**

## Quick start

### Requirements

For Ubuntu:

```bash
sudo apt update && sudo apt upgrade
sudo apt install -y cmake ccache build-essential git pkg-config rapidjson-dev pybind11-dev libyaml-cpp-dev python3-dev python3-virtualenv libopenblas-dev libpcre2-dev libprotobuf-dev protobuf-compiler libsqlite3-dev
```

### Python

1. `pip install kotki -v`
2. [Install language translation models](https://github.com/kroketio/kotki/releases)

#### Programmatically

```python3
import kotki
kotki.scan()  # auto-find language translation models
# kotki.scan("/path/to/registry.json")  # or supply the path

# English -> German
kotki.translate("Whenever I am at the office, I like to drink coffee.", "ende")
'Wann immer ich im büro bin, trinke ich gerne kaffee.'

# Bulgarian -> English
kotki.translate("Румънците получиха дълго чакани новини: пенсиите и минималната заплата ще бъдат увеличени от 2023 г.", "bgen")
'Romanians have received long-awaited news: pensions and minimum wages will be increased from 2023'

# Dutch -> English
>>> kotki.translate("Auto begeeft het nadat man benzine steelt in Breda, blijkt dieselauto te zijn", "nlen")
'Car breaks after man steals gas in Breda, turns out to be diesel car'

# English -> Polish
>>> kotki.translate("I am going outside to buy some Pierogi.", "enpl")
'Jadę na zewnątrz, żeby kupić Pierogi.'
```

#### CLI

```text
$ kotki-cli --help
Usage: kotki-cli [OPTIONS]

  Translate some text.

Options:
  -i, --input TEXT         Text to translate  [required]
  -m, --model TEXT         Model names. Use -l to list. Leave empty to guess
                           the input language automatically.
  -r, --registry FILENAME  Path to registry.json. Leave empty for auto-
                           detection of translation models.
  -l, --list               List available models.
  -d, --debug              Print debug log.
  --help                   Show this message and exit.
```

#### Self-hosted web-interface

Example: [kotki.kroket.io](https://kotki.kroket.io)

```text
$ kotki-web --help
Usage: kotki-web [OPTIONS]

  Exposes kotki via HTTP web interface and provide an API.

Options:
  -h, --host TEXT          bind host (default: 127.0.0.1)  [required]
  -p, --port INTEGER       bind port (default: 7000)  [required]
  -d, --debug              run Quart web-framework in debug
  -r, --registry FILENAME  Path to registry.json. Leave empty for auto-
                           detection of translation models.
  --help                   Show this message and exit.
```

### C++

Link against `kotki-lib` (CMake target, see `src/demo/` for reference).

```cpp
#include <string>
#include "kotki/kotki.h"

using namespace std;
int main(int argc, char *argv[]) {
  auto *kotki = new Kotki();
  kotki->scan();
  // auto loadedModels = kotki->listModels();  // show currently loaded language models
  cout << kotki->translate("This should work, in theory.", "ende");  // English to German
  return 0;
}
```

## why

Kotki is aimed at developers who "just want to translate some text" in their C++ or Python applications without 
too much headache, as other translation frameworks are often big, difficult to compile, non-performant, etc.

## Producing libkotki

`libkotki.so` or `libkotki.a`

#### Via CMake

Install [marian-lite](https://github.com/kroketio/marian-lite) (and its dependencies) manually
(and if you are lazy, you can let kotki download the dependencies
automatically via `-DVENDORED_LIBS=ON` - though your mileage may vary).

- `STATIC` - Produce static binary (TODO: doesn't work yet)
- `SHARED` - Produce shared binary
- `BUILD_DEMO` - Produce example demo application(s)

```bash
cmake -DBUILD_DEMO=ON -DSTATIC=OFF -DSHARED=ON -Bbuild .
make -Cbuild -j6
sudo make -Cbuild install  # install into /usr/local/...
```

#### Via debian packaging

```bash
sudo apt install -y debhelper
dpkg-buildpackage -b -uc
````

## Library usage (CMake)

```cmake
cmake_minimum_required(VERSION 3.16)
find_package(kotki REQUIRED)
target_link_libraries(my_app PRIVATE kotki::kotki-lib)
```

## Models

The translation models are borrowed from the
Mozilla [Firefox Translations](https://addons.mozilla.org/en-US/firefox/addon/firefox-translations/) extension. **You need 
to manually download these models.** They are conveniently packaged into a single archive that can be 
downloaded over at [kotki/releases](https://github.com/kroketio/kotki/releases).

Extract to `~/.config/kotki/models/` for automatic detection:

```bash
mkdir -p ~/.config/kotki/models/
wget https://github.com/kroketio/kotki/releases/download/v0.4.5/kotki_models_0.3.3.zip
unzip kotki_models_0.3.3.zip -d ~/.config/kotki/models
```

Or supply your own path `scan("/path/to/registry.json")`.

## Performance / footprint

Translations are **fast** - Translating a simple sentence is generally **under** `10ms`
(except the first time, due to model loading). Translation models are loaded on-demand.
This means that model loading does not happen during `scan()` but during the first use
of `translate()` - which typically takes (only) `100ms` (per model). So if you have
a project that uses both `translate('foo', 'enfr')` and `translate('foo', 'fren')` - you'll be using 2 models (and consequently `~50MB` worth of RAM during the duration of your program).

Note that translations are done synchronously (and thus are 'blocking').

## Acknowledgements

This project was made possible through the combined effort of all researchers
and [partners](https://browser.mt/partners/) in the Bergamot project (Jerin Philip, et al). The [translation models](https://github.com/mozilla/firefox-translations/blob/main/extension/model/modelRegistry.js) are prepared as part of the Mozilla project. The translation engine used is [bergamot-translator](https://github.com/browsermt/bergamot-translator) which is based on [marian](https://github.com/browsermt/marian-dev).

## Bergamot-Translator

Kotki differs from Bergamot-Translator. The changes are specified below:

- Removed async/blocking worker pools
- Removed async/callback style translations
- Removed code related to parsing of HTML
- Work from a single JSON config file (`registry.json`)
- Dynamically generate marian configs 'on-the-fly'
- Simplified the example C++ CLI program (`src/demo/kotki.cpp`).
- Switch from [marian-dev](https://github.com/browsermt/marian-dev) to [marian-lite](https://github.com/kroketio/marian-lite)
- Simplified Python bindings
- Simplified the build system (cleaned up various CMakeLists.txt)
- Introduced automatic use of `ccache` for compilations
- Supply CMake configs for kotki (and its dependencies)
- Supply debian packaging for kotki (and its dependencies)
- Removed support for Apple, Microsoft, WASM (rip)
- Removed usage of proprietary libraries like CUDA, Intel MKL
- Removed unit tests
- Removed CI/CD definitions
- Introduced new dependency: rapidjson
- Doxygen, and other documentation removed

## License

MPL 2.0
