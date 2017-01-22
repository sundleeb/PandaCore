from ROOT import TCanvas,gStyle,TColor,TFile,TTree,TH2F,TEllipse,TObjArray,TGraphPolar,TLine
import ROOT as root
from math import pi,log,cos,sin
from array import array

kJet = root.kOrange
kElectron = root.kBlue
kMuon = root.kRed
kPhoton = root.kGreen
kTau = root.kYellow

class NeroEventDisplay(object):
	def __init__(self):
		maxEta=5
		self.baseEtaphi = TH2F('baseEtaphi',';#eta;#phi',100,-maxEta,maxEta,100,-pi,pi)
		dummy = array('d',[0.])
		self.baseXY = TGraphPolar(1,dummy,dummy,dummy,dummy)
		self.baseEtaphi.SetStats(0)
		self.etaphiParticles = TObjArray()
		self.XYParticles = TObjArray()
		self.c1 = TCanvas('c1','c1',int(200*maxEta),int(200*pi))
		self.c1.SetGrid(1)
		self.c2 = TCanvas('c2','c2',800,800)
		self.c2.SetGrid(1)
		self.jetsName = 'jetpuppi'
		self.doTaus = False
	def setFile(self,f):
		if type(f)==type(''):
			self.file = TFile(f)
		else:
			self.file = f
		self.tree = self.file.FindObjectAny('events')
	def drawEntry(self,entryNumber,saveFig=False):
		self.baseEtaphi.Reset()
		self.etaphiParticles.Clear()
		self.XYParticles.Clear()
		self.tree.GetEntry(entryNumber)
		self.baseEtaphi.SetTitle('Event %i/Run %i'%(self.tree.eventNum,self.tree.runNum))
		self.baseXY.SetTitle('Event %i/Run %i'%(self.tree.eventNum,self.tree.runNum))
		
		# jets
		jetP4s =getattr(self.tree,self.jetsName+'P4') 
		jetSelBits = getattr(self.tree,self.jetsName+'SelBits')
		nJets = jetP4s.GetEntries()
		for iJ in xrange(nJets):
			jetP4 = jetP4s.At(iJ)
			if not(jetSelBits[iJ]&(1<<8)):
				continue
			if jetP4.Pt()<10:
				continue
			jet = TEllipse(jetP4.Eta(),jetP4.Phi(),log(jetP4.Pt())/10)
			jet.SetLineColor(kJet)
			jet.SetLineWidth(2)
			jet.SetFillColor(kJet)
			jet.SetFillStyle(3004)
			self.etaphiParticles.Add(jet)
			pt = jetP4.Pt()
			phi = jetP4.Phi()
			jetxy = TLine(0,0,(cos(phi)*log(pt)/log(2000)),(sin(phi)*log(pt)/log(2000)))
			jetxy.SetLineWidth(2)
			jetxy.SetLineColor(kJet)
			self.XYParticles.Add(jetxy)

		# leptons
		lepP4s =getattr(self.tree,'lepP4') 
		lepSelBits = self.tree.lepSelBits
		nLeps = lepP4s.GetEntries()
		lepPdgIds = self.tree.lepPdgId
		for iL in xrange(nLeps):
			lepP4 = lepP4s.At(iL)
			selBit = lepSelBits[iL]
			if not((selBit&(1<<4)) and (selBit&(1<<12))):
				continue
			if lepP4.Pt()<10:
				continue
			if abs(lepPdgIds[iL])==11: 
				color = kElectron
			else: 
				color = kMuon
			lep = TEllipse(lepP4.Eta(),lepP4.Phi(),log(lepP4.Pt())/10)
			lep.SetLineColor(color)
			lep.SetLineWidth(2)
			lep.SetFillColor(color)
			lep.SetFillStyle(3004)
			self.etaphiParticles.Add(lep)
			pt = lepP4.Pt()
			phi = lepP4.Phi()
			lepxy = TLine(0,0,(cos(phi)*log(pt)/log(2000)),(sin(phi)*log(pt)/log(2000)))
			lepxy.SetLineWidth(2)
			lepxy.SetLineColor(color)
			self.XYParticles.Add(lepxy)

		# photons
		photonP4s =getattr(self.tree,'photonP4') 
		photonSelBits = self.tree.photonSelBits
		nPhotons = photonP4s.GetEntries()
		for iA in xrange(nPhotons):
			selBit = photonSelBits[iA]
			if not(selBit&(1<<3)):
				continue
			photonP4 = photonP4s.At(iA)
			if photonP4.Pt()<10:
				continue
			photon = TEllipse(photonP4.Eta(),photonP4.Phi(),log(photonP4.Pt())/10)
			photon.SetLineColor(kPhoton)
			photon.SetLineWidth(2)
			photon.SetFillColor(kPhoton)
			photon.SetFillStyle(3004)
			self.etaphiParticles.Add(photon)
			pt = photonP4.Pt()
			phi = photonP4.Phi()
			photonxy = TLine(0,0,(cos(phi)*log(pt)/log(2000)),(sin(phi)*log(pt)/log(2000)))
			photonxy.SetLineWidth(2)
			photonxy.SetLineColor(kPhoton)
			self.XYParticles.Add(photonxy)


		# taus
		if self.doTaus:
			tauP4s =getattr(self.tree,'tauP4') 
			tauSelBits = self.tree.tauSelBits
			nTaus = tauP4s.GetEntries()
			for iT in xrange(nTaus):
				selBit = tauSelBits[iT]
				if not(selBit&(1<<2)):
					continue
				tauP4 = tauP4s.At(iT)
				if tauP4.Pt()<10:
					continue
				tau = TEllipse(tauP4.Eta(),tauP4.Phi(),log(tauP4.Pt())/10)
				tau.SetLineColor(kTau)
				tau.SetLineWidth(2)
				tau.SetFillColor(kTau)
				tau.SetFillStyle(3004)
				self.etaphiParticles.Add(tau)
				pt = tauP4.Pt()
				phi = tauP4.Phi()
				tauxy = TLine(0,0,(cos(phi)*log(pt)/log(2000)),(sin(phi)*log(pt)/log(2000)))
				tauxy.SetLineWidth(2)
				tauxy.SetLineColor(kTau)
				self.XYParticles.Add(tauxy)

			
		self.c1.cd()
		self.c1.Clear()
		self.baseEtaphi.Draw()
		for p in self.etaphiParticles:
			p.Draw()

		self.c2.Clear()
		self.c2.cd()
		self.baseXY.Draw('EP')
		self.c2.Update()
		self.baseXY.GetPolargram().SetNdivPolar(0)
		self.baseXY.GetPolargram().SetNdivRadial(0)
		self.baseXY.GetPolargram().SetRadialLabelSize(0)
		self.baseXY.GetPolargram().SetPolarLabelSize(0)
		self.baseXY.GetPolargram().SetToRadian()
		for p in self.XYParticles:
			p.Draw()

		if saveFig:
			self.c1.SaveAs('event_%i_run_%i_etaphi.png'%(self.tree.eventNum,self.tree.runNum))
			self.c1.SaveAs('event_%i_run_%i_etaphi.pdf'%(self.tree.eventNum,self.tree.runNum))
			self.c2.SaveAs('event_%i_run_%i_phi.png'%(self.tree.eventNum,self.tree.runNum))
			self.c2.SaveAs('event_%i_run_%i_phi.pdf'%(self.tree.eventNum,self.tree.runNum))
