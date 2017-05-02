#!/usr/bin/env python
'''@package docstring
Module that catalogs C++ objects known to ROOT/PandaCore.
Load libraries on-demand
'''

from Misc import *

class Library():
    '''Simple class that defines a library'''
    def __init__(self,name,objects,deps=[]):
        self.name = name
        self.objects = objects
        self.deps = deps

loaded = []

libraries = [
    Library(name='PandaCoreTools', objects = [ 'Functions',
                                               'Common',
                                               'TreeTools',
                                               'DuplicateRemover',
                                               'Normalizer',
                                               'Cutter',
                                               'BranchAdder',
                                               'EventSyncher', 
                                             ]
           ),
    Library(name='PandaCoreLearning', objects = [ 'TMVATrainer',
                                                  'TMVABranchAdder',
                                                ]
           ),
    Library(name='PandaCoreStatistics', objects = [ 'RooExpErf',
                                                  ]
           ),
    Library(name='PandaCoreDrawers',objects = ['CanvasDrawer',
                                               'GraphDrawer',
                                               'GraphErrDrawer',
                                               'GraphAsymmErrDrawer',
                                               'HistogramDrawer',
                                               'PlotUtility',
                                               'ROCTool',
                                              ]
           ),
    Library(name='PandaAnalysisFlat',objects = ['PandaAnalyzer',
                                                'GenAnalyzer',
                                                'LimitTreeBuilder',
                                                'SFTreeBuilder',
                                                'BTagTreeBuilder',
                                               ],
            ), 
    Library(name='RedPandaCluster',objects = ['Clusterer',] ),
]
# TODO: refactor SCRAMJet to have dictionary interface, will need following libs:
# 'libfastjet.so','libfastjetcontribfragile.so','libfastjetplugins.so','libfastjettools.so'

from ROOT import gROOT, gSystem, gInterpreter
import ROOT as root
from os import getenv

def load_lib(libpath):
    PInfo('PandaCore.Tools.Load','Loading %s'%libpath)
    gSystem.Load(libpath)


def Load(request):
    '''
    Function that loads any necessary shared object files 

    @type request: str
    @param request: Name of library or class
    '''

    requested_lib = None
    for l in libraries:
        if l.name==request or request in l.objects:
            requested_lib = l
            break

    if requested_lib in loaded:
        PWarning('PandaCore.Tools.Load','Requested %s has already been loaded'%(request))
        return

    if not requested_lib:
        PError('PandaCore.Tools.Load','Could not load lib %s'%request)

    for d in requested_lib.deps:
        if 'CMSSW' in d:
            load_lib(d)
        else:
            load_lib('lib'+d+'.so')

    load_lib('lib'+requested_lib.name+'.so')
