#!/bin/bash

pd=${1}

#for f in $(ls ${CERNBOX}/monotop80/v1.8/${pd}/${pd}/*/*/*.root)
for f in $(ls ${CERNBOX}/monotop80/v1.8/${pd}/${pd}_Run2016B-PromptReco-v2/*/*/*.root)
do
  if [ -f $f ]
  then
    if [ "$(du ${f})" != "0" ]
    then
      target=$(echo $f | sed "s?${CERNBOX}?${EOS}?")
      echo "rm $target"
      rm $target
    fi
  fi
done
