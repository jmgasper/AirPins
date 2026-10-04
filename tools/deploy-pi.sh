#!/usr/bin/env bash
# Lab helper: builds AirPins, copies it to the Raspberry Pi 4 lab board's
# ~/config/non-packaged/apps and starts it there (the running one is ended).
#   tools/deploy-pi.sh [board address] [AirPins arguments]
set -euo pipefail
ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
LAB=${LAB:-/mnt/HaikuWork/rpi4/haiku/tools/rpi4}
HOST=${1:-192.168.1.214}
shift || true
"$ROOT/tools/build-cross.sh" all >/dev/null
TARGET=/boot/home/config/non-packaged/apps/AirPins
python3 "$LAB/send.py" "$HOST" "$ROOT/build-arm64/AirPins" /boot/home/AirPins.staged
python3 "$LAB/shell.py" "$HOST" "for t in \$(ps | grep '[A]irPins' | awk '{print \$(NF-3)}'); do kill \$t; done; sleep 1; mkdir -p \$(dirname $TARGET); mv /boot/home/AirPins.staged $TARGET; chmod +x $TARGET; mimeset -f $TARGET; (cd /boot/home; ${AIRPINS_ENV:-} $TARGET $* > /boot/home/airpins.log 2>&1 &); sleep 2; ps | grep -c '[A]irPins'" 30
