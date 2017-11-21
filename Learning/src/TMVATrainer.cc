#include "../interface/TMVATrainer.h"
#include "TMVA/Config.h"

TMVATrainer::TMVATrainer(TString name, TString workdir_):
  workdir(workdir_)
{
  TMVA::gConfig().GetIONames().fWeightFileDir = workdir.Data();
  outfile = new TFile(workdir+"/tmva_"+name+".root","RECREATE");
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
  dataloader = new TMVA::DataLoader(name.Data());
#endif
  factory = new TMVA::Factory(name.Data(), outfile,
      //"!V:!Silent:Color:DrawProgressBar:Transformations=I:AnalysisType=Classification");
      "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification");
  dataloader->AddSpectator("eventNumber",'l');
  dataloader->AddSpectator("runNumber",'I');
}

TMVATrainer::~TMVATrainer() {
  outfile->Close();
  sigfile->Close(); bgfile->Close();
  delete factory;
}

void TMVATrainer::AddVariable(TString v, char t, TString title) {
  if(title=="") 
    dataloader->AddVariable(v,t);
  else
    dataloader->AddVariable(v,title,"",t);
}

void TMVATrainer::AddSpectator(TString v, char t, TString title) {
  if(title=="") 
    dataloader->AddSpectator(v,t);
  else
    dataloader->AddSpectator(v,title,"",t);
}

void TMVATrainer::SetFiles(TString sigpath, TString bgpath) {
  sigfile = TFile::Open(sigpath);
  bgfile  = TFile::Open(bgpath);
  sigtree = (TTree*)sigfile->Get(treename);
  bgtree  = (TTree*)bgfile->Get(treename);

  dataloader->AddSignalTree(sigtree,1);
  dataloader->AddBackgroundTree(bgtree,1);

  dataloader->SetSignalWeightExpression(sigweight);
  dataloader->SetBackgroundWeightExpression(bgweight);
  
  dataloader->PrepareTrainingAndTestTree(sigcut.Data(),bgcut.Data(),"nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V");
}

void TMVATrainer::BookBDT(BDTType t) {
  switch (t) {
    case kAda:
      BookBDT("!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=5:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=10");
      break;
    case kGradWide:
      BookBDT("!H:!V:NTrees=300:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.30:nCuts=20:MaxDepth=5");
      break;
    case kGradDeep:
      BookBDT("!H:!V:NTrees=30:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.10:nCuts=20:MaxDepth=10");
      break;
    default:
      BookBDT();
  }
}

void TMVATrainer::BookBDT(TString opt) {
  if (opt=="") {
    opt = "!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=5:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=10";
  }

  PInfo("TMVATrainer::BookBDT",TString::Format("using options %s",opt.Data()));
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
  factory->BookMethod(dataloader,TMVA::Types::kBDT,"BDT",opt);
#else
  factory->BookMethod(TMVA::Types::kBDT,"BDT",opt);
#endif
}

void TMVATrainer::BookCuts(TString opt) {
  if (opt=="") {
    opt = "!H:!V:EffMethod=EffSel:FitMethod=GA";
  }

  PInfo("TMVATrainer::BookCuts",TString::Format("using options %s",opt.Data()));
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
  factory->BookMethod(dataloader,TMVA::Types::kCuts,"Cuts",opt);
#else
  factory->BookMethod(TMVA::Types::kCuts,"Cuts",opt);
#endif
}

void TMVATrainer::TrainAll() {
  factory->TrainAllMethods();
  factory->TestAllMethods();
  factory->EvaluateAllMethods();
}

