#include "../interface/Normalizer.h"

void Normalizer::NormalizeTree(TTree *t, double totalEvts, double xsec) 
{
    float inWeight=1, outWeight=1;
    double inWeightD=1;
    t->SetBranchStatus("*",0);
    turnOnBranches(t,inWeightName.Data());
    TBranch *b = t->Branch(outWeightName.Data(),&outWeight,TString::Format("%s/F",outWeightName.Data()));
    if (inWeightName!="") {
       if (isFloat)    
         t->SetBranchAddress(inWeightName.Data(),&inWeight);
       else
        t->SetBranchAddress(inWeightName.Data(),&inWeightD);
    } else {
        inWeight = 1;
        inWeightD = 1;
    }
    unsigned int nEntries = t->GetEntries();
    unsigned int iE=0;
    ProgressReporter pr("Normalizer::NormalizeTree",&iE,&nEntries,10);
    for (iE=0; iE!=nEntries; ++iE) {
        pr.Report();
        t->GetEntry(iE);
        if (isFloat)
            outWeight = xsec*inWeight/totalEvts;
        else
            outWeight = xsec*inWeightD/totalEvts;
        b->Fill();
    }
    t->SetBranchStatus("*",1);
}


void Normalizer::NormalizeTree(TString fpath, double xsec) 
{
    TFile *fIn = TFile::Open(fpath,"UPDATE");
    TTree *t = (TTree*)fIn->Get(treeName.Data());
    TH1F *h = (TH1F*)fIn->Get(histName.Data());
    if (t==NULL || h==NULL) {
        PError("Normalizer::NormalizeTree",TString::Format("Could not normalize %s because tree=%p and hist=%p\n",fpath.Data(),t,h));
        return; 
    }
    NormalizeTree(t,h->Integral(),xsec);
    fIn->WriteTObject(t,treeName.Data(),"overwrite");
    fIn->Close();
}
