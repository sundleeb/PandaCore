from ROOT import gPad,TH2D,TH1D,TFile,TChain,TTree,gROOT,TCanvas
import ROOT as root
from array import array as arr
from numpy import log2,sqrt,array,empty
from PandaCore.Tools.Misc import *

hcounter=0

def getCov(t,AVar,BVar,cut='',aBins=None,bBins=None):
  global hcounter
  if aBins!=None and bBins!=None:
    t.Draw("%s:%s>>htmp(%i,%f,%f,%i,%f,%f)"%(AVar,BVar,bBins[0],bBins[1],bBins[2],aBins[0],aBins[1],aBins[2]),cut,"colz")
  else:
    t.Draw("%s:%s>>htmp"%(AVar,BVar),cut,"colz")
  htmp = gPad.GetPrimitive('htmp')
  hcounter += 1
  return htmp.GetCovariance()

def getPearson(t,AVar,BVar,cut='',aBins=None,bBins=None):
  global hcounter
  if aBins!=None and bBins!=None:
    htmp = TH2D('h%i'%hcounter,'h%i'%hcounter,aBins[0],aBins[1],aBins[2],bBins[0],bBins[1],bBins[2])
    t.Draw("%s:%s>>h%i(%i,%f,%f,%i,%f,%f)"%(AVar,BVar,hcounter,bBins[0],bBins[1],bBins[2],aBins[0],aBins[1],aBins[2]),cut,"colz")
    htmp = getattr(root,'h%i'%hcounter)
  else:
    t.Draw("%s:%s>>h%i"%(AVar,BVar,hcounter),cut,"colz")
    htmp = getattr(root,'h%i'%hcounter)
  hcounter += 1
  return htmp.GetCorrelationFactor()

class SquarePlotter(object):
  def __init__(self,fsigpath,fbgpath,treename):
    self.fsig = TFile(fsigpath); self.tsig = self.fsig.Get(treename)
    self.fbg = TFile(fbgpath); self.tbg = self.fbg.Get(treename)
    gPad.cd()
  def makeCovPlots(self,xs,sigcut='',bgcut=''):
    # format of xs is [ [var,title,(optional nbins, binlo, binhi)] ]
    nX = len(xs)
    h2 = TH2D('hcov','hcov',nX,0,nX,nX,0,nX)
    # label X and Y in opposite order
    for iB in xrange(1,nX+1):
      h2.GetXaxis().SetBinLabel(iB,xs[iB-1][1])
      h2.GetYaxis().SetBinLabel(nX-iB+1,xs[iB-1][1])
    h2bg = h2.Clone('hcovbg')
    for iX in xrange(nX):
      for iY in xrange(iX,nX):
        x = xs[iX]; y = xs[iY];
        xbins = None if len(x)<3 else x[2]
        ybins = None if len(y)<3 else y[2]
        covsig = getCov(self.tsig,x[0],y[0],sigcut,xbins,ybins)
        h2.SetBinContent(iX+1,nX-iY,covsig)
        covbg = getCov(self.tbg,x[0],y[0],bgcut,xbins,ybins)
        h2bg.SetBinContent(iX+1,nX-iY,covbg)
    return h2,h2bg
  def makeCorrPlots(self,xs,sigcut='',bgcut=''):
    # format of xs is [ [var,title,(optional nbins, binlo, binhi)] ]
    nX = len(xs)
    h2 = TH2D('hcorr','hcorr',nX,0,nX,nX,0,nX)
    h2.GetZaxis().SetTitle('Correlation')
    # label X and Y in opposite order
    for iB in xrange(1,nX+1):
      h2.GetXaxis().SetBinLabel(iB,xs[iB-1][1])
      h2.GetYaxis().SetBinLabel(nX-iB+1,xs[iB-1][1])
    h2bg = h2.Clone('hcorrbg')
    for iX in xrange(nX):
      for iY in xrange(iX,nX):
        x = xs[iX]; y = xs[iY];
        if not(x==y):
          xbins = None if len(x)<3 else x[2]
          ybins = None if len(y)<3 else y[2]
          PInfo("SquarePlotter.makeCorrPlots","%s vs %s"%(x[0],y[0]))
          corrsig = getPearson(self.tsig,x[0],y[0],sigcut,xbins,ybins)
          corrbg = getPearson(self.tbg,x[0],y[0],bgcut,xbins,ybins)
        else:
          corrsig=1
          corrbg=1
        h2.SetBinContent(iX+1,nX-iY,corrsig)
        h2bg.SetBinContent(iX+1,nX-iY,corrbg)
    return h2,h2bg

