#!/bin/sh

# script to download and "install" FFTW for windows, so we can use it for juce
# see http://www.fftw.org/install/windows.html

bits="$1"
libexe="Lib.exe"
outdirbase="resources/fftw3"
fftwversion=3.3.5

if [ "x${bits}" = "x" ]; then bits="32 64"; fi

def2lib() {
 for f3 in libfftw3*.*; do
   f=$(echo libfftw${f3#libfftw3} | sed -e 's|-|-3.|')
   mv -v "${f3}" "${f}"
   def=${f%.def}
   if [ "${f}" != "${def}" ]; then
     sed -e "1s/LIBRARY ${f3%.def}.dll/LIBRARY ${f%.def}.dll/" -i "${f}"
   fi
 done
 for def in libfftw*.def; do
  "${libexe}" -machine:${machine} -def:"${def}"
 done
}

for b in ${bits}; do
  fftw="fftw-${fftwversion}-dll${b}.zip"
  outdir="${outdirbase}/x${b}"
  machine="x${b}"
  if [ "x${b}" = "x32" ]; then
    outdir="${outdirbase}/win${b}"
    machine=x86
  fi
  rm -rf "${outdir}"
  mkdir -p "${outdir}"

  url="ftp://ftp.fftw.org/pub/fftw/${fftw}"

# install fftw3
  echo "extracting ${url} to ${outdir}"

  curl -o "${fftw}" "${url}"
  unzip -q "${fftw}" -d "${outdir}"
  (cd "${outdir}"; def2lib)
  rm "${fftw}"
done
