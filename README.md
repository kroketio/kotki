![https://i.imgur.com/2alDAhA.png](https://i.imgur.com/2alDAhA.png)

# kotki

[![License: MPL v2](https://img.shields.io/badge/License-MPL%20v2-blue.svg)](https://www.mozilla.org/en-US/MPL/2.0/)

Simple and fast language translations without using the cloud.

Translation models borrowed from the Mozilla extension ['Firefox Translations'](https://addons.mozilla.org/en-US/firefox/addon/firefox-translations/). 

Built on top of [Bergamot](https://browser.mt/) and 
[Marian](https://github.com/kroketio/marian-lite/) - efficient Neural Machine Translation framework written in C++. 

100% FOSS.

## Web instances ([source](https://github.com/kroketio/kotki-web))

- [kotki.kroket.io](https://kotki.kroket.io)

## Example

Kotki is a C++ library but Python bindings are available:

```python3
import kotki
kotki.loadRegistry("/home/user/example/registry.json")

kotki.translate("Whenever I am at the office, I like to drink coffee.", "ende")
'Wann immer ich im büro bin, trinke ich gerne kaffee.'

kotki.translate("Румънците получиха дълго чакани новини: пенсиите и минималната заплата ще бъдат увеличени от 2023 г.", "bgen")
'Romanians have received long-awaited news: pensions and minimum wages will be increased from 2023'

kotki.translate("jij bent geboren in de stad Den Haag.", "nlen")
'You were born in The Hague.'
```

### C++

```cpp
#include <string>
#include "kotki/kotki.h"

using namespace std;
int main(int argc, char *argv[]) {
  auto *kotki = new Kotki();
  kotki->loadRegistryFromFile('/home/user/example/models/registry.json');
  // std::vector<string> langs = kotki->listModels();  // list languages
  cout << kotki->translate("This should work, in theory.", "ennl");  // English to Dutch
  return 0;
}
```

## why

Other translation frameworks are often big, difficult to compile, non-performant.

Kotki is a modified [Bergamot-Translator](https://github.com/browsermt/bergamot-translator/) aimed
at ease-of-use for developers who want to translate some text in their C++ or Python
applications without too much headache. Kotki provides a dead-simple API.

The translation models are provided on the [kotki/releases](https://github.com/kroketio/kotki/releases) page.

## Requirements

```bash
apt install -y cmake ccache pkg-config rapidjson-dev pybind11-dev libyaml-cpp-dev 
```

## Building the Python module

Note: the above system packages are needed to compile via `pip`.

```bash
pip install kotki -v
```

Compile times:

- AMD Ryzen 9 5900x - 12c/24t 64GB RAM - 30sec
- AMD Ryzen 7 4700U - 4c/8t 32GB RAM - 1.5min
- i7-1165G7 - 4c/8t 32GB RAM - 1.5min
- random VPS at OVH - 4c 8GB RAM - 4min

## Building the library from source

When building from source (to produce `libkotki.so` or `libkotki.a`, or both) we will need to 
install [marian-lite](https://github.com/kroketio/marian-lite) (and its dependencies) manually.

When that is done, some CMake options are available:

- `STATIC` - Produce static binary (TODO: doesn't work yet)
- `SHARED` - Produce shared binary
- `BUILD_DEMO` - Produce example demo application(s)

```bash
cmake -DBUILD_DEMO=ON -DSTATIC=OFF -DSHARED=ON -Bbuild .
make -Cbuild -j6
sudo make -Cbuild install  # install into /usr/local/...
```

In your own CMake application, find kotki via pkconfig:

```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(KOTKI REQUIRED kotki)

message(STATUS "kotki libraries: ${KOTKI_LIBRARIES}")
message(STATUS "kotki include dirs: ${KOTKI_INCLUDE_DIRS}")

target_link_libraries(myapp PUBLIC ${KOTKI_LIBRARIES})
target_include_directories(myapp PUBLIC ${KOTKI_INCLUDE_DIRS})
```

## Direct usage via CMake

For direct use in a C++ project (via `add_subdirectory()`) you would link 
against `kotki-lib`. See `src/demo/kotki.cpp` and `src/CMakeLists.txt` for reference.

## Models

The translation models are borrowed from the
Mozilla [Firefox Translations](https://addons.mozilla.org/en-US/firefox/addon/firefox-translations/)
extension. **You need to manually download these models.** They are conveniently packaged into a single
archive that can be downloaded at [kotki/releases](https://github.com/kroketio/kotki/releases).

`registry.json` is included in this archive - which is needed for the `loadRegistry()` call.

## Performance / footprint

Translations are **fast** - (probably) faster than other Python packages that do 
language translation. Translating a simple sentence is
usually **under** `10ms` (except the first time, due to model loading). Loading a
single translation model seems to take up around `40MB` in RAM.

Translation models are loaded on-demand. This means that model
loading does not happen during `loadRegistry()` but during the first use
of `translate()` - which typically takes (only) `100ms` (per model). So if you have
a project that uses both `translate('foo', 'enfr')` and `translate('foo', 'fren')` - you'll be using 2
models (and consequently `80MB` worth of RAM during the duration of your program).

Note that translations are done synchronously (and thus are 'blocking').

## Acknowledgements

This project was made possible through the combined effort of all researchers
and [partners](https://browser.mt/partners/) in the Bergamot project (Jerin Philip, et al). The
[translation models](https://github.com/mozilla/firefox-translations/blob/main/extension/model/modelRegistry.js) are
prepared as part of the Mozilla project. The translation engine used is
[bergamot-translator](https://github.com/browsermt/bergamot-translator) which
is based on [marian](https://github.com/browsermt/marian-dev).

## Bergamot-Translator

Kotki differs from Bergamot-Translator. The changes are specified below:

- Removed async/blocking worker pools
- Removed async/callback style translations
- Removed code related to parsing of HTML
- Work from a single JSON config file (`registry.json`)
- Dynamically generate marian configs 'on-the-fly'
- Simplified the example C++ CLI program (`app/kotki.cpp`).
- Switch from [marian-dev](https://github.com/browsermt/marian-dev) to [marian-lite](https://github.com/kroketio/marian-lite)
- Simplified Python bindings (expose only 3 functions)
- Simplified the build system (cleaned up various CMakeLists.txt)
- Introduced automatic use of `ccache` for compilations
- Removed support for Apple, Microsoft, WASM (rip)
- Removed usage of proprietary libraries like CUDA, Intel MKL
- Removed unit tests
- Removed CI/CD definitions
- Introduced new dependency: rapidjson
- Doxygen, and other documentation removed

## License

MPL 2.0