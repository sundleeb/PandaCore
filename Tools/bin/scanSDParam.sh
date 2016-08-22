#!/bin/bash

ANADIR=${CMSSW_BASE}/src/MitAna
PANDADIR=${CMSSW_BASE}/src/MitPanda

# for mass in 130 140 150 170 190; do
# for mass in 140; do 
for mass in 6 8 10 14; do
  echo "setting width = ${mass}"
  sed "s/XXX/_${mass}/g" ${ANADIR}/config/condor_SDtemplate.jdl > ${ANADIR}/config/condor_template.jdl
  sed "s/XXX/_${mass}/g" ${PANDADIR}/FlatNtupler/scripts/runFlat_SDtempl.py > ${PANDADIR}/FlatNtupler/scripts/runFlat_SDscan.py
  yes | ${ANADIR}/bin/runOnDatasets.py "$@" --fraction 0.2 --cfg ${PANDADIR}/FlatNtupler/config/samples/zprimeSmallWidth.cfg --analysis ${PANDADIR}/FlatNtupler/scripts/runFlat_SDscan.py --num-files 1 --name topTag_signal_SDWWidth${mass}
  yes | ${ANADIR}/bin/runOnDatasets.py "$@" --fraction 0.2 --cfg ${PANDADIR}/FlatNtupler/config/samples/qcdPtBinned.cfg --analysis ${PANDADIR}/FlatNtupler/scripts/runFlat_SDscan.py --num-files 1 --name topTag_qcd_SDWWidth${mass}
  sleep 2
done
