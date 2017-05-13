#include "../interface/TreeTools.h"

void turnOnBranches(TTree*t,TString s) {
  std::vector<TString> deps = getDependencies(s);
  for (auto d : deps) {
    TBranch *b = (TBranch*)t->GetListOfBranches()->FindObject(d);
    if (b)
      t->SetBranchStatus(d,1);
  }
}

void turnOnBranchesChain(TChain*t,TString s) {
  std::vector<TString> deps = getDependencies(s);
  for (auto d : deps) {
    TBranch *b = (TBranch*)t->GetListOfBranches()->FindObject(d);
    if (b)
      t->SetBranchStatus(d,1);
  }
}