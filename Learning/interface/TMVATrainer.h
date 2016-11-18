#ifndef PANDACORE_LEARNING_TMVATrainer
#define PANDACORE_LEARNING_TMVATrainer

#include "TFile.h"
#include "TTree.h"
#include "TCut.h"
#include "TString.h"
#include "TTreeFormula.h"
#include "TMVA/Tools.h"
#include "TMVA/Factory.h"
#include "vector"
#include "PandaCore/Tools/interface/Common.h"
#include "PandaCore/Tools/interface/TreeTools.h"

class TMVATrainer
{
public:
  enum BDTType {
    kAda,
    kGradWide,
    kGradDeep
  };
  TMVATrainer(TString name, TString workdir_); 
  ~TMVATrainer();

  void AddVariable(TString v, char t, TString title="");
  void AddSpectator(TString v, char t, TString title="");
  void SetFiles(TString sigpath, TString bgpath);
  void BookBDT(BDTType t);
  void BookBDT(TString opt="");
  void TrainAll();

  TString treename="";
  TString sigcut="", bgcut="";
  TString sigweight="", bgweight="";
private:
  TMVA::Factory *factory=0;
  TFile *sigfile=0, *bgfile=0;
  TTree *sigtree=0, *bgtree=0;
  TFile *outfile;
  TString workdir;
   
};
#endif
