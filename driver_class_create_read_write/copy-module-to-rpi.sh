#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  ./copy-module-to-rpi.sh <rpi_ip> <rpi_user> <local_module_file> <remote_target_dir> [ssh_port]

Examples:
  ./copy-module-to-rpi.sh 192.168.0.105 janusz /work/hello_module/hello.ko /tmp
  ./copy-module-to-rpi.sh 192.168.0.105 janusz ./hello.ko /home/janusz/modules 22
EOF
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

if [[ $# -lt 4 || $# -gt 5 ]]; then
  usage
  exit 1
fi

RPI_IP="$1"
RPI_USER="$2"
LOCAL_FILE="$3"
REMOTE_DIR="$4"
SSH_PORT="${5:-22}"

if [[ ! -f "$LOCAL_FILE" ]]; then
  echo "ERROR: Local file not found: $LOCAL_FILE" >&2
  exit 2
fi

FILE_NAME="$(basename "$LOCAL_FILE")"
REMOTE_PATH="$REMOTE_DIR/$FILE_NAME"
SSH_TARGET="$RPI_USER@$RPI_IP"

echo "==> Ensuring remote directory exists: $REMOTE_DIR"
ssh -p "$SSH_PORT" "$SSH_TARGET" "mkdir -p '$REMOTE_DIR'"

echo "==> Copying module to $SSH_TARGET:$REMOTE_PATH"
scp -P "$SSH_PORT" "$LOCAL_FILE" "$SSH_TARGET:$REMOTE_PATH"

echo "==> Done"
echo "Remote file: $REMOTE_PATH"
