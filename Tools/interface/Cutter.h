#ifndef PANDACORE_TOOLS_Cutter
#define PANDACORE_TOOLS_Cutter

#include "TTree.h"
#include "TFile.h"
#include "TBranch.h"
#include "TString.h"
#include "TH1F.h"
#include "Common.h"

class Cutter
{
public:
  Cutter() { }
  ~Cutter() { }

  TTree *CutTree(TTree *t,TString cut) {
    PInfo("PandaCore::Cutter::CutTree",TString::Format("accepting %i/%i entries\n",(int)t->GetEntries(cut),(int)t->GetEntries()));
    return (TTree*)t->CopyTree(cut);
  }
  void Cut(TString finpath, TString foutpath, TString cut) {
    TFile *fin = TFile::Open(finpath);
    TTree *t = (TTree*)fin->Get(treeName.Data());
    TFile *fout = TFile::Open(foutpath,"RECREATE");
    TTree *tout = CutTree(t,cut);
    fout->WriteTObject(tout,treeName);
  }

  TString treeName = "events";
};
#endif
