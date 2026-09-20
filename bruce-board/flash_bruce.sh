#!/usr/bin/env bash
# Builds Bruce with the custom esp32-audit-dualboot board profile and
# flashes ONLY the resulting app binary to the ota_1 slot (0x310000) of
# this project's dual-boot partition table (../partitions_16mb.csv).
#
# Does NOT touch the bootloader, partition table, or ota_0 -- those
# belong to the audit firmware and must already be flashed first with a
# normal `pio run -t upload` from the project root. Re-run this script
# any time you want to update Bruce without re-flashing the audit
# firmware.
#
# Usage:
#   ./flash_bruce.sh /path/to/bruce/checkout [serial-port]
#
# If serial-port is omitted, esptool.py auto-detects it.
set -euo pipefail

BRUCE_DIR="${1:?Usage: $0 /path/to/bruce/checkout [serial-port]}"
PORT="${2:-}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ ! -f "$BRUCE_DIR/platformio.ini" ]; then
    echo "Error: $BRUCE_DIR doesn't look like a Bruce checkout (no platformio.ini)." >&2
    echo "Clone it first: git clone https://github.com/pr3y/Bruce.git" >&2
    exit 1
fi

echo "==> Copying board profile into the Bruce checkout"
mkdir -p "$BRUCE_DIR/boards/esp32-audit-dualboot" "$BRUCE_DIR/boards/_boards_json"
cp "$SCRIPT_DIR/esp32-audit-dualboot/esp32-audit-dualboot.ini" "$BRUCE_DIR/boards/esp32-audit-dualboot/"
cp "$SCRIPT_DIR/esp32-audit-dualboot/pins_arduino.h" "$BRUCE_DIR/boards/esp32-audit-dualboot/"
cp "$SCRIPT_DIR/esp32-audit-dualboot/interface.cpp" "$BRUCE_DIR/boards/esp32-audit-dualboot/"
cp "$SCRIPT_DIR/esp32-audit-dualboot/partitions_ota1_check.csv" "$BRUCE_DIR/boards/esp32-audit-dualboot/"
cp "$SCRIPT_DIR/_boards_json/esp32-audit-dualboot.json" "$BRUCE_DIR/boards/_boards_json/"

echo "==> Building Bruce for esp32-audit-dualboot"
( cd "$BRUCE_DIR" && pio run -e esp32-audit-dualboot )

FW_BIN="$BRUCE_DIR/.pio/build/esp32-audit-dualboot/firmware.bin"
if [ ! -f "$FW_BIN" ]; then
    echo "Error: build didn't produce $FW_BIN" >&2
    exit 1
fi

ESPTOOL_ARGS=(--chip esp32s3 --baud 460800)
if [ -n "$PORT" ]; then ESPTOOL_ARGS+=(--port "$PORT"); fi

echo "==> Flashing Bruce to ota_1 (0x310000) -- audit firmware/bootloader/ota_0 untouched"
python3 -m esptool "${ESPTOOL_ARGS[@]}" write_flash 0x310000 "$FW_BIN"

echo "==> Done. Power-cycle once to boot the audit firmware normally,"
echo "    then use its menu's \"Boot into Bruce\" action to switch."
