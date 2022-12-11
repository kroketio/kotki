# DEV

for internal development.

## Creating the tiny model archive

see `dl_slim.py`

## Creating the model archive 

Registry file can be found in [mozilla/firefox-translation](https://raw.githubusercontent.com/mozilla/firefox-translations/main/extension/model/modelRegistry.js).

```bash
wget https://raw.githubusercontent.com/mozilla/firefox-translations/main/extension/model/modelRegistry.js
```

lets generate a `registry.json`:

```bash
printf "$(cat modelRegistry.js)\n\n$(cat print.js)" | node > registry.json
```

lets generate a URL list:

```bash
printf "$(cat modelRegistry.js)\n\n$(cat urls.js)" | node > urls
```

Put `registry.json` into folder `0.3.3/`:

```bash
mkdir -p "0.3.3"
mv registry.json "0.3.3"
```

Download all URLs (all the models) into this folder.

```bash
cat urls | sort -u | parallel -j8 wget -qP "0.3.3"
```

Create the archive

```bash
zip -r kotki_models_0.3.3.zip "0.3.3"
```

Done.

## `generate_ssplit_bundle.py`

internal tool to generate `nb_prefix.h`
