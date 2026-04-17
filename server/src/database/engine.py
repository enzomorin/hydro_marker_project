"""
Opens the SQLite file and provides the @with_session decorator
that all repository methods use.

Three SQLite PRAGMAs applied at startup:
  journal_mode=WAL    → antenna can write while dashboard reads simultaneously
  foreign_keys=ON     → SQLite enforces marker_id must exist in markers table
  synchronous=NORMAL  → safe against crashes, faster than the default FULL
"""

import os
from collections.abc import Callable
from functools import wraps
from typing import Any, TypeVar

from sqlmodel import Session, SQLModel, create_engine

Fct = TypeVar("Fct", bound=Callable[..., Any])

def build_engine(db_path: str):
    """
    Create a SQLite engine from a filesystem path.
    Creates the directory automatically if it does not exist.
    """
    os.makedirs(os.path.dirname(db_path) or ".", exist_ok=True)

    engine = create_engine(
        f"sqlite:///{db_path}",
        echo=False,
        connect_args={"check_same_thread": False},  # required for FastAPI's threading model
    )

    with engine.connect() as conn:
        conn.exec_driver_sql("PRAGMA journal_mode=WAL")
        conn.exec_driver_sql("PRAGMA foreign_keys=ON")
        conn.exec_driver_sql("PRAGMA synchronous=NORMAL")

    return engine


def init_db(engine: Any) -> None:
    """Create all tables defined with table=True in models.py (IF NOT EXISTS)."""
    SQLModel.metadata.create_all(engine)


def _open(engine: Any) -> Session:
    return Session(engine, expire_on_commit=False)


def with_session(commit: bool = True) -> Callable[[Fct], Fct]:
    """
    Decorator for repository methods.

    - Opens a fresh session before the method runs.
    - Commits on success  (if commit=True).
    - Rolls back on error (always).
    - Closes the session  (always).

    Usage:
        @with_session(commit=True)   ← INSERT / UPDATE / DELETE
        @with_session(commit=False)  ← SELECT  (nothing to save)
    """
    def decorator(function: Fct) -> Fct:
        @wraps(function)
        def wrapper(self: Any, *args: Any, **kwargs: Any) -> Any:
            with _open(self.engine) as session:
                try:
                    result = function(self, session, *args, **kwargs)
                    if commit:
                        session.commit()
                    return result
                except Exception:
                    session.rollback()
                    raise
        return wrapper
    return decorator