## everything below this needs to be rewritten because it is shitty shitty code

class MutualInformer(object):
  def __init__(self,tree):
    # note tA and tB can be the same tree, just want to avoid case where truth value is in a diff tree
    # although easy solution is to make it a friend
    self.t = tree
    self.cachedABT=None
    self.cachedAB=None
    self.Aname=""
    self.Bname=""
  def __H1(self,varName,nBins,low,high,cut=""):
    usingCached = False
    if self.cachedABT:
      # only single variable entropy to compute for mi w/truth is for truth
      htmp = self.cachedABT.ProjectionZ('_pz')
      usingCached = True
    if self.cachedAB and not usingCached:
      if varName==self.Aname:
        htmp = self.cachedAB.ProjectionX("_px")
        usingCached = True
      elif varName==self.Bname:
        htmp = self.cachedAB.ProjectionY("_py")
        usingCached = True
    if not usingCached:
      self.t.Draw("%s>>htmp(%i,%f,%f)"%(varName,nBins,low,high),cut)
      htmp = gPad.GetPrimitive('htmp')
    entropy=0
    integral = 1.*htmp.Integral()
    for iB in xrange(1,nBins+1):
      p = htmp.GetBinContent(iB)/integral
      if p>0:
        # p<0 if weights?
        entropy -= p*log2(p)
    return entropy,htmp
  def __H2(self,AVar,nABins,ALow,AHigh,BVar,nBBins,BLow,BHigh,cut=""):
    usingCached = False
    if self.cachedABT: # this is only used for MITruth
      htmp = TH2D("h2","h2",nABins,ALow,AHigh,nBBins,BLow,BHigh)
      for iB in xrange(1,nABins+1):
        for jB in xrange(1,nBBins+1):
          htmp.SetBinContent(iB,jB,self.cachedABT.GetBinContent(iB,jB,1)+self.cachedABT.GetBinContent(iB,jB,2))
    if not usingCached:
      print "cut: |%s|"%(cut)
      print "varexp: |%s|"%("%s:%s>>htmpAB(%i,%f,%f,%i,%f,%f)"%(BVar,AVar,nABins,ALow,AHigh,nBBins,BLow,BHigh))
      self.t.Draw("%s:%s>>htmpAB(%i,%f,%f,%i,%f,%f)"%(BVar,AVar,nABins,ALow,AHigh,nBBins,BLow,BHigh),cut,"colz")
      htmp = gPad.GetPrimitive('htmpAB')
    entropy=0
    integral = 1.*htmp.Integral()
    for iB in xrange(1,nABins+1):
      for jB in xrange(1,nBBins+1):
        p = htmp.GetBinContent(iB,jB)/integral
        if p>0:
          entropy -= p*log2(p)
    return entropy,htmp
  def __H3(self,AVar,nABins,ALow,AHigh,BVar,nBBins,BLow,BHigh,CVar,nCBins,CLow,CHigh,cut=""):
    print "cut: |%s|"%(cut)
    print "varexp: |%s|"%("%s:%s:%s>>htmpABC(%i,%f,%f,%i,%f,%f,%i,%f,%f)"%(CVar,BVar,AVar,nABins,ALow,AHigh,nBBins,BLow,BHigh,nCBins,CLow,CHigh))
    self.t.Draw("%s:%s:%s>>htmpABC(%i,%f,%f,%i,%f,%f,%i,%f,%f)"%(CVar,BVar,AVar,nABins,ALow,AHigh,nBBins,BLow,BHigh,nCBins,CLow,CHigh),"","lego")
