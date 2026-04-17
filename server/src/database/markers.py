"""
All SQL operations for the markers table.
Nothing here knows about HTTP — it only speaks to the database.
"""

from typing import Optional

from sqlmodel import desc, select

from src.database.engine import with_session
from src.models import Marker, MarkerUpdate


class MarkerRepository:
    _IMMUTABLE = {"id", "created_at"}  # fields the API can never overwrite

    def __init__(self, engine):
        self.engine = engine

    # ── READ ──────────────────────────────────────────────────────────────────

    @with_session(commit=False)
    def get_all(self, session, include_deleted: bool = False) -> list[Marker]:
        q = select(Marker)
        if not include_deleted:
            q = q.where(Marker.is_deleted == False)
        return list(session.exec(q.order_by(desc(Marker.created_at))).all())

    @with_session(commit=False)
    def get(self, session, marker_id: int, include_deleted: bool = False) -> Optional[Marker]:
        marker = session.get(Marker, marker_id)
        if marker is None:
            return None
        if marker.is_deleted and not include_deleted:
            return None
        return marker

    # ── WRITE ─────────────────────────────────────────────────────────────────

    @with_session(commit=True)
    def create(self, session, marker: Marker) -> Marker:
        session.add(marker)
        session.flush()
        session.refresh(marker)
        return marker

    @with_session(commit=True)
    def update(self, session, marker_id: int, data: MarkerUpdate) -> Optional[Marker]:
        marker = session.get(Marker, marker_id)
        if marker is None or marker.is_deleted:
            return None
        for key, value in data.model_dump(exclude_unset=True).items():
            if key not in self._IMMUTABLE:
                setattr(marker, key, value)
        session.add(marker)
        session.flush()
        session.refresh(marker)
        return marker

    @with_session(commit=True)
    def soft_delete(self, session, marker_id: int) -> bool:
        """Set is_deleted=True. Row stays in DB and can be restored."""
        marker = session.get(Marker, marker_id)
        if marker is None or marker.is_deleted:
            return False
        marker.is_deleted = True
        return True

    @with_session(commit=True)
    def restore(self, session, marker_id: int) -> bool:
        """Pull a soft-deleted marker back out of the trash."""
        marker = session.get(Marker, marker_id)
        if marker is None or not marker.is_deleted:
            return False
        marker.is_deleted = False
        return True

    @with_session(commit=True)
    def hard_delete(self, session, marker_id: int) -> bool:
        """
        Permanently delete a marker and ALL its readings.
        Child readings are deleted first to satisfy the FK constraint,
        then the parent marker row is removed.
        """
        marker = session.get(Marker, marker_id)
        if marker is None:
            return False

        from src.models import Reading
        readings = session.exec(
            select(Reading).where(Reading.marker_id == marker_id)
        ).all()
        for r in readings:
            session.delete(r)

        session.flush()
        session.delete(marker)
        return True