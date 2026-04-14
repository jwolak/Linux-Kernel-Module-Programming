#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  ./build-driver.sh [--clean|--in-tree]

Options:
  --clean    Remove module artifacts (including build/)
  --in-tree  Build directly in source dir (without build/)

Default:
  Build out-of-tree into ./build
EOF
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

case "${1:-}" in
  "")
    make
    ;;
  --clean)
    make clean
    ;;
  --in-tree)
    make in-tree
    ;;
  *)
    echo "Unknown option: ${1}" >&2
    usage
    exit 1
    ;;
esac
