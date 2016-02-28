#! /usr/bin/env bash

case "$(uname -s)" in
    Darwin)
		  echo 'Detected Mac OS X'
		  PACKAGE_FILENAME="dart_runtime_libs_1.8.5_macosx_x64.zip"
     	;;
   	Linux)
    	echo 'Error: Linux Platform not yet supported, exiting.'
    	exit 0
     	;;
  	CYGWIN*|MINGW*|MSYS*)
     	echo 'Detected MS Windows'
		  PACKAGE_FILENAME="dart_runtime_libs_1.8.5_msw_x64.zip"
     	;;
  	*)
    	echo 'Error: Unknown OS, returning.' 
    	exit 0
    	;;
esac

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
mv -v "package/lib/"* ${DEST_PATH}"/"

# cleanup:
rm -rf package
rm ${PACKAGE_FILENAME}
