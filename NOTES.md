# Notes

## Base
- Project Picori (999sian/tmc) at d377558 (v0.9.6, 2026-10-20): SDL3, a
  software GBA PPU (port/ppu), agbplay audio, Dear ImGui menus, in-process
  asset extraction from the ROM at first start (packs in assets/<region>/).
  Multi-region (USA/EU/JP) in one binary (MULTI_REGION).
- Picori targets 64-bit only (static assert in port/port_types.h); the Vita
  is 32-bit ARM. The 3DS port (EstebanPdN/zelda-tmc-3ds, also 32-bit ARM,
  from an older Picori) shows what that takes:
  - u32/s32 as unsigned int/int: VitaSDK's newlib makes uint32_t
    `unsigned long` (as N64's), which breaks the decomp's prototypes/enums
    (TMC_VITA next to TMC_N64 in gba/types.h and port_types.h);
  - the 64-bit struct paddings (`#ifdef PC_PORT` pads for pointer sizes)
    only when `__SIZEOF_POINTER__ == 8`: 82 places taken over from the 3DS
    port by matching their context (script), 7 more found by the layout
    asserts, and room.c's / port_stubs.c's 64-bit-only code;
    PORT_POINTERS_ARE_64_BIT in global.h.
- Vita addresses (0x81000000 up) never overlap the GBA ranges
  (0x02000000-0x0A000000) Picori maps, unlike the 3DS's.

## Vita specifics (patches/picori/0001-ps-vita.patch, vita/src)
- Data folder ux0:data/tmc: chdir there at start-up (as Picori does on
  Android) and port::ExecutableDir() returns it.
- Asset packs: read into memory (no mmap).
- Speech (TTS) off, crash backtraces off (no execinfo), update check finds no
  curl (popen stub).
- Threads: Picori's extraction uses std::async workers and its music mixer
  std::mutex; libstdc++ threads need pthread_cancel linked (-u) and
  pthread-embedded initialised before libstdc++'s priority-0 constructors
  (sms-vita saw a crash at start-up when forcing threads on without that).
  vita_threads.cpp calls pthread_init() at priority 0 and logs whether
  std::thread works. 1 MiB stacks for attribute-less threads.
- printf family wrapped for %zu (VitaSDK newlib has no C99 formats).
- Compile check without VitaSDK: clang --target=armv7a with VitaSDK headers
  (uint32_t forced to unsigned long as in GCC): all 741 files compile.

## Enums
- The decomp needs 4-byte enums (it declares enum-taking functions as u32):
  -fno-short-enums, while VitaSDK's libraries and SDL3 keep the EABI's packed
  enums (SDL disables its own enum-size assert on the Vita). The SDL structs
  Picori shares that hold enums (SDL_AudioSpec.format, the keyboard event's
  scancode) keep the same field offsets either way (alignment), and enums
  passed by value are registers.

## Local statics
- First run crashed in the start menu: ImGui_ImplSDL3_NewFrame's local static
  -> libstdc++'s __cxa_guard_acquire -> pthread_mutex_lock(NULL): its global
  mutex (made through pthread_once) was never made. vita_guard.cpp replaces
  the three __cxa_guard functions (--wrap) with a recursive Vita kernel
  mutex; the self-test logs whether pthread_once runs its routine.

## First play (896eeaf)
- Extraction took 2 h 40 min on the memory card (sweep 90 min, sprites 25,
  areas 23, palettes 16): thousands of small files (assets_src/, rom_data/).
  To cut before the EU version (runtime-only extraction).
- rom_data/ off on the Vita (2789 page files read at every start, written on
  the first; the ROM is required anyway).
- Profiling on ([profile] every 120 frames) and the audio report once a
  second while tuning; audio: GBA-accurate mode (NEAREST resampling, no
  post-process) and a 2048-frame buffer by default.
- Title screen pillarboxed to 3:2 with the "blurred" side fill (Picori's
  default bg_fill); true widescreen only in gameplay (TASK_GAME).
- Start on the title screen crashed (MemFill32 <- sub_0805F5CC <- ShowTextBox
  <- DrawFileSelectSettingsHint): Port_IsFontGBAEncoded sniffs a GBA blob by
  word 1 (gfx_dest) being a GBA address, which a native Font with a VRAM
  gfx_dest also is on 32-bit; Port_DecodeFontGBA then turned its native
  pointers into NULL. On 32-bit both layouts match: field-by-field resolve
  only (text.c), with 0x02000D00 -> gTextGfxBuffer as the decoder does.

## Second play (e2f4370)
- Start-up much faster without rom_data.
- [profile]: game tick ~5 ms, present ~4 ms, yet only 15-27 presents per 120
  ticks: the decoupled pacer's cost check held presents back (~10 fps on
  screen). Vita: legacy coupled loop, one present per tick paced by VSync.