#    self.t.Draw("massSoftDrop>>htmpABC","")
    htmp = gPad.GetPrimitive('htmpABC')
    entropy=0
    integral = 1.*htmp.Integral()
    for iB in xrange(1,nABins+1):
      for jB in xrange(1,nBBins+1):
        for kB in xrange(1,nCBins+1):
          p = htmp.GetBinContent(iB,jB,kB)/integral
          if p>0:
            entropy -= p*log2(p)
    return entropy,htmp
  def MI(self,AVar,nABins,ALow,AHigh,BVar,nBBins,BLow,BHigh,cut=""):
    self.Aname = AVar
    self.Bname = BVar
    self.cachedABT=None
    hAB,histAB = self.__H2(AVar,nABins,ALow,AHigh,BVar,nBBins,BLow,BHigh,cut)
    self.cachedAB=histAB
    hA,histA = self.__H1(AVar,nABins,ALow,AHigh,cut)
    hB,histB = self.__H1(BVar,nBBins,BLow,BHigh,cut)
    self.cachedAB=None
    histA.Delete()
#    histB.Delete()
    histAB.Delete()
    print hA,hB,hAB
    return hA+hB-hAB
  def MITruth(self,AVar,nABins,ALow,AHigh,BVar,nBBins,BLow,BHigh,cut="",truthName="truth"):
    hABT,histABT = self.__H3(AVar,nABins,ALow,AHigh,BVar,nBBins,BLow,BHigh,truthName,2,0,1,cut)
    self.cachedABT=histABT
    hAB,histAB = self.__H2(AVar,nABins,ALow,AHigh,BVar,nBBins,BLow,BHigh,cut)
    hT,histT = self.__H1(truthName,2,0,1,cut)
    self.cachedABT=None
    histT.Delete()
    histAB.Delete()
    histABT.Delete()
    return hT+hAB-hABT

