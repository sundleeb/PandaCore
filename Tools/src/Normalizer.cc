#include "../interface/Normalizer.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
using namespace std;
void Normalizer::NormalizeTree(TTree *t, double totalEvts, double xsec) 
{
    float inWeight=1, outWeight=1;
    double inWeightD=1;
    t->SetBranchStatus("*",0);
    turnOnBranches(t,inWeightName.Data());
    TBranch *b = t->Branch(outWeightName.Data(),&outWeight,TString::Format("%s/F",outWeightName.Data()));
    if (inWeightName!="") {
       if (isFloat){    
        t->SetBranchAddress(inWeightName.Data(),&inWeight);
        std::cout << "inWeight1= "<<inWeight<<std::endl;
        }
       else{
        t->SetBranchAddress(inWeightName.Data(),&inWeightD);
        std::cout << "inWeight2= "<<inWeight<<std::endl;
        }
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
    std::cout << "outWeight= "<<outWeight<<std::endl;
    std::cout << "inWeight= "<<inWeight<<std::endl;
    std::cout << "inWeightD= "<<inWeightD<<std::endl;
    std::cout << "xsec= "<<xsec<<std::endl;
    std::cout << "totalEvts= "<<totalEvts<<std::endl;
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
