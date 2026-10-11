# FBFrameCounter

Open-source frame-window analyzer for Geometry Dash (Geode, C++23).

## Build
Local: `export GEODE_SDK=/path/to/geode && cmake -B build && cmake --build build --config RelWithDebInfo`.
CI: `.github/workflows/build.yml` builds Windows, macOS (universal), iOS, Android32 and Android64 with
`geode-sdk/build-geode-mod@main` (`sdk: nightly`) and uploads one combined artifact.

## Layout
`src/main.cpp` hooks · `OlkFPC` global state · `FrameAnalyzer` headless sim + insights · `MacroParser` gdr/json/gdr2/cml/ttr3 ·
`PlaybackController` · `RenderManager` (FFmpeg pipe) · `SoundManager` (FMOD) · `ui/` overlays and popups.

## Definitions
- **Window** = min(max back-shift, max forward-shift) in scan-FPS frames that still survives `scan-horizon` frames.
  Shifts stop at neighbouring inputs. `-1` = the click already fails unshifted (excluded from stats).
- Put an optional `bing.mp3` in `resources/` (falls back to `GJ_button01.ogg`).

## Known risks (verify against current bindings)
See the notes delivered with the project: Popup API, `handleButton`, `createCheckpoint`/`loadFromCheckpoint`,
`playEffectAdvanced` signature, `m_gameState.m_currentProgress`, GD version strings in `mod.json`.
