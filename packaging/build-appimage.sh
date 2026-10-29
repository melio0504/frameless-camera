#!/usr/bin/env bash

set -euo pipefail

readonly SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
readonly PROJECT_DIR="$(cd -- "$SCRIPT_DIR/.." && pwd)"
readonly TOOLS_DIR="$PROJECT_DIR/build/appimage-tools"
readonly DIST_DIR="$PROJECT_DIR/dist"
readonly LINUXDEPLOY="$TOOLS_DIR/linuxdeploy-x86_64.AppImage"
readonly QT_PLUGIN="$TOOLS_DIR/linuxdeploy-plugin-qt-x86_64.AppImage"
readonly OUTPUT="$DIST_DIR/frameless-camera-x86_64.AppImage"

readonly LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
readonly QT_PLUGIN_URL="https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"

if [[ "$(uname -m)" != "x86_64" ]]; then
  echo "AppImage packaging currently supports x86_64 only." >&2
  exit 1
fi

if ! command -v curl >/dev/null 2>&1; then
  echo "curl is required. Install it with: sudo apt install curl" >&2
  exit 1
fi

if ! command -v qmake6 >/dev/null 2>&1; then
  echo "qmake6 is required. Install it with: sudo apt install qt6-base-dev" >&2
  exit 1
fi

download_tool() {
  local url="$1"
  local destination="$2"

  if [[ -x "$destination" ]]; then
    return
  fi

  local temporary
  temporary="$(mktemp "$destination.download.XXXXXX")"

  echo "Downloading $(basename -- "$destination")..."
  if ! curl --fail --location --show-error --retry 3 \
    --proto '=https' --tlsv1.2 \
    --output "$temporary" "$url"; then
    rm -f -- "$temporary"
    return 1
  fi

  chmod +x "$temporary"
  mv -- "$temporary" "$destination"
}

mkdir -p -- "$TOOLS_DIR" "$DIST_DIR"

download_tool "$LINUXDEPLOY_URL" "$LINUXDEPLOY"
download_tool "$QT_PLUGIN_URL" "$QT_PLUGIN"

make -C "$PROJECT_DIR" all

readonly APPDIR="$(mktemp -d "$PROJECT_DIR/build/AppDir.XXXXXX")"
trap 'rm -rf -- "$APPDIR"' EXIT

export APPIMAGE_EXTRACT_AND_RUN=1
export EXTRA_PLATFORM_PLUGINS="libqwayland-egl.so;libqwayland-generic.so"
export LDAI_OUTPUT="$OUTPUT"
export QMAKE="$(command -v qmake6)"

"$LINUXDEPLOY" \
  --appdir "$APPDIR" \
  --executable "$PROJECT_DIR/build/frameless-camera" \
  --desktop-file "$SCRIPT_DIR/frameless-camera.desktop" \
  --icon-file "$SCRIPT_DIR/frameless-camera.svg" \
  --plugin qt \
  --output appimage

chmod +x "$OUTPUT"
echo "AppImage created: $OUTPUT"
