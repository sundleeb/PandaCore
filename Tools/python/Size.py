#!/usr/bin/env python

import glob
import sys,os
import re
from array import array
import ROOT as root
from subprocess import call,check_output
from Misc import *

colorarray=array('i',[root.kGreen+2, root.kRed+2, root.kBlue, root.kMagenta+2, root.kOrange,
		root.kGreen-7, root.kCyan, root.kRed-7, root.kBlue-7,root.kMagenta,
		root.kYellow, root.kViolet, root.kGray, root.kRed, root.kCyan+2,
		])

class PandaFile:
	def __init__(self,fpath,nickname):
		self.nickname = nickname
		self.fpath = fpath
		self.branches = {}

class SizeChecker:
	def __init__(self):
		self.pfiles = []
	def add_file(self,fpath,nickname):
		self.pfiles.append(PandaFile(fpath,nickname))
	def dump_info(self,makePlot=False,outdir=None):
		for pfile in self.pfiles:
			cmd = '''root -b -l	<<EOF 
			 gSystem->Load("libPandaProdObjects.so");
			 TFile *f = TFile::Open("%s");
			 f->cd("PandaNtupler");
			 events->Print();
			 .q
			EOF''' % (pfile.fpath)
			out= check_output (cmd ,shell=True)
			thisbranch = ''
			thisevents = 0
			thissize = 0
			thiscompression = 99999
			for line in out.split('\n'):
				if '*********' in line : continue
				if '.........' in line : continue
				line = line.replace('*','')
				if line=='': continue
#				PDebug('PandaCore.Tools.Size.SizeChecker',line)
				header = line.split(':')[0]
				if 'Branch' in header or 'BranchElement' in line:
					# composite branch, skip
					pass
				elif 'Br' in header or 'Tree' in header:
					# treat the whole tree like a branch
					if thisbranch!='':
						# flush the previous branch
						pfile.branches[thisbranch] = (thissize,thiscompression,thisevents)
						thisbranch = ''
						thisevents = 0
						thissize = 0
						thiscompression = 99999
					thisbranch = line.split(':')[1].strip()
					splitBranch=False
				elif 'Entries' in header:
					thisevents = int(line.split(':')[1])
					thissize = int( re.sub('bytes.*','',re.sub(' ','',line.split(':')[-1].split('=')[-1])) )
				elif 'Basket' in header or 'Tree compression' in line:
					thiscompression = float( line.split('=')[-1] )
				else:
					pass
			nentries = pfile.branches['events'][2]
			sorted_branches = sorted(pfile.branches, key = lambda x: pfile.branches[x][0])
			output = ''
			output += '--------------------------------------------------------------\n'
			output += 'sample =%s\n'%(pfile.nickname)
			output += '--------------------------------------------------------------\n'
			output += '%-15s |%12s	| compression \n'%('branch name','bytes/event')
			output += '--------------------------------------------------------------\n'
			for b in sorted_branches:
				info = pfile.branches[b]
				if info[0]<nentries:
					continue
				output += '%-15s | %-12.2f | %.3g\n'%(b,info[0]*1./nentries,info[1])
			if not outdir:
				print output
				return
			ftxt = open(outdir+'/'+pfile.nickname+'.txt','w')
			ftxt.write(output)
			ftxt.close()
			if makePlot:
				c = root.TCanvas('c','c',1200,600)
				c.Divide(2,1)
				c.cd(1)
				vals = []
				labels = []
				nvals = min(len(sorted_branches),15)
				sorted_branches.reverse()
				for b in sorted_branches:
					if len(vals)>nvals:
						break
					if b=='events':
						continue
					info = pfile.branches[b]
					if info[0]<nentries:
						continue
					labels.append(b)
					vals.append(info[0]*1./nentries)
				pie = root.TPie('pie','notitle',nvals,array('f',vals),colorarray)
				for iV in range(nvals):
					pie.SetEntryLineColor(iV,root.kBlack)
					pie.SetEntryLineWidth(iV,root.kBlack)
					pie.SetEntryLabel(iV,labels[iV] )
				pie.SetTitle(pfile.nickname)
				pie.SetCircle(0.5,.45,0.3)
				pie.Draw('rsc')
				
				c.cd(2)
				txt = root.TLatex()
				txt.SetNDC()
				txt.SetTextSize(0.03)
				txt.SetTextFont(82)
				output_lines = output.split('\n')
				output_lines.reverse()
				tablesize = min(18,len(output_lines))
				for iL in range(tablesize+2):
					if iL==tablesize+1:
						txt.DrawLatex(0.15,0.01+iL*0.05,'%-15s |%12s	| compression'%('branch name','bytes/event'))
					elif iL==tablesize:
						txt.DrawLatex(0.15,0.01+iL*0.05,'--------------------------------------------------------')
					else:
						txt.DrawLatex(0.15,0.01+iL*0.05,output_lines[iL].strip())
				for ext in ['png','pdf']:
					c.SaveAs(outdir+'/'+pfile.nickname+'.'+ext)

