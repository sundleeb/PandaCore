#ifndef PANDACORE_TOOLS_BranchAdder
#define PANDACORE_TOOLS_BranchAdder

#include "TTree.h"
#include "TFile.h"
#include "TBranch.h"
#include "TString.h"
#include "TTreeFormula.h"
#include "Common.h"

class BranchAdder
{
public:
  BranchAdder() { }
  ~BranchAdder() { }

  void AddBranchFromFormula(TTree *t) {
    float newBranchVal=0;
    TBranch *b = t->Branch(newBranchName.Data(),&newBranchVal,TString::Format("%s/F",newBranchName.Data()));

    TTreeFormula tformula(newBranchName.Data(),formula.Data(),t);
    tformula.SetQuickLoad(true);

    unsigned int nEntries = t->GetEntries();
    unsigned int iE=0;
    ProgressReporter pr("PandaCore::BranchAdder",&iE,&nEntries,10);
    for (iE=0; iE!=nEntries; ++iE) {
      pr.Report();
      t->GetEntry(iE);
      newBranchVal = tformula.EvalInstance();
      b->Fill();
    }
  }

  void AddBranchFromFormula(TString fpath) {
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

  void AddBranchFromHistogram(TTree *t, TH1F *h) {
    float newBranchVal=0;
    TBranch *b = t->Branch(newBranchName.Data(),&newBranchVal,TString::Format("%s/F",newBranchName.Data()));

    TTreeFormula tformula(newBranchName.Data(),formula.Data(),t);
    tformula.SetQuickLoad(true);

    unsigned int nEntries = t->GetEntries();
    unsigned int iE=0;
    ProgressReporter pr("PandaCore::BranchAdder",&iE,&nEntries,10);
    for (iE=0; iE!=nEntries; ++iE) {
      pr.Report();
      t->GetEntry(iE);
      double xval = tformula.EvalInstance();
      newBranchVal = h->GetBinContent(h->FindBin(xval));
      b->Fill();
    }
  }

  void AddBranchFromHistogram(TString fpath, TH1F *h) {
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

  TString formula = "";
  TString newBranchName = "";
  TString treeName = "events";
};
#endif
