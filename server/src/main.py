"""
Entry point. Wires everything together and starts the server.

Startup sequence (inside lifespan):
  1. Read settings from .env
  2. Open the SQLite file  (creates it + the data/ dir if needed)
  3. Run CREATE TABLE IF NOT EXISTS for every model
  4. Build the two repositories and attach them to this module
     so the API/ routers can import them without circular imports
  5. Yield → server handles requests
  6. Dispose the engine on shutdown
"""

import sys
import os

# Fix for "python src/main.py" — adds project root to sys.path
_project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if _project_root not in sys.path:
    sys.path.insert(0, _project_root)


from contextlib import asynccontextmanager

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from src.config.config import get_settings
from src.database import MarkerRepository, ReadingRepository, build_engine, init_db
from src.api import markers, reading


marker_repo:  MarkerRepository  | None = None
reading_repo: ReadingRepository | None = None


@asynccontextmanager
async def lifespan(app: FastAPI):
    cfg = get_settings()

    engine = build_engine(cfg.db_path)
    init_db(engine)

    import src.main as _self
    _self.marker_repo  = MarkerRepository(engine)
    _self.reading_repo = ReadingRepository(engine)

    yield

    engine.dispose()


cfg = get_settings()

app = FastAPI(
    title=cfg.app_name,
    description="REST API for river water-level markers connected via RF antenna.",
    version="1.0.0",
    debug=cfg.debug,
    lifespan=lifespan,
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:8080", "http://localhost:3000"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(markers.router, prefix="/api/v1")
app.include_router(reading.router, prefix="/api/v1")


@app.get("/health", tags=["Health"])
async def health():
    return {"status": "ok", "service": cfg.app_name}


if __name__ == "__main__":
    import uvicorn
    uvicorn.run("src.main:app", host="127.0.0.1", port=8000, reload=True)