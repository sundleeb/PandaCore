#!/usr/bin/env python

from Misc import *

class Library():
  def __init__(self,name,pkg,d):
    self.name = name
    self.pkg = pkg
    self.d = d

objects = {
  'Learning' : Library(name='Learning',pkg='PandaCore',
                        d={
                            'TMVATrainer':['TMVATrainer'],
                            'TMVABranchAdder':['TMVABranchAdder'],
                          }),
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
  'SCRAMJetAnalyzer' : Library(name='Analyzer',pkg='SCRAMJet',
                        d={
                            'Analyzer':['Analyzer'],
                          }),
}

from ROOT import gROOT, gSystem, gInterpreter
import ROOT as root
from os import getenv

def Load(lib,obj):
  gInterpreter.AddIncludePath("%s/src/fastjet/include/"%(getenv('CMSSW_BASE'))) # why isn't this set by CMSSW...?
  includepath=None
  subpackage = objects[lib]
  for header in subpackage.d:
    if obj in subpackage.d[header]:
      includepath = "${CMSSW_BASE}/src/%s/%s/interface/%s.h"%(subpackage.pkg,subpackage.name,header)
      break
  if includepath==None:
    PError('PandaCore.Tools.Load','Could not load %s from %s'%(obj,lib))
    return
  libpath = "lib%s%s.so"%(subpackage.pkg,subpackage.name)
  PInfo('PandaCore.Tools.Load','Loading %s'%libpath)
  gSystem.Load(libpath)
  if 'SCRAMJet' in lib:
    for objpath in ['libSCRAMJetSDAlgorithm.so','libSCRAMJetObjects.so','libfastjet.so','libfastjetcontribfragile.so','libfastjetplugins.so','libfastjettools.so']:
      PInfo('PandaCore.Tools.Load','Loading %s'%objpath)
      gSystem.Load(objpath)
  if 'Nero' in obj:
    neropath = 'libNeroProducerCore.so'
    PInfo('PandaCore.Tools.Load','Loading %s'%neropath)
    gSystem.Load(neropath)
  PInfo('PandaCore.Tools.Load','Including %s'%includepath)
  gROOT.LoadMacro(includepath)
