#!/bin/sh

df -h

JUCEVERSION=$1
OSFLAVOUR=$2
OUTDIR=$3

rm -rf "${OUTDIR}"
mkdir -p "${OUTDIR}"

URL=https://github.com/juce-framework/JUCE/releases/download/${JUCEVERSION}/juce-${JUCEVERSION}-${OSFLAVOUR}.zip
echo "getting juce.zip from ${URL}"

curl -L -o juce.zip "${URL}"
unzip -q juce.zip -d "${OUTDIR}"
mv "${OUTDIR}"/JUCE/* "${OUTDIR}" || true

.git-ci/getvst.sh "${OUTDIR}/modules/juce_audio_processors/format_types/VST3_SDK/pluginterfaces/vst2.x"
