"""
All table definitions and JSON shapes live here.

Each entity is split into classes:
    XxxBase    → shared fields
    Xxx        → the real DB table   (table=True, adds id + timestamps)
    XxxCreate  → what the API accepts as input  (no id, server fills it)
    XxxUpdate  → partial update body (every field Optional for PATCH)
    XxxRead    → what the API returns as output (includes id + timestamps)
"""

from datetime import datetime, timezone
from typing import Any, Optional

from sqlmodel import Field, Relationship, SQLModel


# ══════════════════════════════════════════════════════════════════════════════
#  MARKER  –  one physical device deployed in the river
# ══════════════════════════════════════════════════════════════════════════════

class MarkerBase(SQLModel):
    name:       str  = Field(index=True)
    location:   str  = Field(description="GPS as 'lat,lon'")
    radius:     int  = Field(default=500, description="Alert radius in metres")
    is_active:  bool = Field(default=True,  index=True)
    is_deleted: bool = Field(default=False, index=True)


class Marker(MarkerBase, table=True):
    __tablename__: Any = "markers"

    id:         Optional[int] = Field(default=None, primary_key=True)
    created_at: datetime      = Field(default_factory=lambda: datetime.now(timezone.utc))

    readings: list["Reading"] = Relationship(back_populates="marker")


class MarkerCreate(MarkerBase):
    pass


class MarkerUpdate(SQLModel):
    name:      Optional[str]  = None
    location:  Optional[str]  = None
    radius:    Optional[int]  = None
    is_active: Optional[bool] = None


class MarkerRead(MarkerBase):
    id:         int
    created_at: datetime


# ══════════════════════════════════════════════════════════════════════════════
#  READING  –  one sensor frame received from a marker via the antenna
# ══════════════════════════════════════════════════════════════════════════════

class ReadingBase(SQLModel):
    marker_id:   int      = Field(foreign_key="markers.id", index=True)
    recorded_at: datetime = Field(
        default_factory=lambda: datetime.now(timezone.utc),
        index=True,
        description="When the marker measured this value",
    )
    # Sensor fields – all optional so partial frames are still stored
    water_level: Optional[float] = Field(default=None, description="Water level in metres")
    pressure:    Optional[float] = Field(default=None, description="Pressure in hPa")
    temperature: Optional[float] = Field(default=None, description="Temperature in °C")
    battery:     Optional[float] = Field(default=None, description="Battery voltage in V")
    raw_payload: Optional[str]   = Field(default=None, description="Original RF frame for debugging")


class Reading(ReadingBase, table=True):
    __tablename__: Any = "readings"

    id:          Optional[int] = Field(default=None, primary_key=True)
    received_at: datetime      = Field(
        default_factory=lambda: datetime.now(timezone.utc),
        description="When the API server received this frame",
    )

    marker: Optional[Marker] = Relationship(back_populates="readings")


class ReadingCreate(ReadingBase):
    pass


class ReadingRead(ReadingBase):
    id:          int
    received_at: datetime