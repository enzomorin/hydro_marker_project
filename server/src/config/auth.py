"""
FastAPI dependencies that check the API key on every request.

Two tiers:
  require_antenna_key → read data + post readings  (antenna firmware uses this)
  require_admin_key   → full control               (only you use this)

Usage in a router:
    @router.post("/")
    async def my_route(_: None = Depends(require_admin_key)):
        ...
"""

from fastapi import Header, HTTPException, status
from src.config.config import get_settings


def _reject():
    raise HTTPException(
        status_code=status.HTTP_401_UNAUTHORIZED,
        detail="Invalid or missing API key",
        headers={"WWW-Authenticate": "X-Api-Key"},
    )


async def require_antenna_key(x_api_key: str | None = Header(default=None)) -> None:
    cfg = get_settings()
    if x_api_key not in (cfg.api_key, cfg.admin_key):
        _reject()


async def require_admin_key(x_api_key: str | None = Header(default=None)) -> None:
    cfg = get_settings()
    if x_api_key != cfg.admin_key:
        _reject()