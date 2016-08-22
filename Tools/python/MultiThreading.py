#!/usr/bin/env python

from ROOT import gSystem,gROOT
import ROOT as root
from multiprocessing import Process,Pool

def globalTarget(s,x):
  print "starting ",s.summary(x)
  r = s.target(x)
  print "finishing ",s.summary(x)
  return r

class Runner(object):
  target = None
  def __init__(self):
#    self.target = None
    self.argList = None
  def setArgList(self,l):
    self.argList = l
  def summary(self,x):
    return
  def run(self,nProc):
    if self.argList is None:
      print 'please call setArgList() first'
      return None
    p = Pool(processes=nProc)
    results = [p.apply_async(globalTarget, args = [self,arg]) for arg in self.argList]
    p.close()
    p.join()
    return results
    
class GenericRunner(Runner):
  def __init__(self,f):
    super(self.__class__,self).__init__()
    Runner.target = lambda s,x : f(*x)
  def summary(self,x):
    return "calling with args %s"%(str(x))

class MacroRunner(Runner):
  def __init__(self,macroName,functionName=None,libs=[], incs=[]):
    super(self.__class__,self).__init__()
    for lib in libs:
      gSystem.Load(lib)
    for inc in incs:
      gROOT.LoadMacro(inc)
    gSystem.CompileMacro(macroName)
    if functionName is not None:
      Runner.target = lambda s,x : getattr(root,functionName)(*x)
      self.functionName = functionName
    else:
      Runner.target = lambda s,x : getattr(root,macroName.replace('.C',''))(*x)
      self.functionName = macroName.replace('.C','')
    self.argList = None
  def summary(self,x):
    return "%s(*%s)"%(self.functionName,str(x))
