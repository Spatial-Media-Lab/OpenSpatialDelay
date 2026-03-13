#!/bin/sh

## script for creating the release binaries on
rm -rf build
rm -rf VST
# rm -rf VST3
rm -rf LV2
rm -rf Standalone

mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DIEM_BUILD_VST2=ON -DVST2SDKPATH=src/VST_SDK/VST2_SDK/ -DIEM_BUILD_LV2=ON -DIEM_BUILD_STANDALONE=ON -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build .

cd ..
mkdir VST
cp -r build/*/*_artefacts/Release/VST/*.vst VST/
# mkdir VST3
# cp -r build/*/*_artefacts/Release/VST3/*.vst3 VST3/
mkdir LV2
cp -r build/*/*_artefacts/Release/LV2/*.lv2 LV2/
mkdir Standalone
cp -r build/*/*_artefacts/Release/Standalone/*.app Standalone/
