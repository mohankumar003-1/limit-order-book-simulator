#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"

if [[ ! -d "$BUILD_DIR" ]]; then
  echo "Configuring CMake build..."
  cmake -S "$ROOT_DIR" -B "$BUILD_DIR"
fi

echo "Building project..."
cmake --build "$BUILD_DIR" --config Release

TEST_EXE="$BUILD_DIR/Release/tests.exe"
APP_EXE="$BUILD_DIR/Release/orderbook.exe"

if [[ ! -f "$TEST_EXE" ]]; then
  echo "ERROR: test executable not found: $TEST_EXE"
  exit 1
fi

echo "Running tests..."
"$TEST_EXE"

if [[ ! -f "$APP_EXE" ]]; then
  echo "ERROR: application executable not found: $APP_EXE"
  exit 1
fi

echo "Tests passed. Starting orderbook..."
"$APP_EXE"
