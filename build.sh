#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
if [[ -f halo.mp3 ]]; then
  command -v ffmpeg >/dev/null || { echo 'ffmpeg needed for MP3 conversion'; exit 1; }
  ffmpeg -hide_banner -loglevel error -y -i halo.mp3 -f s16le -acodec pcm_s16le -ac 2 -ar 22050 halo.pcm
fi
NXDK_DIR="$(cd ../nxdk && pwd)"
export NXDK_DIR
eval "$("$NXDK_DIR/bin/activate" -s)"
make
mkdir -p release
cp bin/default.xbe release/default.xbe
echo "[DIAGNOSTIC] Building separate no-SDL screen test"
make -C diagnostic
cp diagnostic/bin/default.xbe release/diagnostic.xbe
if [[ -f halo.pcm ]]; then cp halo.pcm release/halo.pcm; fi
echo "Built release/default.xbe"