class Plotter(object):
  counter=0
  def __init__(self,sigFilePath,bgFilePath=None,treeName="jets"):
      self.sigFilePath = sigFilePath
      self.bgFilePath = bgFilePath
      self.treeName=treeName
      self.friends = None
      self.customBins=False
  def AddFriend(self,f):
    if self.friends == None:
      self.friends = [f]
    else:
      self.friends.append(f)
  def makeMIPlot(self,varList,doTruth=True):
    self.chain = TChain(self.treeName)
    self.chain.AddFile(self.bgFilePath)
    self.chain.AddFile(self.sigFilePath)
    fSig = TFile(self.sigFilePath)
    fBg = TFile(self.bgFilePath)
    tSig = fSig.Get(self.treeName)
    tBg = fBg.Get(self.treeName)
    self.fBuffer = TFile('/tmp/buffer.root','RECREATE')
    self.truthtree = TTree('truthtree','truthtree')
    truth = arr('f',[1])
    self.truthtree.Branch('truth',truth,'truth/F')
    nEntries = tSig.GetEntries()
    for iE in xrange(nEntries):
      self.truthtree.Fill()
    truth = arr('f',[0])
    nEntries = tBg.GetEntries()
    for iE in xrange(nEntries):
      self.truthtree.Fill()
    self.chain.AddFriend(self.truthtree)
    mi = MutualInformer(self.chain)
    nVars = len(varList)
    hplot = TH2D('mi'+str(Plotter.counter),'mi'+str(Plotter.counter),nVars,0,nVars,nVars,0,nVars)
    Plotter.counter+=1
    for iV in xrange(nVars):
      for jV in xrange(iV):
        varA = varList[iV]
        varB = varList[jV]
        if doTruth:
          val=mi.MITruth(varA[0],varA[1],varA[2],varA[3],varB[0],varB[1],varB[2],varB[3])
        else:
          val=mi.MI(varA[0],varA[1],varA[2],varA[3],varB[0],varB[1],varB[2],varB[3])
        print iV+1,jV+1,val
        hplot.SetBinContent(jV+1,iV+1,val)
        hplot.SetBinContent(iV+1,jV+1,val)
    return hplot
  def makeCorrelationPlot(self,varList1,varList2,cut=""):
    fIn = TFile(self.sigFilePath)
    t = fIn.Get(self.treeName)
    if self.friends is not None:
      for f in self.friends:
        tFriend = fIn.Get(f)
        t.AddFriend(tFriend)
    gROOT.cd()
    c = Correlator(t)
    nVars1 = len(varList1)
    isSymmetric=False
    if varList2==None:
      varList2 = varList1
      isSymmetric=True
    nVars2 = len(varList2)
    hplot = TH2D('cor'+str(Plotter.counter),'cor'+str(Plotter.counter),nVars1,0,nVars1,nVars2,0,nVars2)
    Plotter.counter+=1
    xaxis = hplot.GetXaxis()
    yaxis = hplot.GetYaxis()
    matrix = empty([nVars1,nVars2])
    if isSymmetric:
      for iV in xrange(nVars1):
        if self.customBins:
          xaxis.SetBinLabel(iV+1,varList1[iV][0])
          yaxis.SetBinLabel(iV+1,varList1[iV][0])
        else:
          xaxis.SetBinLabel(iV+1,varList1[iV])
          yaxis.SetBinLabel(iV+1,varList1[iV])
        for jV in xrange(iV+1):
          varA = varList1[iV]
          varB = varList1[jV]
          if self.customBins:
            val=c.pearsonSuperFast(varA[0],varB[0],cut,varList1,varA[1],varB[1])
          else:
            val=c.pearsonSuperFast(varA,varB,cut,varList1)
          hplot.SetBinContent(jV+1,iV+1,val)
          hplot.SetBinContent(iV+1,jV+1,val)
          matrix[iV,jV] = val
          matrix[jV,iV]=val
    else:
      for iV in xrange(nVars1):
        if self.customBins:
          xaxis.SetBinLabel(iV+1,varList1[iV][0])
        else:
          xaxis.SetBinLabel(iV+1,varList1[iV])
        for jV in xrange(nVars2):
          if self.customBins:
            yaxis.SetBinLabel(jV+1,varList2[jV][0])
          else:
            yaxis.SetBinLabel(jV+1,varList2[jV])
          varA = varList1[iV]
          varB = varList2[jV]
          if self.customBins:
            val=c.pearsonSuperFast(varA[0],varB[0],cut,varA[1],varB[1])
          else:
            val=c.pearsonSuperFast(varA,varB,cut,varList1)
          print iV,jV,val
          hplot.SetBinContent(iV+1,jV+1,val)
          matrix[iV,jV] = val
    return hplot,matrix
  def makeCovariancePlot(self,varList1,varList2,cut=""):
    fIn = TFile(self.sigFilePath)
    t = fIn.Get(self.treeName)
    gROOT.cd()
    c = Covariator(t)
    nVars1 = len(varList1)
    isSymmetric=False
    if varList2==None:
      varList2 = varList1
      isSymmetric=True
    nVars2 = len(varList2)
    hplot = TH2D('cov'+str(Plotter.counter),'cov'+str(Plotter.counter),nVars1,0,nVars1,nVars2,0,nVars2)
    Plotter.counter+=1
    xaxis = hplot.GetXaxis()
    yaxis = hplot.GetYaxis()
    matrix = empty([nVars1,nVars2])
    if isSymmetric:
      for iV in xrange(nVars1):
        xaxis.SetBinLabel(iV+1,varList1[iV])
        yaxis.SetBinLabel(iV+1,varList1[iV])
        for jV in xrange(iV+1):
          varA = varList1[iV]
          varB = varList1[jV]
          val=c.covSuperFast(varA,varB,cut)
          hplot.SetBinContent(jV+1,iV+1,val)
          hplot.SetBinContent(iV+1,jV+1,val)
          matrix[iV,jV] = val
          matrix[jV,iV]=val
    else:
      for iV in xrange(nVars1):
        xaxis.SetBinLabel(iV+1,varList1[iV])
        for jV in xrange(nVars2):
          yaxis.SetBinLabel(jV+1,varList2[jV])
          varA = varList1[iV]
          varB = varList2[jV]
          val=c.covSuperFast(varA,varB,cut)
          hplot.SetBinContent(iV+1,jV+1,val)
          matrix[iV,jV] = val
    return hplot,matrix
