from fastapi import FastAPI
from firebase_admin import initialize_app
from firebase_functions import https_fn
from a2wsgi import ASGIMiddleware
import os

# Optional: point to Auth emulator
os.environ["FIREBASE_AUTH_EMULATOR_HOST"] = "localhost:9099"

# Optional: force watchdog to use polling (avoids _ThreadHandle crash on Python 3.13)
os.environ["WATCHDOG_USE_POLLING"] = "true"

# Optional: disable Flask debug reloader if somehow used
os.environ["FLASK_ENV"] = "production"

try:
    initialize_app()
except ValueError:
    pass

# FastAPI app
app = FastAPI()

@app.get("/")
def root():
    return {"message": "FastAPI working via Firebase Emulator!"}

@app.get("/ping")
def ping():
    return {"ping": "pong"}

# Wrap FastAPI in WSGI adapter for Firebase
wrapped_app = ASGIMiddleware(app)

@https_fn.on_request()
def api_entrypoint(request):
    return wrapped_app(request.environ, lambda *args: None)
