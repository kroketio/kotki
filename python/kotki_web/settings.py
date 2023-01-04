# SPDX-License-Identifier: MPL-2.0
# SPDX-FileCopyrightText: 2022 Kroket Ltd. <code@kroket.io>

import os
cwd = os.path.dirname(os.path.realpath(__file__))


def bool_env(val):
    return val is True or (isinstance(val, str) and (val.lower() == 'true' or val == '1'))


DEBUG = bool_env(os.environ.get("DEBUG", "false"))
HOST = os.environ.get("HOST", "127.0.0.1")
PORT = int(os.environ.get("PORT", 7000))
APP_SECRET = os.environ.get("APP_SECRET", os.urandom(24))
KOTKI_REGISTRY = os.environ.get("KOTKI_REGISTRY")
API_CONTENT_LIMIT = int(os.environ.get("API_CONTENT_LIMIT", 2500))  # chars
