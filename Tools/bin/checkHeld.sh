#!/bin/bash

for errfile in $(for jid in $(condor_q $USER | grep H | grep -o "^[0-9]*"); do condor_q $jid -long | grep ^Err | sed 's/Err[^"]*"//' | sed 's/"//' ; done); do echo ${errfile}; grep -i error ${errfile}; done
