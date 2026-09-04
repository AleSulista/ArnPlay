#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
APP_PATH="${BUILD_DIR}/ArnPlay.app"
DIST_DIR="${ROOT_DIR}/dist"
STAGE_DIR="${BUILD_DIR}/dmg-stage"
VERSION="$(sed -nE 's/^project\(ArnPlay VERSION ([0-9.]+).*/\1/p' "${ROOT_DIR}/CMakeLists.txt")"
DMG_PATH="${DIST_DIR}/ArnPlay-${VERSION}-Intel.dmg"

if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "Erro: este empacotamento deve ser executado no macOS."
    exit 1
fi

if [[ "$(uname -m)" != "x86_64" ]]; then
    echo "Aviso: esta edição foi preparada para macOS Intel x86_64."
fi

for command_name in cmake brew codesign hdiutil; do
    if ! command -v "${command_name}" >/dev/null 2>&1; then
        echo "Erro: comando obrigatório não encontrado: ${command_name}"
        exit 1
    fi
done

QT_PREFIX="$(brew --prefix qt)"
echo "[1/4] Compilando ArnPlay ${VERSION}..."
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G Ninja \
    -DCMAKE_PREFIX_PATH="${QT_PREFIX}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES=x86_64
cmake --build "${BUILD_DIR}"

echo "[2/4] Aplicando assinatura local ao aplicativo..."
codesign --force --deep --sign - "${APP_PATH}"
codesign --verify --deep --strict "${APP_PATH}"

echo "[3/4] Montando o instalador DMG..."
mkdir -p "${DIST_DIR}"
rm -rf "${STAGE_DIR}"
mkdir -p "${STAGE_DIR}"
ditto "${APP_PATH}" "${STAGE_DIR}/ArnPlay.app"
ln -s /Applications "${STAGE_DIR}/Applications"
rm -f "${DMG_PATH}"
hdiutil create -volname "ArnPlay ${VERSION}" -srcfolder "${STAGE_DIR}" \
    -ov -format UDZO "${DMG_PATH}"

echo "[4/4] Verificando o resultado..."
hdiutil verify "${DMG_PATH}"

echo
echo "Pronto: ${DMG_PATH}"
echo "Instale arrastando ArnPlay para a pasta Applications."
