#!/usr/bin/env python

from Misc import *

class Libary():
  def __init__(self,name,pkg,d):
    self.name = name
    self.lib = lib
    self.d = d

objects = {
  'Tools' : Library(name='Tools', pkg='PandaCore',
                        d={
                            'DuplicateRemover':['DuplicateRemover'],
                            'Normalizer':['Normalizer'],
                            'BranchAdder':['BranchAdder']
                          }),
  'Drawers' : Library(name='Drawers',pkg='PandaCore',
                      d={
                            'CanvasDrawer':['CanvasDrawer'],
                            'GraphDrawer':['GraphDrawer','GraphErrDrawer','GraphAsymmErrDrawer'],
                            'HistogramDrawer':['HistogramDrawer'],
                            'PlotUtility':['PlotUtility','Process','Distribution'],
                            'ROCTool':['ROCTool'],
                      }),
  'Skimmers' : Library(name='Skimmers',pkg='MitPanda',
                        d={
                            'LimitTreeBuilder':['VariableMap','Process','Region','LimitTreeBuilder'],
                            'NeroSkimmer':['NeroSkimmer'],
                            'SFSkimmer':['SFSkimmer'],
                          }),
}

from ROOT import gROOT, gSystem
import ROOT as root

def Load(lib,obj):
  includepath=None
  subpackage = objects[lib]
  for header in subpackage.d:
    if obj in subpackage.d[header]:
      includepath = "${CMSSW_BASE}/src/%s/%s/interface/%s.h"%(subpackage.pkg,subpackage.name,header)
      break
  if includepath==None:
    PError('PandaCore.Tools.Load','Could not load %s from %s'%(obj,lib))
    return
  PInfo('PandaCore.Tools.Load','Including %s'%includepath)
  gROOT.LoadMacro(includepath)
  libpath = "lib%s%s.so"%(subpackage.pkg,subpackage.name)
  PInfo('PandaCore.Tools.Load','Loading %s'%libpath)
  gSystem.Load(libpath)
  if 'Nero' in obj:
    neropath = 'libNeroProducerCore.so'
    PInfo('MitPanda.Tools.Load','Loading %s'%neropath)
    gSystem.Load(neropath)
