# The Legend of Zelda: The Minish Cap for the PS Vita, built from Project
# Picori (picori/, downloaded and patched by build.sh) with VitaSDK.
#
#   ./build.sh   (configures this with the Vita toolchain and makes build/tmc.vpk)
cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(tmc_vita C CXX)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "The Minish Cap")
set(VITA_TITLEID "TMCV00001")
set(VITA_VERSION "01.14")

set(P ${CMAKE_SOURCE_DIR}/picori)
set(D ${CMAKE_SOURCE_DIR}/deps)
set(TMC_VITA_COMMIT "unknown" CACHE STRING "this project's commit (logged at start-up)")
# The rendered width: 384 is Picori's release setting (16:9 and wider
# windows show more of the room; menus and fixed scenes stay 240).
set(TMC_WIDESCREEN_WIDTH 384 CACHE STRING "framebuffer width capacity (240 = GBA)")

# ---------------------------------------------------------------- sources
set(PORT_FILES
  port_main.c port_android_log.c port_audio.c port_shm_framebuffer.c
  port_runtime_config.cpp port_debug_menu.cpp port_imgui_menu.cpp port_hd_text.cpp
  port_menu_background.c port_menu_background_imgui.cpp port_level_editor.cpp
  port_prelaunch_logo.cpp port_tts.cpp port_a11y_cues.c port_a11y_audio.c
  port_debug_actions.c port_flag_names.cpp port_debug_entities.c port_debug_memory_watch.c
  port_quicksave.c port_practice.c port_inline_ptrs.c port_script_addrs.c port_offset_remap.c
  port_asset_bootstrap.cpp port_asset_index.c port_update_check.c port_asset_loader.cpp
  port_asset_pipeline.cpp port_asset_log.cpp port_asset_pak.cpp port_asset_pak_loader.cpp
  port_exe_path.cpp port_debug_verbose.c port_rom_picker.c
  rando/rando.cpp rando/rando_logic.cpp rando/rando_file_menu.c rando/rando_save.c
  rando/rando_runtime.c rando/rando_newfile.c rando/rando_entrance.cpp rando/rando_cosmetic.cpp
  rando/rando_music.c rando/rando_keymap.c
  port_reborn.cpp port_m4a_backend.cpp generated_sounds_embed.cpp port_ppu.cpp
  port_gpu_renderer.cpp port_gpu_raster.cpp port_voxel.cpp port_gpu_obj_cull.c
  port_gpu_raster_gl.cpp port_glslp_parser.cpp port_glslp_runtime.cpp
  port_icon.cpp port_mods.cpp port_rom.c port_region_data.c port_stubs.c stubs_autogen.c
  data_stubs_autogen.c port_rom_stubs.c flag_remap_generated.c port_bios.c
  port_bugreport.cpp port_bugreport_state.c port_linked_stubs.c port_figurines.c port_draw.c
  port_gba_mem.c port_hdma.c port_upscale.c port_save.c port_softslots.c
  port_roll_attack_macro.c port_touch_controls.cpp port_filter.c port_animation.c port_math.c
  port_text_render.c port_gameplay_stubs.c port_m4a_stubs.c port_room_funcs.c
  port_script_funcs.c port_analog_movement.c port_audio_mute.cpp port_discord_rpc.c)
list(TRANSFORM PORT_FILES PREPEND ${P}/port/)

