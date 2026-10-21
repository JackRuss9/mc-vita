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
