#include "../interface/DuplicateRemover.h"

void DuplicateRemover::Merge(TTree *t1, TTree *t2, TString fOutPath) 
{
  int run,lumi;
  ULong64_t event;
  t1->SetBranchAddress(runName.Data(),&run);
  t1->SetBranchAddress(lumiName.Data(),&lumi);
  t1->SetBranchAddress(eventName.Data(),&event);

  std::unordered_set<EventObj>knownEvents;

  unsigned int nEntries = t1->GetEntries();
  for (unsigned int iE=0; iE!=nEntries; ++iE) {
    t1->GetEntry(iE);
    EventObj knownEvent;
    knownEvent.run = run; knownEvent.lumi = lumi; knownEvent.evt = event;
    knownEvents.insert(knownEvent);
  } 
  if (verbose)
    PInfo("DuplicateRemover::Merge",
        TString::Format("\t%llu/%llu events from t1\n",
                        t1->GetEntries(),t1->GetEntries()));

  TEntryList *mask = new TEntryList(t2);
  t2->SetBranchAddress(runName.Data(),&run);
  t2->SetBranchAddress(lumiName.Data(),&lumi);
  t2->SetBranchAddress(eventName.Data(),&event);
  nEntries = t2->GetEntries();
  for (unsigned int iE=0; iE!=nEntries; ++iE) {
    t2->GetEntry(iE);
    EventObj thisEvent;
    thisEvent.run = run; thisEvent.lumi = lumi; thisEvent.evt = event;
    if (knownEvents.find(thisEvent)==knownEvents.end()) {
      mask->Enter(iE,t2);
    }
  } 
  t2->SetEntryList(mask);

  TFile *fOut = new TFile(fOutPath.Data(),"RECREATE");
  TTree *tCopied = t2->CopyTree("1==1");
  if (verbose)
    PInfo("DuplicateRemover::Merge",
        TString::Format("\t%llu/%llu events from t2\n",
                        tCopied->GetEntries(),t2->GetEntries()));
  TList *col = new TList(); col->Add(t1);
  tCopied->Merge(col);
  if (verbose)
    PInfo("DuplicateRemover::Merge",
        TString::Format("\t%llu/%llu events merged\n",
                        tCopied->GetEntries(),
                        t1->GetEntries()+t2->GetEntries()));
  fOut->WriteTObject(tCopied,treeName.Data());
  fOut->Close();

  mask->Delete();
}

void DuplicateRemover::Merge(TString f1Path, TString f2Path, TString fOutPath) 
{
  PInfo("DuplicateRemover::Merge",TString::Format("merging %s",f1Path.Data()));
  PInfo("DuplicateRemover::Merge",TString::Format("      + %s",f2Path.Data()));
  PInfo("DuplicateRemover::Merge",TString::Format("     => %s",fOutPath.Data()));
  verbose=true;
  TFile *f1 = new TFile(f1Path.Data());
  TFile *f2 = new TFile(f2Path.Data());
  TTree *t1 = (TTree*)f1->Get(treeName.Data());
  TTree *t2 = (TTree*)f2->Get(treeName.Data());
  Merge(t1,t2,fOutPath);
}
