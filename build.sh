#!/bin/bash
set -e

ROOT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DIST_DIR="$ROOT_DIR/dist"
CORES="$(nproc --all)"
DEBUG=0

for arg in "$@"; do
    case "$arg" in
        --debug|-d)
            DEBUG=1
            ;;
        *)
            DIST_DIR="$arg"
            ;;
    esac
done

echo
echo "*** Compiling sys-HDR ***"
TITLE_ID="$(grep -oP '"title_id":\s*"0x\K(\w+)' "$ROOT_DIR/sysmodule/perms.json")"

pushd "$ROOT_DIR/sysmodule"
# The DEBUG define isn't tracked by make's header dependency files, so force
# a clean rebuild whenever it flips between runs to avoid stale objects.
STATE_FILE=".last_debug_state"
LAST_DEBUG="$(cat "$STATE_FILE" 2>/dev/null || echo "")"
if [[ "$LAST_DEBUG" != "$DEBUG" ]]; then
    make clean
fi
echo "$DEBUG" > "$STATE_FILE"

make -j$CORES DEBUG=$DEBUG
popd > /dev/null

mkdir -p "$DIST_DIR/atmosphere/contents/$TITLE_ID/flags"
cp -vf "$ROOT_DIR/sysmodule/out/sys-hdr.nsp" "$DIST_DIR/atmosphere/contents/$TITLE_ID/exefs.nsp"
>"$DIST_DIR/atmosphere/contents/$TITLE_ID/flags/boot2.flag"
cp -vf "$ROOT_DIR/sysmodule/toolbox.json" "$DIST_DIR/atmosphere/contents/$TITLE_ID/toolbox.json"

echo
echo "*** Compiling sys-HDR overlay ***"
pushd "$ROOT_DIR/overlay"
make -j$CORES
popd > /dev/null

mkdir -p "$DIST_DIR/switch/.overlays"
cp -vf "$ROOT_DIR/overlay/sys-hdr.ovl" "$DIST_DIR/switch/.overlays/sys-hdr.ovl"

echo
echo "*** Copying assets ***"
mkdir -p "$DIST_DIR/config/sys-hdr"
cp -vf "$ROOT_DIR/config.ini.template" "$DIST_DIR/config/sys-hdr/config.ini.template"

if [[ "$DEBUG" == "1" ]]; then
    echo
    echo "*** Debug build: sys-hdr will log to sdmc:/config/sys-hdr/sys-hdr.log ***"
fi
echo
