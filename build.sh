#!/usr/bin/env bash
# Builds The Minish Cap for the PS Vita: build/tmc.vpk
#
# The first run downloads Project Picori (picori/, the native port of the
# Minish Cap decompilation, at the version this project was made for) and
# applies patches/picori/*.patch; when the patches change, picori/ is reset
# to that version and they are applied again. No game data is downloaded:
# the game reads your own ROM on the Vita (see README.md).
#
#   ./build.sh            build (only what changed)
#   ./build.sh --clean    delete build/ first
#   ./build.sh --my-art   use your own home-screen pictures from livearea/
#                         (vita/livearea/make.py); for your own builds only,
#                         a plain ./build.sh (e.g. for a release) uses the
#                         placeholders
#   JOBS=2 ./build.sh     fewer parallel compiler jobs
set -euo pipefail
cd "$(dirname "$0")"

PICORI_URL=https://github.com/999sian/tmc.git
PICORI_COMMIT=d377558d973c648e8d0dc491bf99af9aaa0d8067
JOBS=${JOBS:-$(nproc)}
B=build

[[ -n "${VITASDK:-}" ]] || { echo "ERROR: VITASDK is not set."; exit 1; }
for tool in git cmake python3; do
  command -v "$tool" >/dev/null || { echo "ERROR: $tool is not installed (sudo apt install $tool)."; exit 1; }
done

CLEAN=0
ART="$PWD/vita/livearea"
for arg in "$@"; do
  case "$arg" in
    --clean) CLEAN=1 ;;
    --my-art)
      [[ -f livearea/icon0.png ]] || { echo "ERROR: no livearea/ yet: run python3 vita/livearea/make.py first."; exit 1; }
      ART="$PWD/livearea" ;;
    *) echo "ERROR: unknown option $arg (--clean, --my-art)"; exit 1 ;;
  esac
done
if [[ $CLEAN == 1 ]]; then
  rm -rf "$B"
fi

if [[ ! -d picori/.git ]]; then
  echo "== Downloading Project Picori"
  rm -rf picori
  git init -q picori
  git -C picori remote add origin "$PICORI_URL"
  git -C picori fetch -q --depth 1 origin "$PICORI_COMMIT"
  git -C picori -c advice.detachedHead=false checkout -q FETCH_HEAD
  # its sound engine (the other submodules are not needed)
  git -C picori submodule update --init -q libs/VirtuaAPU
fi

# The patches, applied to a clean picori whenever they change.
want=$(cat patches/picori/*.patch | sha256sum | cut -d' ' -f1)
have=$(cat picori/.tmc-vita-patches 2>/dev/null || true)
if [[ "$want" != "$have" ]]; then
  echo "== Applying patches/picori to a clean picori"
  git -C picori reset -q --hard "$PICORI_COMMIT"
  git -C picori clean -q -fd -e .tmc-vita-patches
  for p in patches/picori/*.patch; do
    echo "   $p"
    git -C picori apply "../$p"
  done
  echo "$want" > picori/.tmc-vita-patches
fi

bash vita/deps.sh

echo "== Configuring"
cmake -S . -B "$B" -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake" \
  -DCMAKE_BUILD_TYPE=Release -DTMC_LIVEAREA_DIR="$ART" -DTMC_VITA_COMMIT="$(git rev-parse --short HEAD 2>/dev/null || echo unknown)"
echo "== Building with $JOBS jobs"
cmake --build "$B" -j "$JOBS"
echo
if [[ "$ART" == "$PWD/livearea" ]]; then
  echo "SUCCESS: $B/tmc.vpk (with your own home-screen pictures: don't publish this one)"
else
  echo "SUCCESS: $B/tmc.vpk"
fi
