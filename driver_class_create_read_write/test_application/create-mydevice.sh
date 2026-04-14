#!/usr/bin/env bash
set -euo pipefail

DRIVER_NAME="simple_device_driver"
DEVICE_PATH="/dev/mydevice"
MINOR="0"

usage() {
  cat <<'EOF'
Usage:
  ./create-mydevice.sh

Creates /dev/mydevice for driver simple_device_driver using the current major
number from /proc/devices.
EOF
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

MAJOR="$(awk -v drv="$DRIVER_NAME" '$2 == drv { print $1 }' /proc/devices)"

if [[ -z "$MAJOR" ]]; then
  echo "ERROR: Driver '$DRIVER_NAME' not found in /proc/devices." >&2
  echo "Load module first, e.g.: sudo insmod ./simple_device_driver.ko" >&2
  exit 1
fi

echo "Detected major for $DRIVER_NAME: $MAJOR"

if [[ -e "$DEVICE_PATH" ]]; then
  echo "Removing existing node: $DEVICE_PATH"
  sudo rm -f "$DEVICE_PATH"
fi

echo "Creating node: $DEVICE_PATH (c $MAJOR $MINOR)"
sudo mknod -m 666 "$DEVICE_PATH" c "$MAJOR" "$MINOR"

echo "Done. Current node:"
ls -l "$DEVICE_PATH"
