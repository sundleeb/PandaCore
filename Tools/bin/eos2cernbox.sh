#!/bin/bash

/afs/cern.ch/project/eos/installation/0.3.84-aquamarine.user/bin/eos.select mkdir -p eos/user/s/snarayan/monotop80/v1.8/${1}

xrdcp --parallel 4 -r root://eoscms//eos/cms/store/user/snarayan/monotop80/v1.8/${1}  root://eosuser//eos/user/s/snarayan/monotop80/v1.8/${1}/
