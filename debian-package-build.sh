#!/usr/bin/env bash

DEBIAN_BUILD_DIR="build-debian"
BUILD_DIR=$1
VERSION=$2
PACKAGE_NAME=regress-pro

function print_usage {
    echo "Usage: $0 <build-dir> <version>"
    exit 1
}

if [ -z $1 -o -z $2 ]
then
    print_usage
fi

rm -f -r ${DEBIAN_BUILD_DIR}
mkdir -p ${DEBIAN_BUILD_DIR}/usr/bin ${DEBIAN_BUILD_DIR}/usr/share/applications ${DEBIAN_BUILD_DIR}/usr/share/icons/hicolor/128x128/apps ${DEBIAN_BUILD_DIR}/usr/share/regress-pro
cp ${BUILD_DIR}/fox-gui/regress ${DEBIAN_BUILD_DIR}/usr/bin
cp -R examples ${DEBIAN_BUILD_DIR}/usr/share/regress-pro
cp src/regress-pro-128x128.png ${DEBIAN_BUILD_DIR}/usr/share/icons/hicolor/128x128/apps/regress-pro.png
fakeroot bash debian/build.sh ${PACKAGE_NAME} ${VERSION}
