#!/usr/bin/env python

'''
  Module containing simple statistical calculations

  Author: Sid Narayanan < sidn AT mit DOT edu >
'''

import ROOT as root
#import root_numpy as rnp
import numpy as np
from PandaCore.Tools.Misc import *
from RooFitUtils import *
from numpyUtils import *

class SimpleVar(object):
  ''' 
    Simple wrapper object to hold a formula and its bounds.
    Automatically creates a RooRealVar.
  '''
  def __init__(self,name,lo,hi,nbins=None,title=None):
    self.name=name
    self.lo=lo
    self.hi=hi
    self.title = title if title else name
    self.rvar = root.RooRealVar(self.name,self.title,self.lo,self.hi)
    self.nbins = nbins if nbins else 50 # is a dummy value if only used in unbinned context

def calcUnbinnedKSFromMC(svars,mcTree,dataTree,mcCut,mcWeight,dataCut=None,returnPlot=False):
  '''
    Calculate unbinned Kolmogorov Smirnoff probability.
    Given TTrees of data and MC simulation,
    builds a KDE pdf out of the simulated data points,
    then calculates the KS stat
  '''
  if not dataCut:
    dataCut=mcCut
  svar=svars[0]
  kde = treeToKDE(mcTree,[s.rvar for s in svars],mcCut,mcWeight)
  obs = sorted(list(treeToArr(dataTree,[svar.name],dataCut)[svar.name]))
  cdf = kde.createCdf(root.RooArgSet(svar.rvar))
  ks = 0
  nD = len(obs)
  for iD in xrange(nD):
    x = obs[iD]
    y_data = 1.*iD/nD
    svar.rvar.setVal(x)
    y_mc = cdf.getVal()
    ks = max(ks,abs(y_data-y_mc))
  # convert ks to a probability
  mD = mcTree.GetEntries(mcCut)
  prob = root.TMath.KolmogorovProb( ks * np.sqrt(dM*dN/(dM+dN)) ) 
  if not returnPlot:
    return prob
  else:
    PError("PandaCore.SimpleStats.calcUnbinnedKSFromMC","Returning plots is not implemented yet")
    return prob

def calcBinnedKS(svar,mcTree,dataTree,mcCut,mcWeight,dataCut=None,returnPlot=False):
  '''
    Calculate binned KS statistic.
    Given TTrees of data and MC simulation,
    builds a binned PDF (histogram) and then
    calculates the KS stat
  '''
  if not dataCut:
    datacut=mcCut
  hdata = root.TH1D('hdata','hdata',svar.nbins,svar,lo,svar.hi)
  hmc = hdata.Clone('hmc')
  mcTree.Draw('%s>>hmc'%(svar.name),tTIMES(mcWeight,mcCut))
  dataTree.Draw('%s>>hdata'%(svar.name),dataCut)
  prob = hmc.KolmogorovTest(hdata)
  if not returnPlot:
    return prob
  else:
    PError("PandaCore.SimpleStats.calcBinnedKS","Returning plots is not implemented yet")
    return prob
