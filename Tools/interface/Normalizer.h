#ifndef PANDACORE_TOOLS_NORMALIZER
#define PANDACORE_TOOLS_NORMALIZER

#include "TTree.h"
#include "TFile.h"
#include "TBranch.h"
#include "TString.h"
#include "TH1F.h"
#include "Common.h"

class Normalizer
{
public:
  Normalizer() { }
  ~Normalizer() { }

  void NormalizeTree(TTree *t, double totalEvts, double xsec) {
    float inWeight=0, outWeight=0;
    TBranch *b = t->Branch(outWeightName.Data(),&outWeight,TString::Format("%s/F",outWeightName.Data()));
    t->SetBranchAddress(inWeightName.Data(),&inWeight);
    unsigned int nEntries = t->GetEntries();
    unsigned int iE=0;
    ProgressReporter pr("PandaCoreNormalizer::NormalizeTree",&iE,&nEntries,10);
    for (iE=0; iE!=nEntries; ++iE) {
      pr.Report();
      t->GetEntry(iE);
      outWeight = xsec*inWeight/totalEvts;
      b->Fill();
    }
  }
  void NormalizeTree(TString fpath, double xsec) {
    TFile *fIn = TFile::Open(fpath,"UPDATE");
    TTree *t = (TTree*)fIn->Get(treeName.Data());
    TH1F *h = (TH1F*)fIn->Get(histName.Data());
    if (t==NULL || h==NULL) {
      fprintf(stderr,"Could not normalize %s because tree=%p and hist=%p\n",fpath.Data(),t,h);
      return; 
    }
    NormalizeTree(t,h->Integral(),xsec);
    fIn->WriteTObject(t);
    fIn->Close();
  }

  TString inWeightName = "mcWeight";
  TString outWeightName = "normalizedWeight";
  TString histName = "hDTotalMCWeight";
  TString treeName = "events";
};
#endif
