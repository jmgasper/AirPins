#!/usr/bin/env bash
# Checks that src/hw/rpi_gpio.h matches the driver's header in the air/OS tree.
#   tools/check-abi.sh [haiku tree]
set -euo pipefail
ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
HAIKU=${1:-/mnt/HaikuWork/rpi4/haiku}
diff <(sed '5,7d' "$ROOT/src/hw/rpi_gpio.h") "$HAIKU/headers/private/drivers/rpi_gpio.h" \
	&& echo "src/hw/rpi_gpio.h matches $HAIKU"
