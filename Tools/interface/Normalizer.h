#ifndef PANDACORE_TOOLS_NORMALIZER
#define PANDACORE_TOOLS_NORMALIZER

#include "TTree.h"
#include "TFile.h"
#include "TBranch.h"
#include "TString.h"
#include "TH1F.h"
#include "Common.h"
#include "TreeTools.h"

/**
 * \brief Normalizes a tree
 *
 * Given an input weight, class adds an output weight
 * that is calculated as in_weight*xsec/total_events
 */
class Normalizer
{
public:
  Normalizer() { }
  ~Normalizer() { }

  /**
   * \param t input tree
   * \param totalEvents total number of events for this process
   * \param xsec cross-section for this process
   * \brief Normalizes the tree given total weight of events and 
   * cross-section
   */
  void NormalizeTree(TTree *t, double totalEvts, double xsec) {
    float inWeight=0, outWeight=0;
    t->SetBranchStatus("*",0);
    turnOnBranches(t,inWeightName.Data());
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
    t->SetBranchStatus("*",1);
  }

  /**
   * \param fpath path to input file 
   * \param xsec cross-section for this process
   * \brief Reads an input file and picks up the tree to normalize
   * as well as a histogram containing the weight of events
   */
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

  TString inWeightName = "mcWeight"; /**< name of input branch */
  TString outWeightName = "normalizedWeight"; /**< name of output branch */
  TString histName = "hDTotalMCWeight"; /**< name of histogram containing MC weights */
  TString treeName = "events"; /**< name of input tree */
};
#endif
