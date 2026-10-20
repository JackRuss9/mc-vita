#!/usr/bin/env bash
# Builds and downloads what the Vita build needs besides VitaSDK (called by
# build.sh), each at a fixed version, into deps/ (nothing is installed into
# VitaSDK):
#   SDL3       window, renderer (the Vita's GPU), sound, controls; built here
#   Dear ImGui Picori's in-game menus (compiled with the game)
#   fmt, nlohmann/json   header-only libraries Picori uses
# libpng and zlib come from VitaSDK (vdpm).
set -euo pipefail
cd "$(dirname "$0")/.."

D=$PWD/deps
P=$D/prefix
JOBS=${JOBS:-$(nproc)}
mkdir -p "$D/src" "$P"

# name url tag
fetch() {
  local name=$1 url=$2 tag=$3 dir=$D/src/$1
  if [[ "$(cat "$dir/.tmc-version" 2>/dev/null || true)" != "$tag" ]]; then
    echo "== Downloading $name $tag"
    rm -rf "$dir"
    git -c advice.detachedHead=false clone -q --depth 1 -b "$tag" "$url" "$dir"
    echo "$tag" > "$dir/.tmc-version"
  fi
}

fetch SDL3 https://github.com/libsdl-org/SDL.git release-3.4.12
fetch imgui https://github.com/ocornut/imgui.git v1.92.7
fetch fmt https://github.com/fmtlib/fmt.git 11.1.4
fetch json https://github.com/nlohmann/json.git v3.11.3

if [[ "$(cat "$P/.sdl3" 2>/dev/null || true)" != "release-3.4.12" ]]; then
  echo "== Building SDL3 for the Vita"
  rm -rf "$D/src/SDL3/build-vita"
  cmake -S "$D/src/SDL3" -B "$D/src/SDL3/build-vita" \
    -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$P" -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TESTS=OFF \
    -DSDL_TEST_LIBRARY=OFF -DSDL_EXAMPLES=OFF >/dev/null
  cmake --build "$D/src/SDL3/build-vita" -j "$JOBS" >/dev/null
  cmake --install "$D/src/SDL3/build-vita" >/dev/null
  echo "release-3.4.12" > "$P/.sdl3"
fi

for lib in libpng.a libz.a; do
  if [[ ! -f "$VITASDK/arm-vita-eabi/lib/$lib" ]]; then
    echo "ERROR: VitaSDK has no $lib. Install it with: vdpm libpng zlib"
    exit 1
  fi
done
echo "== Libraries ready (deps/)"
