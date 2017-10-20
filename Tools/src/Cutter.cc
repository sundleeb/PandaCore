#include "../interface/Cutter.h"


TTree *Cutter::CutTree(TTree *t,TString cut) {
  PInfo("PandaCore::Cutter::CutTree",
      TString::Format("accepting %i/%i entries\n",(int)t->GetEntries(cut),(int)t->GetEntries()));
  return (TTree*)t->CopyTree(cut);
}

void Cutter::Cut(TString finpath, TString foutpath, TString cut) {
  TFile *fin = TFile::Open(finpath);
  TTree *t = (TTree*)fin->Get(treeName.Data());
  TFile *fout = TFile::Open(foutpath,"RECREATE");
  TTree *tout = CutTree(t,cut);
  fout->WriteTObject(tout,treeName);
}
