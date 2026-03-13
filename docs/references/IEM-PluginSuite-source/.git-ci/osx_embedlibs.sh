#!/bin/sh

embed() {
  bin=$1
  otool -L "$bin" | awk '{print $1}' | grep "^/usr/local" | while read lib; do
    mkdir -p ../Frameworks/
    cp -r "${lib}" ../Frameworks/
    embedded=@loader_path/../Frameworks/${lib##*/}
    install_name_tool -change "${lib}" "${embedded}" "${bin}"
  done
}

for f in "$@"; do
  echo "embedding non-standard libraries for ${f}"
  (cd "${f%/*}"; embed "${f##*/}")
done

