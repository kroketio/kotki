import sys
from pathlib import Path
sys.path.append(str(Path(__file__).parent))

import click

import settings
from kweb import create_app


@click.command()
@click.option('-h', '--host', 'host', default=settings.HOST, required=True, help=f"bind host (default: {settings.HOST})")
@click.option('-p', '--port', 'port', default=settings.PORT, required=True, help=f"bind port (default: {settings.PORT})")
@click.option('-d', '--debug', 'dbg', is_flag=True, default=False, help='run Quart web-framework in debug')
@click.option('-r', '--registry', 'registry', required=False, type=click.File('r', lazy=True),
              help='Path to registry.json. Leave empty for auto-detection of translation models.')
def kotki_web_handler(host, port, dbg, registry):
    """
    Exposes kotki via HTTP web interface and provide an API.
    """
    settings.HOST = host
    settings.PORT = port
    settings.DEBUG = dbg

    if settings.DEBUG:
        from asyncio import get_event_loop
        loop = get_event_loop()
        loop.set_debug(True)
    if registry:
        settings.KOTKI_REGISTRY = registry

    app = create_app()
    app.run(settings.HOST, port=settings.PORT, debug=settings.DEBUG, use_reloader=False)
