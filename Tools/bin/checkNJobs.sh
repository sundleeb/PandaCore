#!/bin/bash

if [ "$1" == "" ]; then echo "provide name please"; exit; fi; condor_q $USER -global -long | grep UserLog | grep -c $1
