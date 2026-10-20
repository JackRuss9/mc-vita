# The Legend of Zelda: The Minish Cap for the PS Vita

A PS Vita build of [Project Picori](https://github.com/999sian/tmc), the native
port of the [Minish Cap decompilation](https://github.com/zeldaret/tmc). The
game runs natively on the Vita (no emulator). No game data is included: it
reads your own ROM, dumped from your own cartridge.

**Status: milestone 1, first build.** The goal is the game booting and
playable with sound and buttons. Planned next: widescreen tuning, an in-game
cheat menu (Select + L), touch-screen item switching, quality-of-life options.

## Building

With [VitaSDK](https://vitasdk.org) installed and `VITASDK` set (and its
`libpng` and `zlib`: `vdpm libpng zlib` if missing):

    ./build.sh

The first run downloads Project Picori and the libraries it needs (SDL3,
Dear ImGui, fmt, nlohmann/json) into `picori/` and `deps/`, then makes
`build/tmc.vpk`.

## On the Vita

1. Install `tmc.vpk` (VitaShell).
2. Copy your ROM to `ux0:data/tmc/baserom.gba` (USA; an EU ROM goes in as
   `baserom_eu.gba`).
3. Start the game. The first start extracts the game's graphics and sound
   from the ROM into `ux0:data/tmc/assets/` (once; it can take a while).

Everything lives in `ux0:data/tmc/`: the ROM, saves (`tmc.sav`), settings
(`config.json`) and the log of the last run, `log.txt`.

## Layout

| Path | What it is |
| --- | --- |
| `picori/` | Project Picori, downloaded by `build.sh` at a fixed version (not in this repository) |
| `patches/picori/` | our changes to it, applied by `build.sh` |
| `vita/src/` | the Vita's side: start-up, paths, threads, POSIX stand-ins |
| `vita/deps.sh` | downloads and builds SDL3, Dear ImGui, fmt and nlohmann/json into `deps/` |
| `vita/livearea/` | the home screen pictures (placeholders) |
| `CMakeLists.txt` | the Vita build (Picori's own build is xmake) |

## Legal

An unofficial fan project, not affiliated with or endorsed by Nintendo. It
contains no game data, art or audio: you need your own copy of the game.
Please do not share ROMs, extracted assets or built `.vpk` files.
Project Picori is under the GPL v3, and so are the changes here.
