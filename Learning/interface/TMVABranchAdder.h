#ifndef PANDACORE_LEARNING_TMVABranchAdder
#define PANDACORE_LEARNING_TMVABranchAdder

#include "TFile.h"
#include "TTree.h"
#include "TCut.h"
#include "TString.h"
#include "TTreeFormula.h"
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"
#include "vector"
#include "PandaCore/Tools/interface/Common.h"

class TMVABranchAdder
{
public:
  class xvar {
    public:
      xvar(TString n_, TString f_) { name=n_; formula=f_; val = new float(0); }
      ~xvar() { delete val;  }
      TString name;
      TString formula;
      float *val;
  };

  TMVABranchAdder() { reader = new TMVA::Reader("!Silent"); } 
  ~TMVABranchAdder() { 
    delete reader; 
    for (auto *x : variables)
      delete x;
    for (auto *x : formulae)
      delete x;
  }

  void AddVariable(TString vname, TString formula);
  void AddFormula(TString vname, TString formula);
  void AddSpectator(TString vname);
  void BookMVA(TString branchname,TString fpath);
  void RunFile(TString fpath);

  TString treename="";
private:
  TMVA::Reader *reader=0;
  std::vector<xvar*> variables;
  std::vector<xvar*> formulae; 
  std::vector<TString> bnames;
  float spectator=0; // this is stupid
  TFile *fin;
  TTree *tin;
};
#endif
