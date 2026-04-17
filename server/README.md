# HydroVinci — API Server

**HydroVinci API server** is an **API** secured by simple authentification system key. The **API** is based with **FastAPI** and deployed with **uvicorn** that handle the logic to store in a **SQLite database** the different markers around the world with his measure data associated:

---
 
## Table of contents
 
- [Overview](#overview)
- [Prerequisites](#prerequisites)
- [Setup](#setup)
- [Running the server](#running-the-server)
- [Authentication](#authentication)
- [Endpoints — Markers](#endpoints--markers)
- [Endpoints — Readings](#endpoints--readings)
- [Database fields](#database-fields)
- [Tech stack](#tech-stack)

---

## Overview
 
Each physical marker deployed in a river communicates sensor data to a ground antenna. The antenna receive the radio frames and pushes them to this API. The API stores everything in a local SQLite database and exposes endpoints to manage, and monitor all markers and their measurements.
 
---
 
## Prerequisites
 
- Python `>= 3.12`
- `pip`
- `git`
- `pyenv` recommended for managing Python versions on Fedora / Linux :)
---
 
## Setup
 
**1. Clone the repository**
 
```bash
git clone https://github.com/enzomorin/hydro_marker_project.git

cd hydro_marker_project/server
```
 
**2. Install Python 3.12**
 
```bash
pyenv install 3.12
pyenv local 3.12
```
 
**3. Create and activate a virtual environment**
 
```bash
python -m venv .venv
 
source .venv/bin/activate    # Linux / macOS
.venv\Scripts\activate       # Windows
```
 
**4. Install dependencies**
 
```bash
pip install -r requirements.txt
```
 
**5. Configure environment variables**
 
```bash
cp .env.example .env
```
 
Open `.env` and replace the placeholder values:
 
```env
DB_PATH=./data/markers.db
 
API_KEY=your_antenna_secret_key
ADMIN_KEY=your_admin_secret_key
 
APP_NAME=River Monitor API
DEBUG=true
```
 
> **Never commit `.env` to git** — it contains your secrets. It is already listed in `.gitignore`.
 
---
 
## Running the server
 
**Option 1 — Direct (recommended for development)**
 
```bash
python src/main.py
```
 
**Option 2 — Uvicorn directly**
 
```bash
python -m uvicorn src.main:app --host 127.0.0.1 --port 8000 --reload
```
 
The server is accessible at: `http://127.0.0.1:8000`
 
Interactive API documentation: `http://127.0.0.1:8000/docs`
 
---
 
## Authentication
 
Every request must include an `X-Api-Key` header. There are two access levels:
 
| Key | Header value | Access |
|---|---|---|
| Antenna key (`API_KEY`) | `X-Api-Key: your_antenna_secret_key` | Read data + push readings |
| Admin key (`ADMIN_KEY`) | `X-Api-Key: your_admin_secret_key` | Full access |
 
The antenna key is safe to flash into hardware firmware. If it is ever compromised, an attacker can only push fake readings — they cannot delete markers or wipe data.
 
```bash
# Example with antenna key
curl -H "X-Api-Key: your_antenna_secret_key" http://127.0.0.1:8000/api/v1/markers/
 
# Example with admin key
curl -H "X-Api-Key: your_admin_secret_key" http://127.0.0.1:8000/api/v1/markers/
```
 
---
 
## Endpoints — Markers
 
### `GET /api/v1/markers/`
Return the list of all active markers.
 
```bash
curl -H "X-Api-Key: your_antenna_secret_key" \
  http://127.0.0.1:8000/api/v1/markers/
```
 
Add `?include_deleted=true` to also return soft-deleted markers.
 
---
 
### `GET /api/v1/markers/{marker_id}`
Return one specific marker by its ID.
 
```bash
curl -H "X-Api-Key: your_antenna_secret_key" \
  http://127.0.0.1:8000/api/v1/markers/1
```
 
---
 
### `POST /api/v1/markers/` — Admin only
Register a new physical marker.
 
```bash
curl -X POST \
  -H "X-Api-Key: your_admin_secret_key" \
  -H "Content-Type: application/json" \
  -d '{"name": "Pont-Neuf", "location": "48.8566,2.3522"}' \
  http://127.0.0.1:8000/api/v1/markers/
```
 
| Field | Type | Required | Default | Description |
|---|---|---|---|---|
| `name` | string | ✓ | — | Human-readable label |
| `location` | string | ✓ | — | GPS coordinates as `"lat,lon"` |
| `radius` | integer | | `500` | Alert radius in metres |
| `is_active` | boolean | | `true` | Whether the marker is receiving data |
 
---
 
### `PATCH /api/v1/markers/{marker_id}` — Admin only
Update one or more fields of an existing marker. Only send the fields you want to change.
 
```bash
curl -X PATCH \
  -H "X-Api-Key: your_admin_secret_key" \
  -H "Content-Type: application/json" \
  -d '{"is_active": false}' \
  http://127.0.0.1:8000/api/v1/markers/1
```
 
---
 
### `DELETE /api/v1/markers/{marker_id}` — Admin only
Move a marker to the trash (soft delete — recoverable).
 
```bash
curl -X DELETE \
  -H "X-Api-Key: your_admin_secret_key" \
  http://127.0.0.1:8000/api/v1/markers/1
```
 
---
 
### `PUT /api/v1/markers/{marker_id}/restore` — Admin only
Recover a soft-deleted marker from the trash.
 
```bash
curl -X PUT \
  -H "X-Api-Key: your_admin_secret_key" \
  http://127.0.0.1:8000/api/v1/markers/1/restore
```
 
---
 
### `DELETE /api/v1/markers/{marker_id}/purge` — Admin only
Permanently delete a marker and **all its readings**. This is irreversible.
 
```bash
curl -X DELETE \
  -H "X-Api-Key: your_admin_secret_key" \
  http://127.0.0.1:8000/api/v1/markers/1/purge
```
 
---
 
## Endpoints — Readings
 
### `POST /api/v1/readings/`
Push a single sensor frame from the antenna.
 
```bash
curl -X POST \
  -H "X-Api-Key: your_antenna_secret_key" \
  -H "Content-Type: application/json" \
  -d '{
    "marker_id": 1,
    "water_level": 1.42,
    "pressure": 1013.0,
    "temperature": 14.5,
    "battery": 3.8
  }' \
  http://127.0.0.1:8000/api/v1/readings/
```
 
| Field | Type | Required | Description |
|---|---|---|---|
| `marker_id` | integer | ✓ | ID of the marker that sent this frame |
| `recorded_at` | datetime | | When the marker took the measurement (defaults to now) |
| `water_level` | float | | Water level in metres |
| `pressure` | float | | Atmospheric pressure in hPa |
| `temperature` | float | | Water temperature in °C |
| `battery` | float | | Battery voltage in V |
| `raw_payload` | string | | Original RF frame bytes, for debugging |
 
All sensor fields are optional — a partial frame is still stored.
 
---
 
### `POST /api/v1/readings/batch`
Push multiple frames in one request. Useful when the antenna was offline and has buffered frames. Either all frames are saved or none are (atomic transaction).
 
```bash
curl -X POST \
  -H "X-Api-Key: your_antenna_secret_key" \
  -H "Content-Type: application/json" \
  -d '[
    {"marker_id": 1, "water_level": 1.40, "temperature": 14.2},
    {"marker_id": 1, "water_level": 1.41, "temperature": 14.3},
    {"marker_id": 1, "water_level": 1.42, "temperature": 14.5}
  ]' \
  http://127.0.0.1:8000/api/v1/readings/batch
```
 
---
 
### `GET /api/v1/readings/latest?marker_id={id}`
Return the most recent reading for a specific marker. This is the endpoint to call for a live dashboard tile.
 
```bash
curl -H "X-Api-Key: your_antenna_secret_key" \
  "http://127.0.0.1:8000/api/v1/readings/latest?marker_id=1"
```
 
---
 
### `GET /api/v1/readings/`
Return a paginated list of readings with optional filters.
 
| Parameter | Type | Description |
|---|---|---|
| `marker_id` | integer | Filter by marker |
| `since` | ISO-8601 datetime | Lower bound on measurement time |
| `until` | ISO-8601 datetime | Upper bound on measurement time |
| `limit` | integer (1–1000) | Max rows to return, default `100` |
| `offset` | integer | Rows to skip, for pagination |
 
```bash
# All readings for marker 1
curl -H "X-Api-Key: your_antenna_secret_key" \
  "http://127.0.0.1:8000/api/v1/readings/?marker_id=1"
 
# Page 2 (rows 101–200)
curl -H "X-Api-Key: your_antenna_secret_key" \
  "http://127.0.0.1:8000/api/v1/readings/?marker_id=1&limit=100&offset=100"
```
 
---
 
### `GET /api/v1/readings/{reading_id}`
Return one specific reading by its numeric ID.
 
```bash
curl -H "X-Api-Key: your_antenna_secret_key" \
  http://127.0.0.1:8000/api/v1/readings/42
```
 
---
 
### `DELETE /api/v1/readings/{reading_id}` — Admin only
Permanently delete one reading. Use this to remove bad data from a faulty sensor or a test transmission.
 
```bash
curl -X DELETE \
  -H "X-Api-Key: your_admin_secret_key" \
  http://127.0.0.1:8000/api/v1/readings/42
```
 
---
 
## Database fields
 
### `markers` table
 
| Field | Type | Description |
|---|---|---|
| `id` | integer | Auto-assigned primary key |
| `name` | string | Human-readable label |
| `location` | string | GPS coordinates as `"lat,lon"` |
| `radius` | integer | Alert radius in metres |
| `is_active` | boolean | Whether the marker is currently active |
| `is_deleted` | boolean | Soft-delete flag |
| `created_at` | datetime | UTC timestamp of registration |
 
### `readings` table
 
| Field | Type | Description |
|---|---|---|
| `id` | integer | Auto-assigned primary key |
| `marker_id` | integer | Foreign key → `markers.id` |
| `recorded_at` | datetime | When the marker took the measurement |
| `received_at` | datetime | When the server received the frame |
| `water_level` | float | Water level in metres |
| `pressure` | float | Atmospheric pressure in hPa |
| `temperature` | float | Water temperature in °C |
| `battery` | float | Battery voltage in V |
| `raw_payload` | string | Original RF frame for debugging |
 
---
 
## Tech used
 
[![FastAPI](https://img.shields.io/badge/FastAPI-009485.svg?logo=fastapi&logoColor=white)](https://fastapi.tiangolo.com/)
[![SQLite](https://img.shields.io/badge/SQLite-%2307405e.svg?logo=sqlite&logoColor=white)](https://sqlite.org/)
[![Uvicorn](https://img.shields.io/badge/Uvicorn-5C2D91?style=flat&logoColor=white)](https://www.uvicorn.org/)
[![Python](https://img.shields.io/badge/Python-3.12-3776AB?logo=python&logoColor=white)](https://python.org/)
[![SQLModel](https://img.shields.io/badge/SQLModel-0.0.18-009485?logo=fastapi&logoColor=white)](https://sqlmodel.tiangolo.com/)