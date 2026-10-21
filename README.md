# The Legend of Zelda: The Minish Cap for the PS Vita

An unofficial PS Vita build of **[Project Picori](https://github.com/999sian/tmc)**,
the native PC port of the **[Minish Cap decompilation](https://github.com/zeldaret/tmc)**.
The game runs natively on the Vita as ARM code, with no emulator. It plays in
widescreen at 50-60 fps, with Picori's settings menu on **L+R** and Vita-only
extras: in-game cheats, a name change, and a skippable intro.

No game data is included. The game reads a ROM you dumped yourself from your
own cartridge.

> **Status: playable.** The USA version in English was played from a new file
> to the end of the first dungeon (Deepwood Shrine), with all the cheats and
> extras below. The rest of the game should work the same way, because it is
> the same code, but nobody has played further on the Vita yet. See
> [What has been tested](#what-has-been-tested).

## Where this comes from

| Project | What it is |
| --- | --- |
| [zeldaret/tmc](https://github.com/zeldaret/tmc) | The decompilation: the original GBA game turned back into C source |
| [999sian/tmc](https://github.com/999sian/tmc) (Project Picori) | A native PC port of that decompilation, built with SDL3. It adds a software GBA video renderer, a better audio engine (agbplay), widescreen, a Dear ImGui settings menu, HD dialogue text, a randomizer, cheats, a warp menu and more |
| **this repository** | Makes Picori build and run on the PS Vita, makes it fast enough, and adds Vita-specific extras |

This repository does **not** contain a copy of Picori. `build.sh` downloads
Picori at a fixed commit (`d377558`) and applies our changes from
`patches/picori/0001-ps-vita.patch`. Everything else that is Vita-only lives in
`vita/`.

All credit for the game code, the port and its features goes to the zeldaret
decompilation team and to the Project Picori contributors. This project only
covers the Vita side.

## Features

### From Project Picori (work on the Vita)
- The whole game, running natively.
- **Widescreen.** Gameplay rooms fill the 16:9 screen when the room is wide
  enough; smaller scenes get black bars.
- **Settings menu** (graphics, audio, gameplay options, warp, randomizer,
  Picori's own cheats and "Reborn" tweaks), opened with **L+R**.
- The startup launcher: pick the region/ROM and the language, and optionally
  skip the launcher on later starts.
- Auto-save states (`state_auto_*.bin`) and the normal in-game save
  (`tmc.sav`).
- Screen filters: **nearest** (the default and the recommended one) and
  **linear** run at full speed; **xBRZ** works but runs at about 7 fps on the
  Vita.

### Added for the Vita
- **L+R** opens and closes the settings menu. On PC it is F8; the on-screen
  hints say L+R.
- **Skippable intro:** Start skips the opening storybook.
- **Cheats tab**, the first tab of the menu. Each cheat is a checkbox,
  remembered in `cheats.txt`:
  - Health doesn't decrease
  - Arrows don't decrease / Bombs don't decrease / Rupees don't decrease
  - Wallet always full (to your current wallet's size)
  - Bomb bag always full / Quiver always full (to *your current* bag and
    quiver size, not the game's maximum; needs bombs or the bow)
  - Mysterious Shells always 999
  - All regular Kinstones, 99 of each red, blue and green piece (not the gold
    or story pieces; needs the Kinstone Bag)
- **Change name...**, in the Cheats tab, renames Link at any time with the
  Vita keyboard. Allowed characters are letters, digits, space and `- . ,`, up
  to 6 characters. An empty name is ignored. **Save in-game afterwards** to
  keep the new name.
- Vita-friendly defaults: plain black side bars, and the menu art background
  off, which was costly on the Vita.
- Fix: the Warp tab no longer draws areas with fewer than 3 rooms on top of
  each other.

### Performance work (Vita)
- The GBA video renderer draws on all three of the Vita's game cores, with
  faster paths for background tiles and for compositing simple lines.
- Colour correction is applied to the palettes instead of to every pixel.
- Audio runs at 32 kHz in GBA-accurate mode. The engine skips silent tracks,
  so there is no crackling.
- CPU/GPU clocks are raised to 444/222/222/166 MHz while the game runs.
- Result: about 55-60 fps in most gameplay. See [Known issues](#known-issues)
  for the slower spots.

## What has been tested

Tested on a real PS Vita (HENkaku, installed with VitaShell), USA ROM, English:

| Area | Result |
| --- | --- |
| First start: launcher, region/language choice, asset extraction | ✅ works (the first extraction is very slow, see below) |
| Title screen, file select, new file, naming | ✅ |
| Intro storybook, skipping it with Start | ✅ |
| Gameplay up to and through the first dungeon (Deepwood Shrine) | ✅ |
| Going between floors and rooms, stairs, room transitions | ✅ |
| Sound effects and music | ✅ (32 kHz GBA-accurate) |
| Widescreen in gameplay | ✅ |
| Settings menu on L+R, warp tab, screen filters | ✅ |
| Every cheat in the Cheats tab | ✅ |
| Change name (including empty and cancelled input) | ✅ |

**Not tested on the Vita:**
- The rest of the game after the first dungeon.
- The **randomizer** (Settings → Randomizer). It is built in, with its logic
  file included, and it saves to `tmc.randomizer` so your normal save is
  untouched. Generating a seed may pause the game for a few seconds. The Seed
  text box can't open the Vita keyboard, so use **Roll new seed** or
  **Random**. The "Copy" buttons do nothing on the Vita.
- **EU/PAL ROMs** and the other languages. The code supports them (put the
  ROM in as `baserom_eu.gba`), but they haven't been tried.
- Picori's extra item slots on □ / △, which are mapped but not tried.
- Most of Picori's other menu options (level editor, the debug tabs, every
  "Reborn" tweak).
- PlayStation TV, suspending and resuming the Vita for a long time, and
  memory cards other than `ux0:`.

## Known issues

- **The first start is very slow.** The launcher extracts every graphic and
  sound from the ROM into `ux0:data/tmc/assets/`. On the Vita this can take a
  couple of hours, and the screen looks frozen the whole time. Leave it
  running. It happens only once; later starts take seconds.
- **Some slower spots:**
  - dungeon rooms with lighting or transparency effects run at about
    40-48 fps;
  - text boxes with HD dialogue on cost a few fps;
  - the file select screen runs at about 30 fps.
- **Room transitions** briefly show the black side bars instead of
  widescreen. This is how Picori does it.
- After the Vita keyboard (Change name) closes, the game can pause for a
  second.
- The LiveArea pictures (icon, background) are placeholders.

## Controls

| Vita | Game |
| --- | --- |
| D-pad | Move |
| ✕ | A |
| ○ | B |
| L / R | L / R |
| Start / Select | Start / Select |
| □ / △ | Picori's extra item slots (untested) |
| **L + R** (together) | Open / close the settings menu |
| In the menu | D-pad to move, ✕ to press |

Buttons can be rebound in the settings menu.

## Installing and playing

1. Download `mc-vita-v1.0.0.vpk` from the
   [Releases](https://github.com/JackRuss9/mc-vita/releases) page (or build it
   yourself, see [Building](#building)) and install it with VitaShell. The
   .vpk contains only the program: no ROM, graphics, music or text from the
   game.
2. Copy your ROM to `ux0:data/tmc/baserom.gba` (USA). An EU ROM goes in as
   `ux0:data/tmc/baserom_eu.gba`, which is untested.
3. Start the game. In the launcher, choose the region and language, tick
   *Skip launcher on future runs* if you like, and press **Play**. Then wait
   for the one-time extraction (see Known issues).

Everything lives in `ux0:data/tmc/`:

| File | What it is |
| --- | --- |
| `baserom.gba` | your ROM |
| `assets/`, `assets_src/`, `rom_data/` | extracted from the ROM on the first start |
| `tmc.sav` | the in-game save |
| `tmc.randomizer` | the randomizer's save |
| `state_auto_*.bin` | auto-save states |
| `config.json` | settings |
| `cheats.txt` | which cheats are on |
| `log.txt` | the log of the last run. Please attach it to bug reports |

## Building

You need Linux or WSL with [VitaSDK](https://vitasdk.org) installed and
`VITASDK` set, plus `git`, `cmake` and `python3`. If VitaSDK's `libpng` and
`zlib` are missing, install them with `vdpm libpng zlib`.

    ./build.sh            # build (only what changed)
    ./build.sh --clean    # rebuild everything

The first run downloads Project Picori into `picori/` and the libraries it
needs into `deps/`: SDL3, Dear ImGui, fmt and nlohmann/json. The output is
`build/tmc.vpk`. The log prints the commit it was built from (`[vita] build
<commit>`), which helps confirm you are running the build you think you are.

### Layout

| Path | What it is |
| --- | --- |
| `build.sh` | downloads Picori at the pinned commit, applies the patch, builds |
| `patches/picori/0001-ps-vita.patch` | every change to Picori's own files |
| `vita/src/` | Vita start-up, logging, threads, cheats, name change, runtime fixes |
| `vita/deps.sh` | builds SDL3, Dear ImGui, fmt and nlohmann/json for the Vita |
| `vita/livearea/` | home-screen pictures (placeholders) |
| `CMakeLists.txt` | the Vita build (Picori itself uses xmake) |
| `NOTES.md` | technical notes: every problem found while porting and how it was solved |

### Porting notes, in short
Picori targets 64-bit PCs, so the Vita (32-bit ARM) needed a few changes. The
full story is in [NOTES.md](NOTES.md).
- pointer-size assumptions in the game's data structures and in the ROM
  address mapping;
- host-pointer checks that assumed addresses above 4 GiB;
- a "deleted entity" test that broke because Vita addresses look negative as
  signed integers;
- VitaSDK's short enums versus `-fno-short-enums`;
- a thread-safe replacement for C++ local statics (`__cxa_guard_*`);
- a faster way for the render threads to find their own state.

## Legal

An unofficial fan project, not affiliated with or endorsed by Nintendo. It
contains no game data, art or audio; you need your own copy of the game.
The released .vpk is compiled from source code only; everything from the
game is extracted on your own Vita from your own ROM.
**Please do not share ROMs, extracted assets (`assets/`, `assets_src/`,
`rom_data/`) or save files, and do not ask for them in issues.**

Project Picori is licensed under the GNU GPL v3, and so is everything in this
repository (see [LICENSE](LICENSE)). The libraries it uses keep their own
licences; see Picori's `THIRD-PARTY-LICENSES.md`.