- [audio]: synth work 35-48 ms per 21 ms buffer once music plays (~2x real
  time) -> crackle. Now 32 kHz output (BGM port, no resampling), agbplay
  at -O3 with vectorisable float maths, and idle tracks skipped (only tracks
  a channel wrote to, or with a reverb tail, are cleared/reverbed/mixed;
  the port's own mixdown skips them too).
- Port_IsValidHostPtr and IsColliding's guard accepted only pointers above
  4 GiB: on 32-bit every host pointer failed (animations, NPC interaction,
  collision). 32-bit: >= 0x10000000 and below 0xFFFF0000.
- Start skips the opening storybook (cutscene.c, states 1-10 -> 11).
- Open: black screen in gameplay after the story (game running: autosaves,
  area changes); visual glitches on the file select and story pages.

## Third play (3d34e0e)
- Sound fixed: with idle tracks skipped the synth takes ~0.3 ms per 21 ms
  buffer (was 35-48). Back to Picori's defaults (48 kHz, enhanced mode).
- One present per tick: smooth to the eye, but the PPU render is ~22 ms a
  frame (the old ~3 ms was averaged over the skipped presents) -> ~25-30 fps.
  Next big item. First step: the PPU's __thread scratch (VPPU_TLS) made
  plain on the Vita like on the N64 (single-threaded render; no TLS segment
  in Vita executables).
- Still: no sprites on the file select (slots, cursor, Link), black after
  the story. [status] line next to each [profile]: task/state, fade, display
  registers, visible OAM count.
- Settings button label: Select+Start (already Picori's gamepad toggle).
- bg_fill default black on the Vita.

## Fourth play (b5d627f)
- [status]: file select has 0 visible OAM entries; after the story the game
  sits in the storybook's state 12 (nullsub, waiting for the entities it
  loaded to move it on) with no fade, BG0-2+OBJ on, width 282: black, and
  no sprites anywhere. Suspect: entities not running/drawing at all.
  [entities] line added: entity list lengths, updates, DrawEntity calls,
  draw-list entries, frame lookups that failed, OAM pieces built.
- Settings: L+R (both within 200 ms) on the Vita; label "L+R".

## Fifth play (3666934) — the sprite / black-screen bug
- [entities]: never more than 1-2 entities alive; the file select's objects
  never appeared. Cause: entity.c marks a deleted entity with prev = -1 and
  tests `(intptr_t)prev < 0` (the decomp's own @bug note). Vita pointers are
  0x81000000+, negative as intptr_t, so every live entity counted as deleted:
  ClearAllDeletedEntities wiped them each frame and GetEmptyEntity handed
  out live ones. Now `== -1`.
- Audio: enhanced mode at 48 kHz = 30-35 ms per buffer (crackle); back to
  32 kHz GBA-accurate on the Vita (0.3 ms).

## Sixth play (79dd583)
- Sprites, text, music fine; rooms' tiles scrambled (16x16 blocks wrong or
  black), Link walking off the room, stairs not working. Cause:
  Port_ResolveEwramPtr maps GBA addresses inside gMapBottom/gMapTop
  (MapLayer, which starts with a pointer) to native offsets with a fixed +4
  for the widened pointer: on 32-bit everything (mapData, collision,
  subTiles, actTiles) landed 4 bytes off. Shift is now sizeof(void*) - 4.

## Speed (6d37c9d: rooms right, 25-33 fps)
- Per frame: game ~5 ms, PPU render ~19-24 ms, rest of present ~6-14 ms
  (incl. the VSync wait, which rounds a 30 ms frame up to 33).
- PPU render now on three cores: the main thread (pinned to core 0) and two
  workers (cores 1, 2) take every third scanline. The PPU's per-thread state
  (IO override, OBJ semi-transparency and OBJ-window masks) is found through
  TPIDRURO (VitaSDK's __thread is emulated and slow); hot loops take a local
  pointer per line. Falls back to one thread if the threads can't be told
  apart. PPU sources at -O3.
- [present] line: texture upload + draw, menus/overlays, flip + VSync.

## Speed, round 2 (67e1b97: 3 render threads)
- 1-room house: ~58 fps (game 4.6, render 6.7, present 12.5 ms); wide rooms
  (282 px): ~45 fps (render 10.7 ms). Of the present: upload 0.6, ImGui
  overlay 1.6 (HD dialogue and menu art draw through it, so it stays), flip
  0.35; the remaining ~3 ms was colour correction over all 384x160 pixels.
- Colour correction now on the PPU's 512-entry palette LUTs (hook
  virtuappu_mode1_palette_lut_post); BG lines drawn per tile run (one map
  read + one row fetch per 8 px); composite fast path for lines without
  windows, colour effects or semi-transparent OBJ.
- [ppu] line: bg/obj/composite time of the main thread's third of the lines.

## Round 3 (31c341e): ~55-60 fps in gameplay
- Wide rooms: render ~5 ms (bg ~2.3, obj ~0.4, composite ~0.6 on the main
  thread's third). File select ~28 fps: "menu art background" draws the side
  art as thousands of ImGui rectangles a frame (overlay 10-15 ms) -> off by
  default on the Vita. F8 menu open: ~8-11 ms of ImGui.
- Cheats (vita/src/vita_cheats.c, Settings -> Cheats tab, cheats.txt):
  health/arrows/bombs/rupees don't decrease (blocked in ModHealth/ModArrows/
  ModBombs/ModRupees), wallet full, regular kinstones 0x6E-0x75 at 99 (with
  the bag), shells 999.
- Warp tab: areas with 1-2 rooms left a SameLine pending, the next header
  drew over the buttons.
- Cheats feedback (d9ccba5): all work except shells: with Reborn's 9999
  shell cap, 999 + a pickup went past 1000 and the 3-digit counter showed
  "0xx". Now held at exactly 999. Added: bomb bag / quiver always full
  (owning bombs / bow), "Change name..." (Vita keyboard via
  SDL_StartTextInput; the text event is taken in port_bios.c's event loop).
- Slow spots (40-48 fps): dungeon rooms with colour effects (composite slow
  path, render ~10 ms) and HD dialogue text boxes (ImGui ~5-7 ms).
- "Unlock all items" after renames (log of 79ff5a4): the keyboard's Enter
  reached Port_DebugMenu_HandleKey (legacy page cursor -> Items -> "Unlock
  all items") because the swallow compared e.key.scancode: SDL_Scancode is
  an enum, 2 bytes in SDL (short enums) but 4 in our view (-fno-short-enums),
  so the read included 2 bytes of padding and matched only sometimes. Compare
  e.key.key (SDL_Keycode, a Uint32) instead. Rule: never read SDL enum-typed
  event fields in our code; use the plain integer ones.

## v1.0.1 fixes (patches/picori/0002-v1.0.1-fixes.patch)
- Frame drops with "game" at 30-55 ms (Crenel Minish path to Melari's mine,
  Veil Falls top with Biggoron, busy sprite scenes): every DMA / CpuSet /
  animation-frame read calls Port_IsLoadedAssetBytes, which walked the whole
  binaryFiles hash map (thousands of cached blobs, a few cache misses each:
  ~1 ms a call on the Vita). That map only grows and its blobs never move, so
  a sorted [begin, end) table rebuilt when its size changes is binary-searched
  instead (also for Port_LoadedAssetBytesEnd).
- PPU: scanlines are taken from a shared counter instead of fixed thirds. The
  workers share their cores with the audio mixer (up to ~11 ms per 23 ms
  buffer in the storm) and system threads; with fixed thirds the main thread
  waited for them (render ~13 ms against ~7 ms of its own lines). The [ppu]
  line still samples every third line, now from whichever thread drew it.
- Rain on Mt Crenel, then pause menu and back: the room became a mess of
  tiles. RestoreGameTask's port addition forced gBG1Buffer/gBG2Buffer into
  VRAM for BG1 and BG2; in the storm BG1 is the rain, whose tilemap the room
  loads straight into VRAM (WeatherChangeManager), so the menu's own tilemap
  left in gBG1Buffer was copied over it. Now only layers that show a map
  (gMapTop/gMapBottom) or that a manager feeds from its own buffer are copied.
- Biggoron (Veil Falls top) at 282 px: his BG1 slice ended at x = 240 (no
  widescreen reveal for BG1, WIN1 clamped to the native width) and the right
  side showed a block of his lower body under a black notch. The reveal now
  continues his body tilemap (gUnk_02006F00, from the column bigGoron.c
  publishes) and WIN1 reaches the frame's edge while it does
  (virtuappu_mode1_ws_win1_extend).
- Crash after loading a save state (data abort, Subtask_FadeOut ->
  RestoreGameTask -> sub_0801AE44 -> sub_080809D4): a snapshot holds gMain but
  not gUI / the menu's VRAM, so one made inside a subtask (pause menu, or an
  area-change autosave taken mid-transition) resumed that subtask with this
  session's gUI, and closing it restored garbage room controls. Save states are
  now made and loaded only in normal play (task GAME, GAMETASK_MAIN,
  GAMEMAIN_UPDATE), with a message otherwise; an area-change autosave waits
  for normal play; old slots made inside a menu are refused.

## v1.0.2: widescreen fills (patches/picori/0003-v1.0.2-widescreen.patch)
- At 282 px, columns past x = 240 are drawn from port-side "shadow" tilemaps,
  which existed only for the room maps and three area-specific BG3 overlays;
  any other 32-column layer was culled at 240 (black band). Now also filled:
  - BG3 static / animated backgrounds and overlays (control 0x1e07, 0x1e04,
    0x1e05) in any area, by wrapping their screenblock (the GBA wraps these
    itself once the parallax passes 16 px): Veil Falls / Mt Crenel skies.
  - Mt Crenel's storm rain on BG1, wrapped the same way.
  - Slice layers, where a manager copies a 32-column window of a 128-column
    tilemap each frame: Biggoron (replacing v1.0.1's special case) and the
    horizontal Minish path parallax (BG1 and BG3). They publish source and
    first column through TmcVita_WsPublishSlice.
- Any enabled 32-column layer still without a fill is logged once per room:
  "[ws] area=.. room=..: BGn (control ....) has no widescreen fill".
