#include "TFile.h"
#include "TTree.h"
#include "TMVA/Tools.h"
#include "TMVA/Factory.h"
#include "TCut.h"
#include "vector"

void trainBDT(TString workDir) {
  TFile *fSig = new TFile(workDir+"/signal.root");
  TFile *fBg = new TFile(workDir+"/qcd.root");

  TFile *fOut = new TFile(workDir+"/bdt.root","RECREATE");
  TString name = "TMVA";
  TCut extracut;
  if (workDir.Contains("CA15")) {
    name += "_CA15";
    extracut = TCut("massSoftDrop>150&&massSoftDrop<240&&pt<470&&TMath::Abs(eta)<2.4");
  } else {
    name += "_AK8";
    extracut = TCut("massSoftDrop>110&&massSoftDrop<210&&pt>470&&TMath::Abs(eta)<2.1");
  }

  TMVA::Factory *factory = new TMVA::Factory(name.Data(), fOut,
                                                        "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );
  TTree *sig = (TTree*)fSig->Get("jets");
  TTree *bg  = (TTree*)fBg->Get("jets");

  TFile *fCuts = new TFile(workDir + "/cuts.root","RECREATE");
  std::vector<const char*> variables;
/*
  variables.push_back("logchi");
  variables.push_back("tau32 := tau3/tau2");
  variables.push_back("QjetVol");
  variables.push_back("QGTag");
  variables.push_back("groomedIso");
  variables.push_back("sjqgtag0");
  variables.push_back("sjqgtag1");
  variables.push_back("sjqgtag2");
*/
  variables.push_back("dR_sj0dR");
  variables.push_back("dR_sj0mW");
  variables.push_back("dR_sj0dRTRF");
  variables.push_back("dR_sj0dPhiTRF");
  variables.push_back("dR_sj1dR");
  variables.push_back("dR_sj1mW");
  variables.push_back("dR_sj1dRTRF");
  variables.push_back("dR_sj1dPhiTRF");
  variables.push_back("dR_sj2dR");
  variables.push_back("dR_sj2mW");
  variables.push_back("dR_sj2dRTRF");
  variables.push_back("dR_sj2dPhiTRF");
  variables.push_back("dRTRF_sj0dR");
  variables.push_back("dRTRF_sj0mW");
  variables.push_back("dRTRF_sj0dRTRF");
  variables.push_back("dRTRF_sj0dPhiTRF");
  variables.push_back("dRTRF_sj1dR");
  variables.push_back("dRTRF_sj1mW");
  variables.push_back("dRTRF_sj1dRTRF");
  variables.push_back("dRTRF_sj1dPhiTRF");
  variables.push_back("dRTRF_sj2dR");
  variables.push_back("dRTRF_sj2mW");
  variables.push_back("dRTRF_sj2dRTRF");
  variables.push_back("dRTRF_sj2dPhiTRF");
  variables.push_back("sumQG_sj0dR");
  variables.push_back("sumQG_sj0mW");
  variables.push_back("sumQG_sj0dRTRF");
  variables.push_back("sumQG_sj0dPhiTRF");
  variables.push_back("sumQG_sj1dR");
  variables.push_back("sumQG_sj1mW");
  variables.push_back("sumQG_sj1dRTRF");
  variables.push_back("sumQG_sj1dPhiTRF");
  variables.push_back("sumQG_sj2dR");
  variables.push_back("sumQG_sj2mW");
  variables.push_back("sumQG_sj2dRTRF");
  variables.push_back("sumQG_sj2dPhiTRF");

  factory->AddVariable("nSubjets",'I');
  std::vector<TMVA::Factory*>factories;
  factories.push_back(factory);
  int counter=0;
  for (const char *var : variables ) {
//    TMVA::Factory *cutFactory = new TMVA::Factory(TString::Format("cuts_%i",counter).Data(),fCuts,
//                                                          "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );
//    cutFactory->AddVariable(var,'F');
//    cutFactory->BookMethod(TMVA::Types::kCuts,TString::Format("CUT_%i",counter).Data());
    factory->AddVariable(var,'F');
//    factories.push_back(cutFactory);
    ++counter;
  }
  
  for (TMVA::Factory *fact : factories) {
    fact->AddSignalTree(sig,1);
    fact->AddBackgroundTree(bg,1);
    fact->SetBackgroundWeightExpression("mcWeight");
    fact->PrepareTrainingAndTestTree(extracut,extracut,"nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );
  }

  factory->BookMethod(TMVA::Types::kBDT,"BDT","!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=3:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20" );
  
  for (TMVA::Factory *fact : factories) {
    fact->TrainAllMethods();
    fact->TestAllMethods();
    fact->EvaluateAllMethods();
  }
  fCuts->Close();
  fOut->Close();
  for (TMVA::Factory *fact : factories) 
    delete fact;
}
