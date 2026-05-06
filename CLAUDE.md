# CLAUDE.md — Handover for Claude Code / agent work

This file orients future sessions on **RSVP Bookworm** firmware for the **Waveshare ESP32-S3-Touch-LCD-3.49** (172×640 QSPI + AXS15231B touch). Treat this as the project-specific source of truth alongside the code; **`README.md` may lag** (e.g. hardware button copy).

## What this repo is

- **Upstream base:** RSVP Nano — one-word RSVP reader, SD library, menus, OTA, optional USB MSC.
- **This fork adds Bookworm:** companion screen after boot (procedural creature, H/B meters, desk actions), NVS-backed state, optional ES8311 desk SFX, hatch/time integration with Wi‑Fi.
- **Primary target board:** `esp32-s3-r8-opi` (PlatformIO) with pins and panel geometry in `src/board/BoardConfig.h`.

## Build and flash

```bash
# Default env (USB MSC on; typical dev build)
pio run -e waveshare_esp32s3_usb_msc

# Bookworm dev menu extras (regenerate pet, +evolution)
pio run -e waveshare_esp32s3_usb_msc_bwdev

# Without USB MSC
pio run -e waveshare_esp32s3
```

Upload: `pio run -e waveshare_esp32s3_usb_msc -t upload` (device may need **BOOT + RESET** for download mode).

Monitor: `pio device monitor` (115200).

## Layout — where to look

| Area | Path | Notes |
|------|------|--------|
| App loop, states, menus, touch routing | `src/app/App.cpp`, `App.h`, `AppState.h` | Large file; search `handleTouch`, `setState`, `MenuScreen` |
| Hardware pins, TCA9554, battery | `src/board/BoardConfig.{h,cpp}` | Single board config; adapt other boards here |
| Display + companion UI | `src/display/DisplayManager.cpp` | Meters, menus, reader; AXS15231B panel |
| Panel / QSPI | `src/display/axs15231b.*` | Low-level display |
| Touch | `src/input/TouchHandler.cpp` | AXS15231B packet layout; landscape mapping + `UI_ROTATED_180` |
| Buttons | `src/input/ButtonHandler.*` | Active-low, pull-up |
| Bookworm sim + persistence | `src/bookworm/*` | `BookWormStore` NVS namespace `bworm` |
| Desk SFX (I2S + ES8311) | `src/audio/BookWormSound.*` | Init may need tuning vs Waveshare audio demos |
| Storage / books | `src/storage/StorageManager.*` | `/books` on SD |
| OTA | `src/update/OtaUpdater.h` + `docs/ota.conf.example` | |

## App state machine (critical)

- **`AppState::Companion`** — default home after boot (unless boot-to-book); touch uses **`handleCompanionTouch`** only.
- **`AppState::Menu`** — list UIs; touch uses **`applyMenuTouchGesture`**.
- **`AppState::Paused` / `Playing`** — reader; **`applyPausedTouchGesture`**.
- **`updateState`** early-returns while **`Companion`**; do not assume global transitions run there.

**Important:** Any UI that must respond to **menu-style taps** (e.g. Settings) must run with **`state_ == Menu`**. Calling `openSettings()` alone while still in `Companion` would draw settings but leave touch on the companion handler — broken UX. Use **`openSettingsFromPowerButton()`** (or equivalent `setState(Menu)` + correct `menuScreen_`) when opening settings from the companion.

## Hardware buttons (current firmware behavior)

- **BOOT (GPIO0):** short press — cycle brightness; long hold — cycle theme.
- **PWR (GPIO16 in `BoardConfig.h` — verify against schematic if button seems dead):**
  - Short press from **Companion / Paused / Playing:** open **Settings** (not generic main menu first).
  - Short press from **Menu** with **main** menu visible: open **Settings**.
  - Short press from **Menu** in a **submenu** (non-main `MenuScreen`): jump to **main menu**.
  - Long hold: power-off sequence (unchanged).

Exiting the app from the menu: use **Resume**, **Desk**, **Library**, etc.; there is no “PWR instantly returns to reader from main menu” shortcut in this fork.

## Touch / handedness

- Logical size **640×172** landscape; native panel dimensions and **`UI_ROTATED_180`** in `BoardConfig.h` affect coordinate transforms.
- Companion desk strip and creature hit targets live in **`handleCompanionTouch`** (`App.cpp`); changes to touch packing in **`TouchHandler`** must stay consistent with AXS15231B reports.

## Bookworm persistence

- Namespace **`bworm`** via **`Preferences`** (`BookWormStore`).
- Reading progress / reader prefs remain separate from Bookworm state.
- Optional **SNTP** after Wi‑Fi for hatch/clock; see `TimeService`.

## Tests (native)

```bash
pio test -e native_test
pio test -e native_bookworm_test
```

## Conventions for edits

- Prefer **small, task-scoped diffs**; match existing style (Arduino-ESP32, `String`, `constexpr` usage as in tree).
- **Do not** “drive-by” rewrite unrelated reader/menu code when fixing companion/touch/audio.
- If adding features that show lists, confirm **`AppState`** and **`handleTouch`** routing.
- C++ standard in env is **not** `-std=c++17`; avoid **`inline constexpr`** in headers unless build flags change (existing `BookWormConfig.h` may warn).

## docs / OTA / web

- **`docs/ota.conf.example`** — optional SD `/config/ota.conf`.
- **`web/`** — flasher / firmware assets; **`tools/`** — versioning, export helpers.

When landing changes, update **`README.md`** only if user-facing behavior or buttons change and docs should match.
