#!/usr/bin/env python

import ROOT as root
from PandaCore.Tools.Misc import *
from PandaCore.Tools.Load import *
import re

Load('Statistics','RooExpErf')

def treeToDS(tree,rvars,cut=None,weight=None):
  args = ['data','data',root.RooArgSet(*rvars),root.RooFit.Import(tree)]
  if cut:
    args.append(root.RooFit.Cut(cut))
  if weight:
    args.append(root.RooFit.WeightVar(weight))
  ds = root.RooDataSet(*args)
  return ds

def treeToKDE(tree,rvars,cut=None,weight=None,bw=1):
  ds = treeToDS(tree,rvars,cut,weight)
  kde = root.RooKeysPdf('kde','kde',rvars[0],ds,root.RooKeysPdf.MirrorBoth,bw)
  return kde

def factoryExtractName(s):
  return s.split('::')[1].split('(')[0]

def factoryExtractParams(s):
  matches = re.findall("[A-z]+\[[\-0-9\.,]+\]",s)
  params = [x.split('[')[0] for x in matches]
  return params

class CategoryManager():
  # manages the fit of a single category
  def __init__(self,name,w=None):
    self.name = name
    self.w = w if w else root.RooWorkspace(name,name)
    self.w.imp = getattr(self.w,'import')
    self.params = {} # maybe useful; safeguard against GC
    self.pdfs = {}
    self.data = None # must be a RooDataHist or RooDataSet prior to fitting

  def __getParam__(self,mname,params,pname,nNorms=0):
    # private function to extract parameters
    if pname in params:
      px = params[pname]
    else:
      if nNorms:
        # if nNorms>0, this is one of nNorms normalization parameters
        # assume normalization is to 1
        px = (1./nNorms,0,1)
      else:
        PError("RooFitUtils.CategoryManager.buildModel",
                "Parameter %s missing in model %s"%(pname,mname))
        return None
    if type(px)==tuple:
      newvar = root.RooRealVar(mname+'_'+pname,mname+'_'+pname,px[0],px[1],px[2]) 
      self.w.imp(newvar)
      self.params[mname+'_'+pname] = newvar
      return newvar
    else:
      # tricky to do type-checking here...assume user is not idiot
      self.params[mname+'_'+pname] = px
      return px

  def addModels(self,mname,model_keys,norm_keys):
    # adds pdfs and creates a new pdf
    # model_keys (norm_keys) is a list where the entries are:
    #   1) strings which are keys in self.pdfs (self.params)
    #   2) or RooAbsPdfs (RooAbsReals)
    
    models = []
    for k in model_keys:
      if type(k)==str:
        models.append(self.pdfs[k])
      else:
        models.append(k)
        self.pdfs[k.GetName()]=k
        self.w.imp(k)
    norms = []
    for k in norm_keys:
      if type(k)==str:
        norms.append(self.params[k])
      elif type(k)==tuple:
        i_n = len(norms)
        nname = mname+'_norm_%i'%i_n
        norm = root.RooRealVar(nname,nname,k[0],k[1],k[2])
        self.params[nname] = norm
        self.w.imp(norm)
        norms.append(norm)
      else:
        norms.append(k)
        self.params[k.GetName()] = k
        self.w.imp(k)

    addmodel = root.RooAddPdf(mname,mname,  
                              root.RooArgList(*models),
                              root.RooArgList(*norms))
    self.w.imp(addmodel)
    self.pdfs[mname] = addmodel
    return addmodel

  def smearModel(self,mname,x,base,smear,params={}):
    # smears a model using some predefined smearings or a custom one
    # mname is the name of the output model. if empty, '_smeared' is appended
    # x is the RooRealVar which is the dependence
    # base and smear are either:
    #   1) a key in self.pdfs (base)
    #   2) a string corresponding to a pre-defined smearing option (smear)
    #   3) externally-defined RooAbsPdf (base) or RooAbsReal(smear)
    # if (2) is used, parameters must be specified in params
    def getParam(pname,nNorms=0):
      return self.__getParam__(mname,params,pname,nNorms)

    if type(base)==str:
      base_model = self.pdfs[base]
      if mname=='':
        mname = base+'_smeared'
    else:
      base_model = base
      self.pdfs[base.GetName()] = base
      self.w.imp(base)
      if mname=='':
        mname = base.GetName()+'_smeared'

    if type(smear)==str:
      smear_model=None
      if smear.upper()=='GAUSS':
        mu = getParam('mu')
        sigma = getParam('sigma')
        smear_model = root.RooGaussian(mname+'_gauss',mname+'_gauss',x,mu,sigma)
        self.pdfs[mname+'_gauss'] = smear_model
      elif  smear.upper()=='CB':
        mu = getParam('mu')
        sigma = getParam('sigma')
        alpha = getParam('alpha')
        n = getParam('n_cb')
        smear_model = root.RooCBShape(mname+'_cb',mname+'_cb',x,mu,sigma,alpha,n)
        self.pdfs[mname+'_cb'] = smear_model
      else:
        PError('RooFitUtils.CategoryManager.buildModel',
                'Smearing %s is not pre-defined'%(smear.upper()))
        return None
    else:
      smear_model = smear

    smeared = root.RooFFTConvPdf(mname,mname,x,smear_model,base_model)
    # smeared = root.RooFFTConvPdf(mname,mname,x,base_model,smear_model)

    self.pdfs[mname] = smeared
    self.w.imp(smeared)
    return smeared

  def buildModel(self,mname,x,opt,params={}):
    # builds some pre-defined commonly-used models
    # mname is the name for this model. used to save to the workspace
    # x is a RooRealVar
    # opt is a string specifying pre-defined options for the model
    # shared is a dict of paramater names 
    #   the values can either be a tuple (init,lo,hi)
    #   or a pre-defined RooAbsReal

    def getParam(pname,nNorms=0):
      return self.__getParam__(mname,params,pname,nNorms)

    model = None
    if opt.upper()=='GAUSS':
      mu = getParam('mu')
      sigma = getParam('sigma')
      model = root.RooGaussian(mname,mname,x,mu,sigma)
      self.pdfs[mname] = model
    elif opt.upper()=='BW':
      mu = getParam('mu')
      sigma = getParam('sigma')
      model = root.RooBreitWigner(mname,mname,x,mu,sigma)
      self.pdfs[mname] = model
    elif  opt.upper()=='CB':
      mu = getParam('mu')
      sigma = getParam('sigma')
      alpha = getParam('alpha')
      n = getParam('n_cb')
      model = root.RooCBShape(mname,mname,x,mu,sigma,alpha,n)
      self.pdfs[mname] = model
    elif opt.upper()=='EXPERF':
      a = getParam('a')
      b = getParam('b')
      c = getParam('c')
      model = root.RooExpErf(mname,mname,x,a,b,c)
      self.pdfs[mname] = model
    elif opt.upper()=='BERN':
      n = params['n']
      coefs = []
      for iN in xrange(n):
        coefs.append( getParam('c_%i'%iN) ) 
      model = root.RooBernstein(mname,mname,x,root.RooArgList(*coefs))
      self.pdfs[mname] = model
    elif opt.upper()=='CBBERN':
      mu = getParam('mu')
      sigma = getParam('sigma')
      alpha = getParam('alpha')
      n_cb = getParam('n_cb')
      n = params['n']
      coefs = []
      for iN in xrange(n):
        coefs.append( getParam('c_%i'%iN) ) 
      mu    = getParam('mu')
      sigma = getParam('sigma')
      ncb   = getParam('norm_cb',2)
      nb    = getParam('norm_bern',2)
      cb = root.RooCBShape(mname,mname,x,mu,sigma,alpha,n_cb)
      if n>0:
        bern = root.RooBernstein(mname+'_bern',mname+'_bern',x,root.RooArgList(*coefs))
        model = root.RooAddPdf(mname,mname,
                                root.RooArgList(cb,bern),
                                root.RooArgList(ncb,nb))
        self.pdfs[mname+'_bern'] = bern
      else:
        model = cb
      self.pdfs[mname+'_cb'] = cb
      self.pdfs[mname] = model
    elif opt.upper()=='BWBERN':
      n = params['n']
      coefs = []
      for iN in xrange(n):
        coefs.append( getParam('c_%i'%iN) ) 
      mu    = getParam('mu')
      sigma = getParam('sigma')
      nbw   = getParam('norm_bw',2)
      nb    = getParam('norm_bern',2)
      bw = root.RooBreitWigner(mname,mname,x,mu,sigma)
      if n>0:
        bern = root.RooBernstein(mname+'_bern',mname+'_bern',x,root.RooArgList(*coefs))
        model = root.RooAddPdf(mname,mname,
                                root.RooArgList(bw,bern),
                                root.RooArgList(nbw,nb))
        self.pdfs[mname+'_bern'] = bern
      else:
        model = bw
      self.pdfs[mname+'_bw'] = bw
      self.pdfs[mname] = model
    elif opt.upper()=='GAUSSBERN':
      n = params['n']
      coefs = []
      for iN in xrange(n):
        coefs.append( getParam('c_%i'%iN) ) 
      mu    = getParam('mu')
      sigma = getParam('sigma')
      ng    = getParam('norm_gauss',2)
      nb    = getParam('norm_bern',2)
      bern = root.RooBernstein(mname+'_bern',mname+'_bern',x,root.RooArgList(*coefs))
      gauss  = root.RooGaussian(mname+'_gauss',mname+'_gauss',x,mu,sigma)
      model = root.RooAddPdf(mname,mname,
                              root.RooArgList(gauss,bern),
                              root.RooArgList(ng,nb))
      self.pdfs[mname+'_bern'] = bern
      self.pdfs[mname+'_gauss'] = gauss
      self.pdfs[mname] = model
    elif opt.upper()=='WGAUSSEXPERF':
      w_mu    = getParam('w_mu')
      w_sigma = getParam('w_sigma')
      mu    = getParam('mu')
      sigma = getParam('sigma')
      a     = getParam('a')
      b     = getParam('b')
      c     = getParam('c')
      ng    = getParam('norm_gauss',3)
      ne    = getParam('norm_experf',3)
      nbw    = getParam('norm_bw',3)
      gauss  = root.RooGaussian(mname+'_gauss',mname+'_gauss',x,mu,sigma)
      bw = root.RooBreitWigner(mname+'_bw',mname+'_bw',x,w_mu,w_sigma)
      experf = root.RooExpErf(mname+'_experf',mname+'_experf',x,a,b,c)
      model  = root.RooAddPdf(mname,mname,
                              root.RooArgList(gauss,experf,bw),
                              root.RooArgList(ng,ne,nbw))
      self.pdfs[mname+'_bw'] = bw
      self.pdfs[mname+'_gauss'] = gauss
      self.pdfs[mname+'_experf'] = experf
      self.pdfs[mname] = model
    elif opt.upper()=='GAUSSEXPERF':
      mu    = getParam('mu')
      sigma = getParam('sigma')
      a     = getParam('a')
      b     = getParam('b')
      c     = getParam('c')
      ng    = getParam('norm_gauss',2)
      ne    = getParam('norm_experf',2)
      gauss  = root.RooGaussian(mname+'_gauss',mname+'_gauss',x,mu,sigma)
      # formula = '@1*TMath::Gaus(@0,@3,@4)+@2*TMath::Exp(@7*@0)*(1+TMath::Erf((@0-@5)/@6))/2'
      # formula = 'TMath::Exp(@3*@0)*(1+TMath::Erf((@0-@1)/@2))/2'
      # model = root.RooGenericPdf(mname,mname,formula,
      #                             root.RooArgList(x,ng,ne,mu,sigma,a,b,c))
      # experfstring = 'exp(@3*@0)'
      # experfstring = 'exp(@3*@0)*(1+TMath::Erf((@0-@1)*@2))/2'
      # experf = root.RooGenericPdf(mname+"_experf",mname+"_experf",
      #                             formula,root.RooArgList(x,a,b,c))
      experf = root.RooExpErf(mname+'_experf',mname+'_experf',x,a,b,c)
      model  = root.RooAddPdf(mname,mname,
                              root.RooArgList(gauss,experf),
                              root.RooArgList(ng,ne))
      self.pdfs[mname+'_gauss'] = gauss
      self.pdfs[mname+'_experf'] = experf
      self.pdfs[mname] = model
    elif opt.upper()=='CBEXPERF':
      mu    = getParam('mu')
      sigma = getParam('sigma')
      a     = getParam('a')
      b     = getParam('b')
      c     = getParam('c')
      alpha = getParam('alpha')
      n = getParam('n_cb')
      ncb   = getParam('norm_cb',2)
      ne    = getParam('norm_experf',2)
      cb  = root.RooCBShape(mname+'_cb',mname+'_cb',x,mu,sigma,alpha,n)
      experf = root.RooExpErf(mname+'_experf',mname+'_experf',x,a,b,c)
      model  = root.RooAddPdf(mname,mname,
                              root.RooArgList(cb,experf),
                              root.RooArgList(ncb,ne))
      self.pdfs[mname+'_cb'] = cb
      self.pdfs[mname+'_experf'] = experf
      self.pdfs[mname] = model
    if not model:
      PError('RooFitUtils.CategoryManager.buildModel',
              'Model %s is not pre-defined'%(opt.upper()))
      return None

    self.w.imp(model)
    return model