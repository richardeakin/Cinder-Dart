#! /usr/bin/env bash

# NOTE: currently only works on Mac OS X

PACKAGE_FILENAME="dart_runtime_libs_1.6.1_macosx_x64.zip"
PACKAGE_URL="https://dl.dropboxusercontent.com/u/3905723/cinder/dart-runtime-packages/"${PACKAGE_FILENAME}
PACKAGE_DEST="dart-runtime/lib"

DIR=$(dirname $0)
DEST_PATH=${DIR}"/"${PACKAGE_DEST}

echo "package url: ${PACKAGE_URL}"
echo "destination path: ${DEST_PATH}"

if [[ -d ${DEST_PATH} ]]
then
    echo "destination folder already exists, removing old first"
	rm -r ${DEST_PATH}
fi

curl -O ${PACKAGE_URL}
unzip ${PACKAGE_FILENAME} -d "package"
mkdir ${DEST_PATH}
mv "package/macosx" ${DEST_PATH}"/macosx"

# cleanup:
rm -rf package
rm ${PACKAGE_FILENAME}