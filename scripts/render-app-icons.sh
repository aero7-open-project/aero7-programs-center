#!/usr/bin/bash
set -euo pipefail

# Rasterize the project owner's existing Programs Center artwork. The master
# includes wide transparent margins; hicolor must not shrink those margins
# into the taskbar's already-small icon slot.
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
master="$root/resources/icons/aero7-programs-center.png"

command -v magick >/dev/null || { echo 'ImageMagick is required.' >&2; exit 1; }
[[ -r "$master" ]] || { echo 'The Programs Center icon master is missing.' >&2; exit 1; }

for size in 16 22 24 32 48 64 128 256 512; do
    width=$(( (size * 94 + 50) / 100 ))
    height=$(( (size * 97 + 50) / 100 ))
    output="$root/resources/icons/${size}x${size}/aero7-programs-center.png"
    mkdir -p -- "$(dirname -- "$output")"
    magick "$master" -trim +repage -filter Lanczos \
        -resize "${width}x${height}!" -gravity center -background none \
        -extent "${size}x${size}" -strip "$output"
done
