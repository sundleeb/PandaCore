#!/usr/bin/env python

from ROOT import gROOT
from os import getenv

gROOT.LoadMacro(getenv('CMSSW_BASE')+'/src/PandaCore/Tools/interface/Functions.h+')
