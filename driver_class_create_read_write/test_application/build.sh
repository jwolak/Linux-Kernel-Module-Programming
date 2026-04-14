#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  ./build.sh [--cmake|--make|--clean]

Options:
  --cmake    Build using CMake (default)
  --make     Build using Makefile
  --clean    Clean build artifacts

Default (no args): CMake build
EOF
}

BUILD_TYPE="${1:-cmake}"

case "$BUILD_TYPE" in
  --cmake)
    echo "Building with CMake..."
    mkdir -p build
    cd build
    cmake -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 ..
    make
    cd ..
    echo "Done. Binary: ./test_simple_module.arm64"
    ;;
  --make)
    echo "Building with Makefile..."
    make
    echo "Done. Binary: ./test_simple_module.arm64"
    ;;
  --clean)
    echo "Cleaning..."
    rm -rf build
    make clean
    echo "Done."
    ;;
  -h|--help)
    usage
    exit 0
    ;;
  *)
    echo "Unknown option: $BUILD_TYPE" >&2
    usage
    exit 1
    ;;
esac
