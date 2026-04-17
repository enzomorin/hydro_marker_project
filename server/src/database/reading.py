"""
All SQL operations for the readings table.
"""

from datetime import datetime
from typing import Optional

from sqlmodel import desc, select

from src.database.engine import with_session
from src.models import Reading


class ReadingRepository:

    def __init__(self, engine):
        self.engine = engine

    # ── READ ──────────────────────────────────────────────────────────────────

    @with_session(commit=False)
    def get_all(
        self,
        session,
        marker_id:  Optional[int]     = None,
        since:      Optional[datetime] = None,
        until:      Optional[datetime] = None,
        limit:      int                = 100,
        offset:     int                = 0,
    ) -> list[Reading]:
        q = select(Reading)
        if marker_id is not None:
            q = q.where(Reading.marker_id == marker_id)
        if since is not None:
            q = q.where(Reading.recorded_at >= since)
        if until is not None:
            q = q.where(Reading.recorded_at <= until)
        q = q.order_by(desc(Reading.recorded_at)).limit(limit).offset(offset)
        return list(session.exec(q).all())

    @with_session(commit=False)
    def get(self, session, reading_id: int) -> Optional[Reading]:
        return session.get(Reading, reading_id)

    @with_session(commit=False)
    def get_latest(self, session, marker_id: int) -> Optional[Reading]:
        """Most recent reading for a given marker — useful for a live dashboard."""
        q = (
            select(Reading)
            .where(Reading.marker_id == marker_id)
            .order_by(desc(Reading.recorded_at))
            .limit(1)
        )
        return session.exec(q).first()

    # ── WRITE ─────────────────────────────────────────────────────────────────

    @with_session(commit=True)
    def create(self, session, reading: Reading) -> Reading:
        session.add(reading)
        session.flush()
        session.refresh(reading)
        return reading

    @with_session(commit=True)
    def create_batch(self, session, readings: list[Reading]) -> list[Reading]:
        """Insert multiple readings in one atomic transaction."""
        for r in readings:
            session.add(r)
        session.flush()
        for r in readings:
            session.refresh(r)
        return readings

    @with_session(commit=True)
    def delete(self, session, reading_id: int) -> bool:
        reading = session.get(Reading, reading_id)
        if reading is None:
            return False
        session.delete(reading)
        return True