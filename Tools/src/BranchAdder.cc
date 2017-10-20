#include "../interface/BranchAdder.h"


void BranchAdder::AddBranchFromFormula(TTree *t) 
{
  float newBranchVal=0;
  
  t->SetBranchStatus("*",0);
  turnOnBranches(t,formula);
  turnOnBranches(t,cut);

  TBranch *b = t->Branch(newBranchName.Data(),&newBranchVal,TString::Format("%s/F",newBranchName.Data()));

  TTreeFormula tformula(newBranchName.Data(),formula.Data(),t);
  tformula.SetQuickLoad(true);

  TTreeFormula tcut("cut",cut.Data(),t);
  tcut.SetQuickLoad(true);

  unsigned int nEntries = t->GetEntries();
  unsigned int iE=0;
  ProgressReporter pr("PandaCore::BranchAdder",&iE,&nEntries,10);
  for (iE=0; iE!=nEntries; ++iE) {
    if (verbose)
      pr.Report();
    t->GetEntry(iE);
    if (tcut.EvalInstance()) {
      newBranchVal = tformula.EvalInstance();
    } else {
      newBranchVal = defaultValue;
    }
    b->Fill();
  }
  t->SetBranchStatus("*",1);
}

void BranchAdder::AddBranchFromFormula(TString fpath) 
{
  TFile *fIn = TFile::Open(fpath,"UPDATE");
  TTree *t = (TTree*)fIn->Get(treeName.Data());
  if (t==NULL) {
    PError("MitPanda::Tools::AddBranch",TString::Format("Could not normalize %s because tree=%p and\n",fpath.Data(),t));
    return; 
  }
  AddBranchFromFormula(t);
  fIn->WriteTObject(t,treeName,"Overwrite");
  fIn->Close();
}

void BranchAdder::AddBranchFromHistogram(TTree *t, TH1 *h) 
{
  TH1D hd;
  h->Copy(hd);
  float newBranchVal=0;
  
  t->SetBranchStatus("*",0);
  turnOnBranches(t,formula);
  turnOnBranches(t,cut);

  TBranch *b = t->Branch(newBranchName.Data(),&newBranchVal,TString::Format("%s/F",newBranchName.Data()));

  TTreeFormula tformula(newBranchName.Data(),formula.Data(),t);
  tformula.SetQuickLoad(true);

  TTreeFormula tcut("cut",cut.Data(),t);
  tcut.SetQuickLoad(true);

  double lo = hd.GetBinCenter(1);
  double hi = hd.GetBinCenter(hd.GetNbinsX());

  unsigned int nEntries = t->GetEntries();
  unsigned int iE=0;
  ProgressReporter pr("PandaCore::BranchAdder",&iE,&nEntries,10);
  for (iE=0; iE!=nEntries; ++iE) {
    if (verbose)
      pr.Report();
    t->GetEntry(iE);
    if (tcut.EvalInstance()) {
      double xval = tformula.EvalInstance();
      xval = bound(xval,lo,hi);
      newBranchVal = hd.GetBinContent(hd.FindBin(xval));
    } else {
      newBranchVal = defaultValue;
    }
    b->Fill();
  }
  t->SetBranchStatus("*",1);
}

void BranchAdder::AddBranchFromHistogram(TString fpath, TH1 *h) 
{
  TFile *fIn = TFile::Open(fpath,"UPDATE");
  TTree *t = (TTree*)fIn->Get(treeName.Data());
  if (t==NULL) {
    PError("MitPanda::Tools::AddBranch",TString::Format("Could not normalize %s because tree=%p and\n",fpath.Data(),t));
    return; 
  }
  AddBranchFromHistogram(t,h);
  fIn->WriteTObject(t,treeName,"Overwrite");
  fIn->Close();
}

void BranchAdder::AddBranchFromHistogram2D(TTree *t, TH2 *h) 
{
  TH2D hd;
  h->Copy(hd);
  float newBranchVal=0;
  
  t->SetBranchStatus("*",0);
  turnOnBranches(t,formula);
  turnOnBranches(t,formulaY);
  turnOnBranches(t,cut);

  TBranch *b = t->Branch(newBranchName.Data(),&newBranchVal,TString::Format("%s/F",newBranchName.Data()));

  TTreeFormula tformulaX(formula.Data(),formula.Data(),t);
  TTreeFormula tformulaY(formulaY.Data(),formulaY.Data(),t);
  tformulaX.SetQuickLoad(true);
  tformulaY.SetQuickLoad(true);

  TTreeFormula tcut("cut",cut.Data(),t);
  tcut.SetQuickLoad(true);

  double loX = hd.GetXaxis()->GetBinCenter(1);
  double hiX = hd.GetXaxis()->GetBinCenter(hd.GetNbinsX());
  double loY = hd.GetYaxis()->GetBinCenter(1);
  double hiY = hd.GetYaxis()->GetBinCenter(hd.GetNbinsY());

  unsigned int nEntries = t->GetEntries();
  unsigned int iE=0;
  ProgressReporter pr("PandaCore::BranchAdder",&iE,&nEntries,10);
  for (iE=0; iE!=nEntries; ++iE) {
    if (verbose)
      pr.Report();
    t->GetEntry(iE);
    if (tcut.EvalInstance()) {
      double xval = tformulaX.EvalInstance();
      xval = bound(xval,loX,hiX);
      double yval = tformulaY.EvalInstance();
      yval = bound(yval,loY,hiY);
      newBranchVal = hd.GetBinContent(hd.FindBin(xval,yval));
    } else {
      newBranchVal = defaultValue;
    }
    b->Fill();
  }
  t->SetBranchStatus("*",1);
}

void BranchAdder::AddBranchFromHistogram2D(TString fpath, TH2 *h) 
{
  TFile *fIn = TFile::Open(fpath,"UPDATE");
  TTree *t = (TTree*)fIn->Get(treeName.Data());
  if (t==NULL) {
    PError("MitPanda::Tools::AddBranch",TString::Format("Could not normalize %s because tree=%p and\n",fpath.Data(),t));
    return; 
  }
  AddBranchFromHistogram2D(t,h);
  fIn->WriteTObject(t,treeName,"Overwrite");
  fIn->Close();
}
