#!/usr/bin/env python
'''@package docstring
Loads some numerical functions defined in a C++ file
'''

from os import getenv
from Load import Load
from ROOT import gROOT

Load('Functions')
#gROOT.LoadMacro(getenv('CMSSW_BASE')+'/src/PandaCore/Tools/interface/Functions.h')
