from typing import Optional
import sys
import json
import logging

from langdetect import detect
import kotki
import click


def loadKotki(path: Optional[str] = None):
    msg = "kotki.load("
    if path:
        msg += f"{path})"
        logging.debug(msg)
        kotki.scan(path.name)
    else:
        msg += ")"
        logging.debug(msg)
        kotki.scan()


@click.command()
@click.option('-i', '--input', 'inp', required=True, help='Text to translate')
@click.option('-m', '--model', 'model', required=False, default='auto', help='Model names. Use -l to list. Leave empty to guess the input language automatically.')
@click.option('-r', '--registry', 'registry', required=False, type=click.File('r', lazy=True), help='Path to registry.json. Leave empty for auto-detection of translation models.')
@click.option('-l', '--list', 'lst', is_flag=True, default=False, help='List available models.')
@click.option('-d', '--debug', 'dbg', is_flag=True, default=False, help='Print debug log.')
def kotki_cli_handler(inp, model, lst, dbg, registry):
    """
    Translate some text.

    Note: repeatedly calling this application is a waste of CPU cycles. Instead,
    make a Python script that imports and uses kotki.
    """
    if dbg:
        logging.basicConfig(level=logging.DEBUG)

    if lst:
        logging.debug("--list supplied, listing model entries")
        loadKotki(registry)
        print(json.dumps(kotki.listModels(), indent=4, sort_keys=True))
        return

    if inp == '-':
        logging.debug("reading from stdin because '-' was supplied")
        inp = sys.stdin.read()

    if not inp:
        logging.error("Empty input?")
        return

    logging.debug(f"input count: {len(inp)}")

    if not model or model == "auto":
        detected_code = detect(inp)
        logging.debug(f"guessing input language: {detected_code}; model: '{detected_code}en'")
        model = f"{detected_code}en"

    # scan available translation models
    loadKotki(registry)

    if model and model not in kotki.listModels():
        logging.error(f"model '{model}' not found")
        return

    translated = kotki.translate(inp, model)
    sys.stdout.write(translated)


if __name__ == '__main__':
    kotki_cli_handler()
