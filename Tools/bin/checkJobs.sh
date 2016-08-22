#!/bin/bash

if [ "$1" == "" ]; then echo "provide name please"; exit; fi; for jid in $( condor_q $USER -global | grep -o "^[0-9]*\." ); do if [ "$(condor_q $jid -global -long | grep UserLog | grep $1)" != "" ]; then condor_q $jid | tail -n 1; fi;  done
