# hydro_marker_project

A beacon placed in small rivers to measure flooding risk, hydric stress, or normal conditions — built for the **Olympiads of Engineering Sciences**.

---

## Project overview

Each marker is a waterproof device deployed in a river. It measures water level, pressure, temperature, and battery voltage, then transmits the data via radio to a ground antenna. The antenna forwards the data to the API server, which stores it and makes it available to a dashboard and a mobile alert application.

---

## Repository structure

```
hydro_marker_project/
│
├── docs/           Documentation, source references, notes, and diagrams
│
├── app/            Dashboard application
│                   Receives data from the ground antenna, processes it,
│                   and sends it to the API. Also includes a companion app
│                   that sends alerts when a nearby marker detects potential
│                   flooding or hydric stress.
│
├── firmware/       Embedded code
│                   MCU code for the marker (sensor reading + radio transmission)
│                   and code for the ground station (receiving + forwarding frames).
│
├── hardware/       PCB design files (KiCad)
│                   Optimised circuit board for the markers — reduces electrical
│                   noise and improves efficiency.
│
├── server/         API server
│                   FastAPI + SQLite REST API that stores marker locations
│                   and all associated measurement data.
│
├── site/           Promotional website
│                   Presents the product and its specifications.
│                   Also displays live marker data and locations on a map.
│
└── tools/          Development utilities
                    Scripts and helpers that speed up development across
                    all parts of the project.
```

---

## Quick start

To run the API server locally, see [`server/README.md`](server/README.md).

---

## Tech used overview

| Component | Technology |
|---|---|
| API server | Python, FastAPI, SQLite |
| Firmware | C++ (MCU), PlatfromIO IDE |
| PCB design | KiCad |
| Dashboard | Electron, Javascript |
| Website | HTML, CSS, Javascript |