#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
APP_PATH="${BUILD_DIR}/ArnPlay.app"
APP_BINARY="${APP_PATH}/Contents/MacOS/ArnPlay"
FRAMEWORKS_DIR="${APP_PATH}/Contents/Frameworks"
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

for command_name in cmake brew otool install_name_tool codesign hdiutil; do
    if ! command -v "${command_name}" >/dev/null 2>&1; then
        echo "Erro: comando obrigatório não encontrado: ${command_name}"
        exit 1
    fi
done

QT_PREFIX="$(brew --prefix qt)"
MACDEPLOYQT="${QT_PREFIX}/bin/macdeployqt"
if [[ ! -x "${MACDEPLOYQT}" ]]; then
    echo "Erro: macdeployqt não encontrado em ${MACDEPLOYQT}"
    exit 1
fi

echo "[1/6] Compilando ArnPlay ${VERSION}..."
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G Ninja \
    -DCMAKE_PREFIX_PATH="${QT_PREFIX}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES=x86_64
cmake --build "${BUILD_DIR}"

echo "[2/6] Incorporando o Qt ao aplicativo..."
"${MACDEPLOYQT}" "${APP_PATH}" -qmldir="${ROOT_DIR}/qml" -always-overwrite

mkdir -p "${FRAMEWORKS_DIR}"

is_external_library() {
    case "$1" in
        /usr/local/*|/opt/homebrew/*) return 0 ;;
        *) return 1 ;;
    esac
}

copy_dependencies() {
    local target="$1"
    local dependency real_dependency destination

    while IFS= read -r dependency; do
        is_external_library "${dependency}" || continue
        [[ -f "${dependency}" ]] || continue

        real_dependency="$(python3 -c 'import os,sys; print(os.path.realpath(sys.argv[1]))' "${dependency}")"
        destination="${FRAMEWORKS_DIR}/$(basename "${real_dependency}")"

        if [[ ! -f "${destination}" ]]; then
            cp -L "${real_dependency}" "${destination}"
            chmod u+w "${destination}"
            install_name_tool -id "@rpath/$(basename "${destination}")" "${destination}" 2>/dev/null || true
            copy_dependencies "${destination}"
        fi

        install_name_tool -change "${dependency}" "@rpath/$(basename "${destination}")" "${target}"
    done < <(otool -L "${target}" | tail -n +2 | awk '{print $1}')
}

echo "[3/6] Incorporando libmpv, FFmpeg e dependências do Homebrew..."
copy_dependencies "${APP_BINARY}"

while IFS= read -r bundled_library; do
    copy_dependencies "${bundled_library}"
done < <(find "${FRAMEWORKS_DIR}" -maxdepth 1 -type f -name '*.dylib' -print)

echo "[4/6] Aplicando assinatura local ao aplicativo..."
codesign --force --deep --sign - "${APP_PATH}"
codesign --verify --deep --strict "${APP_PATH}"

echo "[5/6] Montando o instalador DMG..."
mkdir -p "${DIST_DIR}"
rm -rf "${STAGE_DIR}"
mkdir -p "${STAGE_DIR}"
ditto "${APP_PATH}" "${STAGE_DIR}/ArnPlay.app"
ln -s /Applications "${STAGE_DIR}/Applications"
rm -f "${DMG_PATH}"
hdiutil create -volname "ArnPlay ${VERSION}" -srcfolder "${STAGE_DIR}" \
    -ov -format UDZO "${DMG_PATH}"

echo "[6/6] Verificando o resultado..."
hdiutil verify "${DMG_PATH}"

echo
echo "Pronto: ${DMG_PATH}"
echo "Instale arrastando ArnPlay para a pasta Applications."

