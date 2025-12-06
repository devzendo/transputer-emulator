#!/bin/bash
set -euf -o pipefail

if [ $(uname) == "Darwin" ]; then
  echo "You haven't found much success building for Pico on macOS. Use the container instead!"
  exit
fi

# Move to the folder the script is in (for consistency) - https://stackoverflow.com/a/246128
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do
  DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
done
cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1

UPONE="${PWD%/*}"

#rm -rf cmake-build-release
#rm -rf cmake-build-debug
#mvn clean
export PICO_SDK_PATH=${UPONE}/PICO_SDK
#mvn -DCROSS=PICO compile -P build
./rl.py -D VARIANT="-D CROSS=PICO" clean compile



