#!/usr/bin/env python
'''@package docstring
Some common python functions
'''

from re import sub
from sys import stdout,stderr

def PInfo(module,msg):
  ''' function to write to stdout'''
  stdout.write('INFO    [%-40s]: %s\n'%(module,msg))

def PWarning(module,msg):
  ''' function to write to stdout'''
  stdout.write('WARNING [%-40s]: %s\n'%(module,msg))

def PDebug(module,msg):
  ''' function to write to stdout'''
  stderr.write('DEBUG   [%-40s]: %s\n'%(module,msg))

def PError(module,msg):
  ''' function to write to stdout'''
  stderr.write('ERROR   [%-40s]: %s\n'%(module,msg))

def setBins(dist,bins):
  ''' Given a list of bin edges, sets them for a PlotUtility::Distribution '''
  for b in bins:
    dist.AddBinEdge(b)

def contains(s1,s2):
  ''' uh... why did I write a function for this? '''
  if type(s2)==type(""):
    return s1.find(s2)>=0
  else:
    for s in s2:
      if s1.find(s)>=0:
        return True
    return False

def tAND(s1,s2):
  ''' ANDs two strings '''
  return "(( "+s1+" ) && ( "+s2+" ))"

def tOR(s1,s2):
  ''' ORs two strings'''
  return "(( "+s1+" ) || ( "+s2+" ))"

def tTIMES(w,s):
  ''' MULTIPLIES two strings'''
  return "( "+w+" ) * ( "+s+" )"

def tNOT(w):
  ''' NOTs two strings'''
  return '!( '+w+' )'

def removeCut(basecut,var):
  ''' 
  Removes the dependence on a particular variable from a formula

  @type basecut: str
  @param basecut: Input formula to modify

  @type var: str
  @param var: Variable to remove from basecut

  @rtype: string
  @return: Returns a formula with the var dependence removed
  '''
  return sub('[0-9\.]*[=<>]*%s'%(var.replace('(','\(').replace(')','\)')),'1==1',sub('%s[=<>]+[0-9\.]+'%(var.replace('(','\(').replace(')','\)')),'1==1',basecut))
