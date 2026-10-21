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
