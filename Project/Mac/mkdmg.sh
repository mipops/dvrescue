#!/bin/sh

if [ $# != 3 ]; then
    echo
    echo "Usage: mkdmg.sh appname kind version"
    echo
    exit 1
fi

APPNAME="$1"
KIND="$2"
VERSION="$3"

if [ "$KIND" = "CLI" ] || [ "$KIND" = "cli" ]; then
    KIND="CLI"
elif [ "$KIND" = "GUI" ] || [ "$KIND" = "gui" ]; then
    KIND="GUI"
else
    echo
    echo "KIND must be either [CLI | cli] or [GUI | gui]"
    echo
    exit 1
fi

APPNAME_lower=`echo ${APPNAME} |awk '{print tolower($0)}'`
KIND_lower=`echo ${KIND} |awk '{print tolower($0)}'`
SIGNATURE="MediaArea.net"
FILES="tmp-${APPNAME}_${KIND}"
TEMPDMG="tmp-${APPNAME}_${KIND}.dmg"
FINALDMG="${APPNAME/ /}_${KIND}_${VERSION}_Mac.dmg"

# Clean up
rm -fr "${FILES}-Root"
rm -fr "${FILES}"
rm -f "${APPNAME}.pkg"
rm -f "${TEMPDMG}"
rm -f "${FINALDMG}"

echo
echo ========== Create the package ==========
echo

mkdir -p "${FILES}/.background"
cp "Logo_White.icns" "${FILES}/.background"
cp ../../LICENSE.txt "${FILES}"

if [ "$KIND" = "CLI" ]; then

    cd ../GNU/CLI
    if test -e ".libs/${APPNAME_lower}"; then
        mv -f ".libs/${APPNAME_lower}" .
    fi
    if ! test -x "${APPNAME_lower}"; then
        echo
        echo "${APPNAME_lower} can’t be found, or this file isn’t a executable."
        echo
        exit 1
    fi
    strip -u -r "${APPNAME_lower}"
    cd ../../Mac

    mkdir -p "${FILES}-Root/usr/local/bin"
    mkdir -p "${FILES}-Root/usr/local/share/man/man1"
    cp "../GNU/CLI/${APPNAME_lower}" "${FILES}-Root/usr/local/bin"
    cp "../../tools/dvloupe" "${FILES}-Root/usr/local/bin"
    cp "../../tools/dvmap" "${FILES}-Root/usr/local/bin"
    cp "../../tools/dvpackager" "${FILES}-Root/usr/local/bin"
    cp "../../tools/dvplay" "${FILES}-Root/usr/local/bin"
    cp "../../tools/dvsampler" "${FILES}-Root/usr/local/bin"
    cp "../../tools/avfctl/avfctl" "${FILES}-Root/usr/local/bin"
    cp "../../Source/CLI/dvrescue.1" "${FILES}-Root/usr/local/share/man/man1"
    codesign -f --options=runtime -s "Developer ID Application: ${SIGNATURE}" --verbose "${FILES}-Root/usr/local/bin/${APPNAME_lower}"
    codesign -f --options=runtime -s "Developer ID Application: ${SIGNATURE}" --verbose "${FILES}-Root/usr/local/bin/avfctl"

    pkgbuild --root "${FILES}-Root" --identifier "net.MediaArea.${APPNAME_lower}.mac-${KIND_lower}" --sign "Developer ID Installer: ${SIGNATURE}" --version "${VERSION}" "${FILES}/${APPNAME_lower}.pkg"
    codesign -f --options=runtime -s "Developer ID Application: ${SIGNATURE}" --verbose "${FILES}/${APPNAME_lower}.pkg"

fi

if [ "$KIND" = "GUI" ]; then
    if ! test -e "../../Source/GUI/dvrescue/build/dvrescue/${APPNAME}.app"; then
        echo
        echo "${APPNAME}.app can’t be found, or this file isn’t a executable."
        echo
        exit 1
    fi
    cp -R "../../Source/GUI/dvrescue/build/dvrescue/${APPNAME}.app" "${FILES}"

    # first pass, sign everything
    codesign --identifier "net.MediaArea.${APPNAME_lower}.mac-${KIND_lower}" --verbose --force --deep --options=runtime --preserve-metadata=entitlements,identifier --sign="Developer ID Application: ${SIGNATURE}" "${FILES}/${APPNAME}.app"
    codesign --identifier "net.MediaArea.${APPNAME_lower}.mac-${KIND_lower}.libs" --verbose --force --options=runtime --sign="Developer ID Application: ${SIGNATURE}" "${FILES}/${APPNAME}.app/Contents/Libraries/"*
    # add entitlements
    codesign --identifier "net.MediaArea.${APPNAME_lower}.mac-${KIND_lower}" --verbose --force --options=runtime --sign="Developer ID Application: ${SIGNATURE}" --entitlements "../../Source/GUI/dvrescue/dvrescue/dvrescue.entitlements" "${FILES}/${APPNAME}.app/Contents/MacOS/dvrescue"
    codesign --identifier "net.MediaArea.${APPNAME_lower}.mac-${KIND_lower}.helpers" --verbose --force --options=runtime --sign="Developer ID Application: ${SIGNATURE}" "${FILES}/${APPNAME}.app/Contents/Helpers/"*
    # second pass, resign
    codesign --identifier "net.MediaArea.${APPNAME_lower}.mac-${KIND_lower}" --verbose --force --deep --options=runtime --preserve-metadata=entitlements,identifier --sign="Developer ID Application: ${SIGNATURE}" "${FILES}/${APPNAME}.app"
fi

echo
echo ========== Create the disk image ==========
echo

# Check if an old image isn't already attached
DEVICE=$(hdiutil info |grep -B 1 "/Volumes/${APPNAME}" |egrep '^/dev/' | sed 1q | awk '{print $1}')
test -e "$DEVICE" && hdiutil detach -force "${DEVICE}"

hdiutil create "${TEMPDMG}" -ov -fs HFS+ -format UDRW -volname "${APPNAME}" -srcfolder "${FILES}"
DEVICE=$(hdiutil attach -readwrite -noverify "${TEMPDMG}" | egrep '^/dev/' | sed 1q | awk '{print $1}')
sleep 2

cd "/Volumes/${APPNAME}"
if [ "$KIND" = "GUI" ]; then
    ln -s /Applications
fi
test -e .DS_Store && rm -fr .DS_Store
cd - >/dev/null

. Osascript_${KIND}.sh
osascript_Function

hdiutil detach "${DEVICE}"
sleep 2

echo
echo ========== Convert to compressed image ==========
echo
hdiutil convert "${TEMPDMG}" -format UDBZ -o "${FINALDMG}"
#codesign -f --deep --options=runtime -s "Developer ID Application: ${SIGNATURE}" --verbose "${FINALDMG}"

unset -v APPNAME APPNAME_lower KIND KIND_lower VERSION SIGNATURE
unset -v TEMPDMG FINALDMG FILES DEVICE
