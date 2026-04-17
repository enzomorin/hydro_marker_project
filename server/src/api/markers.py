"""
HTTP endpoints for managing physical markers.

Read endpoints  (GET)    → antenna key or admin key
Write endpoints (everything else) → admin key only

All routes are mounted under /api/v1/markers/ by main.py.
"""

from fastapi import APIRouter, Depends, HTTPException, status

from src.config.auth import require_admin_key, require_antenna_key
from src.database import MarkerRepository
from src.models import Marker, MarkerCreate, MarkerRead, MarkerUpdate

router = APIRouter(prefix="/markers", tags=["Markers"])


def _repo() -> MarkerRepository:
    from src.main import marker_repo
    if marker_repo is None:
        raise RuntimeError("Server not fully started — marker_repo is None")
    return marker_repo


# GET /api/v1/markers/
@router.get("/", response_model=list[MarkerRead])
async def list_markers(
    include_deleted: bool = False,
    _: None = Depends(require_antenna_key),
    repo: MarkerRepository = Depends(_repo),
):
    return repo.get_all(include_deleted=include_deleted)


# GET /api/v1/markers/{marker_id}
@router.get("/{marker_id}", response_model=MarkerRead)
async def get_marker(
    marker_id: int,
    include_deleted: bool = False,
    _: None = Depends(require_antenna_key),
    repo: MarkerRepository = Depends(_repo),
):
    marker = repo.get(marker_id, include_deleted=include_deleted)
    if marker is None:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Marker not found")
    return marker


# POST /api/v1/markers/
@router.post("/", response_model=MarkerRead, status_code=status.HTTP_201_CREATED)
async def create_marker(
    payload: MarkerCreate,
    _: None = Depends(require_admin_key),
    repo: MarkerRepository = Depends(_repo),
):
    return repo.create(Marker(**payload.model_dump()))


# PATCH /api/v1/markers/{marker_id}
@router.patch("/{marker_id}", response_model=MarkerRead)
async def update_marker(
    marker_id: int,
    payload: MarkerUpdate,
    _: None = Depends(require_admin_key),
    repo: MarkerRepository = Depends(_repo),
):
    marker = repo.update(marker_id, payload)
    if marker is None:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Marker not found")
    return marker


# DELETE /api/v1/markers/{marker_id}   → soft delete (recoverable)
@router.delete("/{marker_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_marker(
    marker_id: int,
    _: None = Depends(require_admin_key),
    repo: MarkerRepository = Depends(_repo),
):
    if not repo.soft_delete(marker_id):
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Marker not found")


# PUT /api/v1/markers/{marker_id}/restore
@router.put("/{marker_id}/restore", response_model=MarkerRead)
async def restore_marker(
    marker_id: int,
    _: None = Depends(require_admin_key),
    repo: MarkerRepository = Depends(_repo),
):
    if not repo.restore(marker_id):
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Marker not found or not in trash",
        )
    return repo.get(marker_id)


# DELETE /api/v1/markers/{marker_id}/purge  → hard delete (irreversible)
@router.delete("/{marker_id}/purge", status_code=status.HTTP_204_NO_CONTENT)
async def purge_marker(
    marker_id: int,
    _: None = Depends(require_admin_key),
    repo: MarkerRepository = Depends(_repo),
):
    if not repo.hard_delete(marker_id):
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Marker not found")