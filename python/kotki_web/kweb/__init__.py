import os
import logging
from typing import List, Optional
from datetime import datetime

from quart import Quart, url_for
from quart_schema import QuartSchema

import kotki
from kweb.constants import model_lookup
import settings

if settings.KOTKI_REGISTRY:
    kotki.scan(settings.KOTKI_REGISTRY)
else:
    kotki.scan()

app: Optional[Quart] = None
model_names: List[str] = []


def create_app():
    global app
    app = Quart(__name__)
    app.config['MAX_CONTENT_LENGTH'] = 50 * 1000 * 1000
    app.secret_key = settings.APP_SECRET
    QuartSchema(app)

    if settings.DEBUG:
        app.logger.setLevel(logging.DEBUG)
        app.debug = True

    @app.context_processor
    def template_variables():
        return {
            'settings': settings,
            'now': datetime.now(),
            'model_names': model_names,
            'model_lookup': model_lookup,
            'API_ROUTE_TRANSLATE': url_for('bp_routes.api_translate', _external=True),
            'API_CONTENT_LIMIT': settings.API_CONTENT_LIMIT
        }

    @app.template_filter('name2lang')
    def name2lang(s):
        global model_lookup
        return f"{model_lookup[s[:2]]} -> {model_lookup[s[2:]]}"

    @app.before_serving
    async def startup():
        global model_names
        model_names = kotki.listModels()

        from kweb.routes import bp_routes
        app.register_blueprint(bp_routes)

    return app
