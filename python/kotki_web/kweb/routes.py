from dataclasses import dataclass
from typing import Optional, Union, Tuple
from datetime import timedelta

import kotki
from langdetect import detect
from quart import render_template, Blueprint, jsonify, Response
from quart_schema import validate_request

import settings
from kweb import model_names, model_lookup

bp_routes = Blueprint('bp_routes', __name__)


@bp_routes.get("/")
async def root():
    return await render_template('index.html')


@dataclass
class TranslationModel:
    translate: str
    model: Optional[str] = None


@dataclass
class TranslationModelResponse:
    translate: str
    model: str = None
    detected_code: Optional[str] = None
    detected: Optional[str] = None


@bp_routes.post("/api/1/translate")
@validate_request(TranslationModel)
async def api_translate(data: TranslationModel) -> Union[TranslationModelResponse, Tuple[Response, int]]:
    if len(data.translate) >= settings.API_CONTENT_LIMIT:
        raise Exception("text too long")

    resp = TranslationModelResponse(translate=data.translate)
    if not data.model:
        resp.detected_code = detect(data.translate)
        resp.detected = model_lookup[resp.detected_code]
        resp.model = f"{resp.detected_code}en"
    else:
        resp.model = data.model

    if resp.model[:2] == resp.model[2:]:
        resp.translate = data.translate
        return resp

    if resp.model not in model_names:
        msg = ""
        if not data.model:
            msg = "Auto-detect error: "
        return jsonify({'error': f"{msg}unknown translation model id '{resp.model}'"}), 500

    resp.translate = kotki.translate(data.translate, resp.model)
    return resp
