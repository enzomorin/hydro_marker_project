"""
HTTP endpoints for antenna ingest and data querying.

POST /readings/        → antenna pushes one frame
POST /readings/batch   → antenna pushes buffered frames (was offline)
GET  /readings/latest  → most recent value for a marker (live dashboard)
GET  /readings/        → paginated history with optional filters
GET  /readings/{id}    → one reading by primary key
DELETE /readings/{id}  → admin hard-delete
"""

from datetime import datetime
from typing import Optional

from fastapi import APIRouter, Depends, HTTPException, Query, status

from src.config.auth import require_admin_key, require_antenna_key
from src.database import MarkerRepository, ReadingRepository
from src.models import Reading, ReadingCreate, ReadingRead

router = APIRouter(prefix="/readings", tags=["Readings"])


def _r_repo() -> ReadingRepository:
    from src.main import reading_repo
    if reading_repo is None:
        raise RuntimeError("Server not fully started — reading_repo is None")
    return reading_repo


def _m_repo() -> MarkerRepository:
    from src.main import marker_repo
    if marker_repo is None:
        raise RuntimeError("Server not fully started — marker_repo is None")
    return marker_repo


def _assert_marker_active(marker_id: int, m_repo: MarkerRepository) -> None:
    """Shared guard — raises 404 or 409 if the marker cannot accept readings."""
    marker = m_repo.get(marker_id)
    if marker is None:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Marker {marker_id} not found",
        )
    if not marker.is_active:
        raise HTTPException(
            status_code=status.HTTP_409_CONFLICT,
            detail=f"Marker {marker_id} is inactive",
        )


# POST /api/v1/readings/
@router.post("/", response_model=ReadingRead, status_code=status.HTTP_201_CREATED)
async def ingest_reading(
    payload: ReadingCreate,
    _: None = Depends(require_antenna_key),
    r_repo: ReadingRepository = Depends(_r_repo),
    m_repo: MarkerRepository  = Depends(_m_repo),
):
    """Single frame from the antenna."""
    _assert_marker_active(payload.marker_id, m_repo)
    return r_repo.create(Reading(**payload.model_dump()))


# POST /api/v1/readings/batch
@router.post("/batch", response_model=list[ReadingRead], status_code=status.HTTP_201_CREATED)
async def ingest_batch(
    payload: list[ReadingCreate],
    _: None = Depends(require_antenna_key),
    r_repo: ReadingRepository = Depends(_r_repo),
    m_repo: MarkerRepository  = Depends(_m_repo),
):
    """Multiple frames in one request. All-or-nothing: if one fails, none are saved."""
    if not payload:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="Batch is empty")

    checked: set[int] = set()
    for item in payload:
        if item.marker_id not in checked:
            _assert_marker_active(item.marker_id, m_repo)
            checked.add(item.marker_id)

    return r_repo.create_batch([Reading(**item.model_dump()) for item in payload])


# GET /api/v1/readings/latest
# ⚠️ Must be defined BEFORE /{reading_id} — otherwise FastAPI reads
#    "latest" as an integer id and returns a 422 validation error.
@router.get("/latest", response_model=ReadingRead)
async def latest_reading(
    marker_id: int = Query(description="Marker ID"),
    _: None = Depends(require_antenna_key),
    r_repo: ReadingRepository = Depends(_r_repo),
):
    reading = r_repo.get_latest(marker_id)
    if reading is None:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"No readings found for marker {marker_id}",
        )
    return reading


# GET /api/v1/readings/
@router.get("/", response_model=list[ReadingRead])
async def list_readings(
    marker_id: Optional[int]      = Query(default=None),
    since:     Optional[datetime]  = Query(default=None, description="ISO-8601 lower bound"),
    until:     Optional[datetime]  = Query(default=None, description="ISO-8601 upper bound"),
    limit:     int                 = Query(default=100, ge=1, le=1000),
    offset:    int                 = Query(default=0, ge=0),
    _: None = Depends(require_antenna_key),
    r_repo: ReadingRepository = Depends(_r_repo),
):
    return r_repo.get_all(marker_id=marker_id, since=since, until=until, limit=limit, offset=offset)


# GET /api/v1/readings/{reading_id}
@router.get("/{reading_id}", response_model=ReadingRead)
async def get_reading(
    reading_id: int,
    _: None = Depends(require_antenna_key),
    r_repo: ReadingRepository = Depends(_r_repo),
):
    reading = r_repo.get(reading_id)
    if reading is None:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Reading not found")
    return reading


# DELETE /api/v1/readings/{reading_id}
@router.delete("/{reading_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_reading(
    reading_id: int,
    _: None = Depends(require_admin_key),
    r_repo: ReadingRepository = Depends(_r_repo),
):
    if not r_repo.delete(reading_id):
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Reading not found")