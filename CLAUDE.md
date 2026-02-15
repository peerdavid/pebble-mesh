# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Pebble Mesh is a watchface for Pebble smartwatches written in C (watch-side) and JavaScript (phone-side). It displays time, date, step count, battery level, and real-time weather. It targets three Pebble platforms: aplite, basalt, and diorite.

## Build Commands

```bash
pebble build          # Compile for all platforms
pebble clean          # Clean build artifacts
pebble install        # Deploy to connected watch or emulator
pebble logs           # View watch logs
pebble analyze-size   # Analyze binary size
```

There is no test suite or linter configured for this project.

## Architecture

### C (Watch-side) — `src/c/`

- **pebble-mesh.c** — Main entry point. Handles window lifecycle, UI rendering, mesh border animation (frame-based at 50ms intervals, 500 frames), time/date display, and the typewriter text animation.
- **config.c / config.h** — Persistent storage for settings, theme management (dark/light/dynamic sunrise-sunset/dynamic quiet-time), and AppMessage inbox handler for receiving config from phone.
- **weather.c / weather.h** — Weather display: maps WMO condition codes to icons, renders temperature and location text.
- **steps.c / steps.h** — Step count display using Pebble Health API, progress bar rendering against configurable daily goal.
- **battery.c / battery.h** — Battery status icon and percentage display.

The display layout uses four corner quadrants: weather icon (upper-left), temperature+location (upper-right), steps+progress (lower-left), battery (lower-right).

### JavaScript (Phone-side) — `src/pkjs/`

- **index.js** — Handles weather fetching (Open-Meteo API for weather/sunrise-sunset, BigDataCloud for reverse geocoding), GPS location, and relays settings to the watch via AppMessage keys.
- **config.js** — Pebble Clay settings UI schema definition (theme, step goal, temperature unit, animations, location).

### Watch↔Phone Communication

Uses Pebble AppMessage with keys defined in `package.json` under `messageKeys`: `WEATHER_TEMPERATURE`, `WEATHER_LOCATION`, `WEATHER_CONDITION`, `WEATHER_IS_DAY`, `WEATHER_REQUEST`, `WEATHER_LOCATION_CONFIG`, `COLOR_THEME`, `STEP_GOAL`, `TEMPERATURE_UNIT`, `ENABLE_ANIMATIONS`.

### Resources

Weather icons, step icon, and battery icon each have light/dark variants in `resources/images/`. All resources are declared in `package.json` under `resources.media`.

## Key Details

- Uses Pebble SDK 3 with WAF build system (`wscript`)
- Settings UI powered by `pebble-clay` dependency
- Weather refreshes every 30 minutes
- Debug logging uses `APP_LOG()` macro
