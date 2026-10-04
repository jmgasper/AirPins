#!/usr/bin/env bash
# Cross-builds AirPins for arm64 Haiku with the air/OS build tree's tools.
#   tools/build-cross.sh [make targets]       (default: all)
set -euo pipefail
ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
WORK=${WORK:-/mnt/HaikuWork}
CROSS=${CROSS:-$WORK/build/arm64/cross-tools-arm64/bin/aarch64-unknown-haiku-}
SYSROOT=${SYSROOT:-$WORK/build/summit-arm64/sysroot}
TOOLS=${TOOLS:-$WORK/build/arm64/objects/linux/x86_64/release/tools}
PRIVATE=$SYSROOT/boot/system/develop/headers/private
make -C "$ROOT" -j"${JOBS:-12}" BUILD=build-arm64 \
	CXX="${CROSS}g++ --sysroot=$SYSROOT" \
	APP_CPPFLAGS="-I$PRIVATE/interface -I$PRIVATE/shared" \
	RC="$TOOLS/rc/rc" XRES="$TOOLS/xres" MIMESET=true "${@:-all}"