file(GLOB PPU_FILES ${P}/port/ppu/src/*.c)
file(GLOB APU_FILES ${P}/libs/VirtuaAPU/src/*.c)
file(GLOB AGBPLAY_FILES ${P}/libs/agbplay_core/*.cpp)

# The game (the decompilation), as Picori's xmake.lua lists it
set(GAME_PATTERNS
  main.c common.c interrupts.c game.c title.c fileselect.c flags.c save.c fade.c color.c
  vram.c screen*.c message.c text.c sound.c script.c scroll.c room.c roomInit.c entity.c
  physics.c collision.c movement.c affine.c sineTable.c ui.c
  player.c playerUtils.c playerHitbox.c playerItem.c playerItemUtils.c
  playerItemDefinitions.c playerItem/*.c
  enemy.c enemyUtils.c enemy/*.c npc.c npcUtils.c npcFunctions.c npcDefinitions.c npc/*.c
  object.c objectUtils.c objectDefinitions.c object/*.c enemyUpdate.c projectile.c
  projectileUtils.c projectileUpdate.c projectile/*.c manager.c manager/*.c item.c
  itemUtils.c itemDefinitions.c itemMetaData.c item/*.c
  subtask.c subtask/*.c cutscene.c backgroundAnimations.c beanstalkSubtask.c
  enterPortalSubtask.c gameOverTask.c gameUtils.c gameData.c kinstone.c droptables.c
  demo.c debug.c flagDebug.c staffroll.c menu/*.c data/figurineMenuData.c data/hitbox.c
  data/areaMetadata.c data/caveBorderMapData.c data/data_080046A4.c
  data/mapActTileToSurfaceType.c data/objPalettes.c data/screenTransitions.c
  data/transitions.c worldEvent/*.c code_08049CD4.c code_08049DF4.c code_0805EC04.c)
set(GAME_FILES)
foreach(pat ${GAME_PATTERNS})
  file(GLOB found ${P}/src/${pat})
  list(APPEND GAME_FILES ${found})
endforeach()

# Dear ImGui (the in-game menus), with its SDL3 and SDL_Renderer backends
set(IMGUI_FILES
  ${D}/src/imgui/imgui.cpp ${D}/src/imgui/imgui_draw.cpp ${D}/src/imgui/imgui_tables.cpp
  ${D}/src/imgui/imgui_widgets.cpp ${D}/src/imgui/imgui_demo.cpp
  ${D}/src/imgui/backends/imgui_impl_sdl3.cpp ${D}/src/imgui/backends/imgui_impl_sdlrenderer3.cpp)

# The Vita's side (start-up, paths, log)
file(GLOB VITA_FILES ${CMAKE_SOURCE_DIR}/vita/src/*.c ${CMAKE_SOURCE_DIR}/vita/src/*.cpp)

# The Picori logo shown at start-up, as a C byte list (xmake's bin2c rule)
set(GEN ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GEN})
file(READ ${P}/docs/picori-logo.png LOGO_HEX HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1, " LOGO_BYTES "${LOGO_HEX}")
file(WRITE ${GEN}/picori-logo.png.h "${LOGO_BYTES}\n")

# SDL3, built for the Vita by vita/deps.sh into deps/prefix
set(SDL3_DIR ${D}/prefix/lib/cmake/SDL3)
find_package(SDL3 REQUIRED CONFIG)

add_executable(tmc ${VITA_FILES} ${PORT_FILES} ${PPU_FILES} ${APU_FILES} ${AGBPLAY_FILES}
  ${P}/tools/src/assets_extractor/assets_extractor_api.cpp ${GAME_FILES} ${IMGUI_FILES})

target_compile_definitions(tmc PRIVATE
  PC_PORT NON_MATCHING USE_HDMA USA ENGLISH REVISION=0 MULTI_REGION TMC_VITA
  TMC_PC_VERSION="0.9.6" TMC_PORT_VERSION="0.9.6"
  MODE1_GBA_WIDTH=${TMC_WIDESCREEN_WIDTH} FMT_HEADER_ONLY=1
  TMC_VITA_COMMIT="${TMC_VITA_COMMIT}"
  # no "open in the system's shell" in ImGui (fork/execvp: none on the Vita)
  IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS)
target_include_directories(tmc PRIVATE
  ${CMAKE_SOURCE_DIR}/vita/include ${GEN} ${P}/include ${P}/libs ${P}/port ${P} ${P}/build/USA ${P}/port/ppu/include
  ${P}/port/shaders/build ${P}/libs/VirtuaAPU/include ${P}/libs/agbplay_core
  ${P}/tools/src/assets_extractor
  ${D}/prefix/include ${D}/src/imgui ${D}/src/fmt/include ${D}/src/json/include)
# region.h in every file, as Picori's build does (MULTI_REGION switches its
# macros to the region found at run time)
target_compile_options(tmc PRIVATE "SHELL:-include ${P}/include/region.h")
# 4-byte enums, as on the PC (the ARM EABI default packs them, and the decomp
# declares functions taking an enum as taking u32).
set(COMMON_FLAGS -O2 -g -fno-short-enums -fno-strict-aliasing -fwrapv -fno-strict-overflow -funsigned-char
  -Wno-unused-parameter -Wno-missing-field-initializers)
target_compile_options(tmc PRIVATE ${COMMON_FLAGS}
  $<$<COMPILE_LANGUAGE:C>:-std=gnu11> $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++20>)

# The music synth (agbplay) is the heaviest code on the Vita: full
# optimisation, and float maths the compiler may vectorise with NEON (its
# NaN guard is in port_m4a_backend.cpp, compiled normally).
set_source_files_properties(${AGBPLAY_FILES} PROPERTIES COMPILE_OPTIONS
  "-O3;-funsafe-math-optimizations;-fno-math-errno")
# The GBA picture renderer runs per pixel: full optimisation.
set_source_files_properties(${PPU_FILES} PROPERTIES COMPILE_OPTIONS "-O3")

target_link_libraries(tmc SDL3::SDL3-static png z m stdc++ pthread ScePower_stub SceKernelDmacMgr_stub)
target_link_options(tmc PRIVATE -Wl,-q -Wl,--no-enum-size-warning)
# libstdc++'s threads on (vita/src/vita_threads.cpp): the symbol it checks for,
# and 1 MiB stacks for threads made without attributes.
target_link_options(tmc PRIVATE -Wl,-u,pthread_cancel -Wl,--wrap=pthread_create)
# Local statics guarded with a Vita mutex (vita/src/vita_guard.cpp)
foreach(fn __cxa_guard_acquire __cxa_guard_release __cxa_guard_abort)
  target_link_options(tmc PRIVATE -Wl,--wrap=${fn})
endforeach()
# newlib here has no C99 printf formats (%zu): the printf family is wrapped to
# rewrite them (vita/src/printf_c99.cpp).
foreach(fn printf fprintf sprintf snprintf vprintf vfprintf vsprintf vsnprintf)
  target_link_options(tmc PRIVATE -Wl,--wrap=${fn})
endforeach()

# Extended memory (ATTRIBUTE2=12): more RAM for the game.
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d ATTRIBUTE2=12")

# The home-screen pictures: the placeholders in vita/livearea/, or your own
# (build.sh --my-art: livearea/ at the top, made by vita/livearea/make.py and
# never committed, see .gitignore).
set(TMC_LIVEAREA_DIR "${CMAKE_SOURCE_DIR}/vita/livearea" CACHE PATH "LiveArea pictures folder")
message(STATUS "LiveArea pictures: ${TMC_LIVEAREA_DIR}")

vita_create_self(eboot.bin tmc UNSAFE)
vita_create_vpk(tmc.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE ${TMC_LIVEAREA_DIR}/icon0.png sce_sys/icon0.png
       ${TMC_LIVEAREA_DIR}/bg.png sce_sys/livearea/contents/bg.png
       ${TMC_LIVEAREA_DIR}/startup.png sce_sys/livearea/contents/startup.png
       ${TMC_LIVEAREA_DIR}/template.xml sce_sys/livearea/contents/template.xml
       # the randomizer's logic (read from app0:, see rando_logic.cpp)
       ${P}/assets/rando/default.logic assets/rando/default.logic)
