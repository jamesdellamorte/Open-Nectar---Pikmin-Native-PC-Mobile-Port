#!/usr/bin/env bash
# build-macos.sh
#
# Build de desarrollo para macOS con las dos regiones de Pikmin 1 juntas.
# El launcher pide `nectar-pal` junto a sí mismo cuando el disco es europeo;
# una build normal solo deja `nectar` (USA), así que aquí se compila también la
# PAL y se copia como nectar-pal a build/pikmin1/bin.
#
# Requisitos: brew install cmake (y xcode-select --install).
# Opcional: brew install libusb para un mando USB Xbox 360 (con reconexión).
# Sin libusb se compila igualmente, con los mandos que SDL reconoce de serie.
#
# Usage: ./scripts/build-macos.sh
set -euo pipefail

root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
jobs="${NECTAR_BUILD_JOBS:-$(sysctl -n hw.ncpu)}"
sdl_version="${SDL_VERSION:-2.32.10}"
sdl_dir="${root}/build/sdl2-${sdl_version}"
archs="$(uname -m)"
usa_dir="${root}/build/pikmin1"
pal_dir="${root}/build/pikmin1-pal"

# Aurora es el único backend gráfico y corre sobre SDL3; el código del juego
# sigue compilando contra la API de SDL2 vía sdl2-compat (brew install
# sdl2-compat), que implementa el ABI de SDL2 sobre el mismo SDL3 de Aurora:
# un solo runtime SDL en el proceso. Ya no hace falta la SDL2 estática.
if ! brew list sdl2-compat >/dev/null 2>&1 && [[ ! -d /opt/homebrew/opt/sdl2-compat && ! -d /usr/local/opt/sdl2-compat ]]; then
    echo "Falta sdl2-compat. Ejecuta: brew install sdl2-compat" >&2
    exit 1
fi
sdl_args=()

cmake -S "${root}/games/pikmin1" -B "${usa_dir}" -DCMAKE_BUILD_TYPE=Release -DPIKMIN_NATIVE_JAUDIO=ON -DPIKMIN_XINPUT_USB=ON "${sdl_args[@]}" \
    -DPIKMIN_GAME_VERSION=VERSION_GPIE01_01
cmake --build "${usa_dir}" --target pikmin_pc pikmin_launcher -j"${jobs}"

cmake -S "${root}/games/pikmin1" -B "${pal_dir}" -DCMAKE_BUILD_TYPE=Release -DPIKMIN_NATIVE_JAUDIO=ON -DPIKMIN_XINPUT_USB=ON "${sdl_args[@]}" \
    -DPIKMIN_GAME_VERSION=VERSION_GPIP01_00
cmake --build "${pal_dir}" --target pikmin_pc -j"${jobs}"

cp "${pal_dir}/bin/nectar" "${usa_dir}/bin/nectar-pal"
chmod 755 "${usa_dir}/bin/nectar-pal"

cmake -S "${root}/games/pikmin2" -B "${root}/build/pikmin2" -DCMAKE_BUILD_TYPE=Release "${sdl_args[@]}" -DPIKMIN_XINPUT_USB=ON \
    -DPIKMIN_GAME_VERSION=VERSION_GPVP01
cmake --build "${root}/build/pikmin2" --target pikmin2_pc -j"${jobs}"

printf 'Listo: %s/bin (nectar, nectar-pal, nectar-launcher)\n' "${usa_dir}"
