#!/usr/bin/env python

from re import sub
from sys import stdout,stderr

def PInfo(module,msg):
  stdout.write('INFO    [%-40s]: %s\n'%(module,msg))

def PWarning(module,msg):
  stdout.write('WARNING [%-40s]: %s\n'%(module,msg))

def PDebug(module,msg):
  stderr.write('DEBUG   [%-40s]: %s\n'%(module,msg))

def PError(module,msg):
  stderr.write('ERROR   [%-40s]: %s\n'%(module,msg))

def setBins(dist,bins):
  for b in bins:
    dist.AddBinEdge(b)

def contains(s1,s2):
  if type(s2)==type(""):
    return s1.find(s2)>=0
  else:
    for s in s2:
      if s1.find(s)>=0:
        return True
    return False

def tAND(s1,s2):
  return "(( "+s1+" ) && ( "+s2+" ))"

def tOR(s1,s2):
  return "(( "+s1+" ) || ( "+s2+" ))"

def tTIMES(w,s):
  return "( "+w+" ) * ( "+s+" )"

def tNOT(w):
  return '!( '+w+' )'

def removeCut(basecut,var):
  return sub('[0-9\.]*[=<>]*%s'%(var.replace('(','\(').replace(')','\)')),'1==1',sub('%s[=<>]+[0-9\.]+'%(var.replace('(','\(').replace(')','\)')),'1==1',basecut))
