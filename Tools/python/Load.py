#!/usr/bin/env python
'''@package docstring
Module that catalogs C++ objects known to ROOT/PandaCore.
Can load the libraries and object definitions on-demand
'''

from Misc import *

class Library():
	'''Simple class that defines a library'''
	def __init__(self,name,pkg,d):
		self.name = name
		self.pkg = pkg
		self.d = d

objects = {
	'Learning' : Library(name='Learning',pkg='PandaCore',
												d={
														'TMVATrainer'				 :['TMVATrainer'],
														'TMVABranchAdder'		 :['TMVABranchAdder'],
													}),
	'Tools' : Library(name='Tools', pkg='PandaCore',
												d={
														'Functions'					 :['Functions'],
														'Common'							:['Common'],
														'TreeTools'					 :['TreeTools'],
														'DuplicateRemover'		:['DuplicateRemover'],
														'Normalizer'					:['Normalizer'],
														'Cutter'							:['Cutter'],
														'BranchAdder'				 :['BranchAdder'],
													}),
	'Drawers' : Library(name='Drawers',pkg='PandaCore',
											d={
														'CanvasDrawer'				:['CanvasDrawer'],
														'GraphDrawer'				 :['GraphDrawer','GraphErrDrawer','GraphAsymmErrDrawer'],
														'HistogramDrawer'		 :['HistogramDrawer'],
														'PlotUtility'				 :['PlotUtility','Process','Distribution'],
														'ROCTool'						 :['ROCTool'],
											}),
	'SCRAMJetAnalyzer' : Library(name='Analyzer',pkg='SCRAMJet',
												d={
														'Analyzer'						:['Analyzer'],
													}),
	'PandaAnalysisFlat' : Library(name='Flat',pkg='PandaAnalysis',
												d={
														'PandaAnalyzer'			 :['PandaAnalyzer'],
														'GenAnalyzer'				 :['GenAnalyzer'],
														'LimitTreeBuilder'		:['LimitTreeBuilder'],
														'SFTreeBuilder'			 :['SFTreeBuilder'],
													}),
	'Statistics'				: Library(name='Statistics',pkg='PandaCore',
													d={
															'RooExpErf'				 :['RooExpErf'],
													}),
}

from ROOT import gROOT, gSystem, gInterpreter
import ROOT as root
from os import getenv

def Load(lib,obj):
	'''
	Function that loads a shared object file and any header files required for a particular object

	@type lib: str
	@param lib: Name of library

	@type obj: str
	@param obj: Name of object
	'''
	root_include_path = getenv('ROOT_INCLUDE_PATH')
	for p in root_include_path.split(":"):
		gInterpreter.AddIncludePath(p)
	gInterpreter.AddIncludePath('/cvmfs/cms.cern.ch/slc6_amd64_gcc530/external/fastjet-contrib/1.020/include/')
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
