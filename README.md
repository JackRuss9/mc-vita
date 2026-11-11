# The Legend of Zelda: The Minish Cap for the PS Vita

An unofficial PS Vita build of **[Project Picori](https://github.com/999sian/tmc)**,
the native PC port of the **[Minish Cap decompilation](https://github.com/zeldaret/tmc)**.
The game runs natively on the Vita as ARM code, with no emulator. It plays in
widescreen at 50-60 fps, with Picori's settings menu on **L+R** and Vita-only
extras: in-game cheats, a name change, and a skippable intro.

No game data is included. The game reads a ROM you dumped yourself from your
own cartridge.

> **Status: complete.** The USA version in English has been played on a real
> Vita from a new file to the end, at 100%: every item and upgrade (all bomb
> bags and quivers, Remote Bombs, Light Arrows, Magic Boomerang...), every
> figurine and every Kinstone fusion. All the cheats and the randomizer were
> tested too. See [What has been tested](#what-has-been-tested).

## Where this comes from

| Project | What it is |
| --- | --- |
| [zeldaret/tmc](https://github.com/zeldaret/tmc) | The decompilation: the original GBA game turned back into C source |
| [999sian/tmc](https://github.com/999sian/tmc) (Project Picori) | A native PC port of that decompilation, built with SDL3. It adds a software GBA video renderer, a better audio engine (agbplay), widescreen, a Dear ImGui settings menu, HD dialogue text, a randomizer, cheats, a warp menu and more |
| **this repository** | Makes Picori build and run on the PS Vita, makes it fast enough, and adds Vita-specific extras |

This repository does **not** contain a copy of Picori. `build.sh` downloads
Picori at a fixed commit (`d377558`) and applies our changes from
`patches/picori/*.patch`, in order. Everything else that is Vita-only lives in
`vita/`.

All credit for the game code, the port and its features goes to the zeldaret
decompilation team and to the Project Picori contributors. This project only
covers the Vita side.

## Features

### From Project Picori (work on the Vita)
- The whole game, running natively.
- **Widescreen.** Gameplay rooms fill the 16:9 screen when the room is wide
  enough; smaller scenes get black bars. On the Vita the extra columns are
  also filled for skies and static backgrounds, fog, steam and light rays,
  dark rooms, Mt Crenel's rain, the Minish path parallax and Biggoron.
- **Settings menu** (graphics, audio, gameplay options, warp, randomizer,
  Picori's own cheats and "Reborn" tweaks), opened with **L+R**.
- The startup launcher: pick the region/ROM and the language, and optionally
  skip the launcher on later starts.
- Save states (manual slots and auto-saves on area change), and the normal
  in-game save (`tmc.sav`). On the Vita, save states are made and loaded only
  during normal play, not with the game's menus open (a state made inside a
  menu crashed when loaded).
- **The randomizer**, with its logic file included in the VPK. Works with the
  USA ROM; see [Randomizer](#randomizer).
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
  - Cucco minigame timer doesn't decrease
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
- The render threads share the scanlines dynamically, so a core slowed down
  by the music or the system doesn't hold the frame back.
- HD dialogue text boxes are built once per change instead of every frame,
  and the file select no longer loads randomizer seeds while the cursor moves.
- No memory-card stalls: auto-saves are written in the background, the audio
  thread never writes to the log, and the timing reports in log.txt are off
  unless you create `ux0:data/tmc/debug_log` (they caused drops to 13 fps in
  Deepwood Shrine when the card was busy).
- Fast lookups for the game's asset data (the old linear search made some
  rooms drop to 14-20 fps: the Minish path to Melari's Mines, Veil Falls with
  Biggoron, busy sprite scenes).
- Result: about 55-60 fps in most gameplay. See [Known issues](#known-issues)
  for the slower spots.

## What has been tested

Tested on a real PS Vita (HENkaku, installed with VitaShell), USA ROM, English:

| Area | Result |
| --- | --- |
| First start: launcher, region/language choice, asset extraction | ✅ works (the first extraction is very slow, see below) |
| Title screen, file select, new file, naming | ✅ |
| Intro storybook, skipping it with Start | ✅ |
| **The whole game, start to finish, at 100%**: all dungeons, every item and upgrade, all figurines, all Kinstone fusions | ✅ |
| Going between floors and rooms, stairs, room transitions | ✅ |
| Sound effects and music | ✅ (32 kHz GBA-accurate) |
| Widescreen in gameplay | ✅ |
| Settings menu on L+R, warp tab, screen filters | ✅ |
| Save states (manual, auto, loading) | ✅ |
| Every cheat in the Cheats tab | ✅ |
| Change name (including empty and cancelled input) | ✅ |
| Randomizer: new seed, playing a randomized file | ✅ |

**Not tested on the Vita:**
- **EU/PAL ROMs** and the other languages. The code supports them (put the
  ROM in as `baserom_eu.gba`), but they haven't been tried.
- Picori's extra item slots on □ / △, which are mapped but not tried.
- Most of Picori's other menu options (level editor, the debug tabs, every
  "Reborn" tweak).
- PlayStation TV, suspending and resuming the Vita for a long time, and
  memory cards other than `ux0:`.

## Randomizer

1. On the file select screen, press **L** and tick **Enable Randomizer
   Mode**. Change the randomizer settings there if you like (the choice is
   remembered).
2. Choose an **empty** save slot and name the file.
3. The randomizer window opens with **Generate & Start** selected: press ✕
   (or **START**). The game may pause for a few seconds while it builds the
   seed. ○ cancels and starts a normal game in that slot instead.

The seed is stored in `tmc.randomizer`, so your normal saves are untouched.
The Seed text box can't open the Vita keyboard, so seeds are always random.
The "Copy" buttons do nothing on the Vita.

## Known issues

- **The first start is very slow.** The launcher extracts every graphic and
  sound from the ROM into `ux0:data/tmc/assets/`. On the Vita this can take a
  couple of hours, and the screen looks frozen the whole time. Leave it
  running. It happens only once; later starts take seconds.
- **Some slower spots:** a few rooms with lighting or transparency effects
  and Mt Crenel in the storm run at about 45-55 fps.
- **Widescreen:** a few special scenes may still be cut at the original
  width on the right. The log names them (`[ws] ... has no widescreen fill`);
  please report them with `log.txt`.
- **Room transitions** briefly show the black side bars instead of
  widescreen. This is how Picori does it.
- After the Vita keyboard (Change name) closes, the game can pause for a
  second.
- The LiveArea pictures (icon, background) are placeholders. You can use
  your own in your builds; see [Your own home-screen pictures](#your-own-home-screen-pictures).

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

1. Download `mc-vita-v1.1.4.vpk` from the
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
| `tmc.randomizer` | the randomizer's seeds for your save slots |
| `state_*.bin` | save states (manual slots and auto-saves) |
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

### Your own home-screen pictures

The repository only has placeholder pictures for the LiveArea. To use your
own in your own builds (for example art you own, which must not be
published), make them with `vita/livearea/make.py` and build with
`--my-art`:

    python3 vita/livearea/make.py ICON.jpg BACKGROUND.jpg
    ./build.sh --my-art

`make.py` (needs `sudo apt install python3-pil`) writes the bubble icon
(128x128), the LiveArea background (840x500) and the Start button picture
(280x158) into `livearea/`. A tall or wide icon picture is fitted inside the
bubble on its own background colour rather than cropped. `--bg-x` and
`--start-x` (0 = left, 1 = right, default 0.5) choose which part of a wide
background is kept. `livearea/` and `.jpg` files are in `.gitignore`, and a
plain `./build.sh` always uses the placeholders, so a VPK you publish never
contains your pictures. Build releases without `--my-art`.

### Layout

| Path | What it is |
| --- | --- |
| `build.sh` | downloads Picori at the pinned commit, applies the patches, builds |
| `patches/picori/0001-ps-vita.patch` | the port itself: every change to Picori's own files for v1.0.0 |
| `patches/picori/0002-...` to `0007-...` | the fixes and additions since (one per version, see NOTES.md) |
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
