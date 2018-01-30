#!/usr/bin/env python

from sys import argv
fpath = argv[1]
argv = []

from ROOT import TFile, TTree

fIn = TFile(fpath)
events = fIn.Get('events')

class lumi(object):
  def __init__(self,low):
    self.low = low
    self.high = low
  def __str__(self):
    return "[%i,%i]"%(self.low,self.high)
  def makeList(self):
    return [self.low,self.high]

unsortedLumis = {}

for iE in xrange(events.GetEntries()):
  events.GetEntry(iE)
  if events.runNumber in unsortedLumis:
    unsortedLumis[events.runNumber].append(events.lumiNumber)
  else:
    unsortedLumis[events.runNumber] = [events.lumiNumber]

sortedLumis = {}

for run,lumiList in unsortedLumis.iteritems():
  lumiList.sort()
  shortList = []
  current = None
  for l in lumiList:
    if current == None:
      current = lumi(l)
    elif (l-current.high)>1:
      # if we are skipping more than one
      shortList.append(current.makeList())
      current = lumi(l)
    else:
      # we are continuing the current set of lumis
      current.high = l
  shortList.append(current.makeList()) # add the last one
  sortedLumis["%i"%(run)] = shortList

print sortedLumis

