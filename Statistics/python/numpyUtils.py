#!/usr/bin/env python

import ROOT as root
#import root_numpy as rnp
import numpy as np
from PandaCore.Tools.Misc import *

def treeToArr(tree,formulae,cut=None,weight=None):
  if weight:
    tupleArray = rnp.tree2array(tree,branches=formulae,selection=cut,include_weight=True,weight_name=weight)
  else:
    tupleArray = rnp.tree2array(tree,branches=formulae,selection=cut)
  return tupleArray

