![https://i.imgur.com/2alDAhA.png](https://i.imgur.com/2alDAhA.png)

# kotki

[![License: MPL v2](https://img.shields.io/badge/License-MPL%20v2-blue.svg)](https://www.mozilla.org/en-US/MPL/2.0/)

This package provides Python bindings for kotki, the Bergamot-based translation engine. Fast & easy 
language translations inside an easy to install Python module.

```python3
import kotki

kotki.loadRegistry("/home/user/example/registry.json")

kotki.translate("Whenever I am at the office, I like to drink coffee", "ende")
'Wann immer ich im Büro bin, trinke ich gerne Kaffee'

kotki.translate("Румънците получиха дълго чакани новини: пенсиите и минималната заплата ще бъдат увеличени от 2023 г.", "bgen")
'Romanians have received long-awaited news: pensions and minimum wages will be increased from 2023'

kotki.translate("jij bent geboren in de stad Den Haag.", "nlen")
'You were born in The Hague.'
```

## Requirements

Lets grab some requirements before we do `pip install`.

```bash
apt install -y ccache rapidjson-dev cmake libpcre++-dev libpcre2-dev python3-dev pybind11-dev
```

Get MKL installed. For example, this does the installation on Ubuntu 21.

```bash
wget -qO- 'https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB' | sudo apt-key add -
sudo sh -c 'echo deb https://apt.repos.intel.com/mkl all main > /etc/apt/sources.list.d/intel-mkl.list'
sudo apt update
sudo apt install -y intel-mkl-64bit-2020.4-912
```

In case you cannot find the correct version to install, you may use the package
manager to search for the correct package name: `apt search intel-mkl-64bit-2020`

```bash
pip install kotki -v
```

##### Compile times

- AMD Ryzen 7 4700U - 4c/8t 32GB RAM - 2min
- i7-1165G7 - 4c/8t 32GB RAM - 3min
- VPS at OVH - 4c 8GB RAM - 8min

## API

The API is straight-forward and contains only 3 functions:

- `loadRegistry(path)`, read available translation models
- `translate(text, language)`, translate some text
- `listModels()`, list available translation models

## Models

The translation models that kotki uses are 'borrowed' from the 
Mozilla [Firefox Translations](https://addons.mozilla.org/en-US/firefox/addon/firefox-translations/) 
extension. **You need to manually download these models.** They are conveniently packaged into a single 
archive that can be downloaded at [github.com/kroketio/kotki/releases](https://github.com/kroketio/kotki/releases). 

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

Note that translations are done synchronously (and thus are 'blocking'). If you need
an async/callback style approach, look at the [Bergamot-Translator](https://github.com/browsermt/bergamot-translator/).

## Acknowledgements

This project was made possible through the combined effort of all researchers
and [partners](https://browser.mt/partners/) in the Bergamot project (Jerin Philip, et al). The
[translation models](https://github.com/mozilla/firefox-translations/blob/main/extension/model/modelRegistry.js) are 
prepared as part of the Mozilla project. The translation engine used is
[bergamot-translator](https://github.com/browsermt/bergamot-translator) which
is based on [marian](https://github.com/browsermt/marian-dev).

## License

MPL 2.0