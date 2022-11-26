for (const lang in modelRegistry) {
    for (const bla in modelRegistry[lang]) {
        let url = `https://storage.googleapis.com/bergamot-models-sandbox/${modelRegistryVersion}/${lang}/${modelRegistry[lang][bla]['name']}`;
        console.log(url);
    }
}
