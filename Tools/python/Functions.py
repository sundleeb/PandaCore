#!/usr/bin/env python
'''@package docstring
Loads some numerical functions defined in a C++ file
'''

from ROOT import gROOT
from os import getenv

gROOT.LoadMacro(getenv('CMSSW_BASE')+'/src/PandaCore/Tools/interface/Functions.h+')
