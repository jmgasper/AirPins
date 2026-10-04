#!/usr/bin/env bash
# Builds AirPins on Haiku and makes its package in artifacts/.
set -euo pipefail
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
cd "$ROOT"
BUILD=${BUILD:-build-haiku}
ARCH=$(getarch 2>/dev/null || uname -m)
make BUILD="$BUILD" all
STAGE=$(mktemp -d /tmp/airpins-package-XXXXXX)
trap 'rm -rf -- "$STAGE"' EXIT
mkdir -p "$STAGE/apps" "$STAGE/data/deskbar/menu/Applications" \
	"$STAGE/documentation/packages/airpins" artifacts
cp "$BUILD/AirPins" "$STAGE/apps/AirPins"
strip --strip-debug "$STAGE/apps/AirPins"
xres -o "$STAGE/apps/AirPins" "$BUILD/AirPins.rsrc"
sed "s/^architecture .*/architecture $ARCH/" resources/AirPins.PackageInfo \
	> "$STAGE/.PackageInfo"
cp README.md LICENSE "$STAGE/documentation/packages/airpins/"
mkdir -p "$STAGE/documentation/packages/airpins/third_party/pigg" \
	"$STAGE/documentation/packages/airpins/icons"
cp third_party/pigg/LICENSE "$STAGE/documentation/packages/airpins/third_party/pigg/"
cp resources/icons/README.md resources/icons/fontawesome/LICENSE.txt \
	resources/icons/fontawesome/CC-BY-4.0.txt \
	"$STAGE/documentation/packages/airpins/icons/"
ln -s ../../../../apps/AirPins "$STAGE/data/deskbar/menu/Applications/AirPins"
(cd "$STAGE" && mimeset -f --all --mimedb data/mime_db \
	--mimedb /boot/system/data/mime_db apps/AirPins)
VERSION=$(sed -n 's/^version //p' resources/AirPins.PackageInfo)
package create -C "$STAGE" "$ROOT/artifacts/airpins-$VERSION-$ARCH.hpkg"
