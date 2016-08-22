#include "TH1F.h"
#include "TH2F.h"
#include "TString.h"
#include <vector>

#ifndef COMMONUTILITIES_H
#define COMMONUTILITIES_H

#define PI 3.141592654
/*
 * Things defined in this header:
 * activateBranch:  activates a branch in a TTree for reading
 * Info,Warning,Error: fprintf messages
 * DeltaR2: calculates delta r squared
 * getVal: find scalefactor, etc from 1d or 2d histogram
 * bound: bound a value between min and max
 * ProgressReporter: class to report progress of a loop 
 * Binner: class to discretely bin a continuous value
 * EventObj: hashable class that contains run,lumi,event
 * */

inline void activateBranch(TTree *t, const char *bname, void *address) {                                  
  t->SetBranchStatus(bname,1);                                                                   
  t->SetBranchAddress(bname,address);                                                            
}                      

inline void PInfo(const char *module, const char *msg) {
  printf("INFO [%s]: %s\n",module,msg);
}

inline void PDebug(const char *module, const char *msg) {
  fprintf(stderr,"DEBUG [%s]: %s\n",module,msg);
}

inline void PWarning(const char *module, const char *msg) {
  fprintf(stderr,"WARNING [%s]: %s\n",module,msg);
}

inline void PError(const char *module, const char *msg) {
  fprintf(stderr,"ERROR [%s]: %s\n",module,msg);
}

inline double DeltaR2(double eta1, double phi1, double eta2, double phi2) {
  float dEta2 = (eta1-eta2); dEta2 *= dEta2;
  float dPhi = TMath::Abs(phi1-phi2);
  if (dPhi>PI) dPhi = 2*PI-dPhi;
  return dEta2 + dPhi*dPhi;
}

inline double getVal(TH1D*h,double val) {
  return h->GetBinContent(h->FindBin(val));
}

inline double getVal(TH2D*h,double val1, double val2) {
  return h->GetBinContent(h->FindBin(val1,val2));
}

inline double bound(double val, double low, double high) {
  return TMath::Max(low,TMath::Min(high,val));
}

class ProgressReporter {
  public:
    ProgressReporter(const char *n, unsigned int *iE, unsigned int *nE, unsigned int nR=100) {
      name = n;
      idx = iE;
      N = nE;
      frequency = nR;
    }
    ~ProgressReporter() { }
    void Report() {
      float progress = 1.*(*idx)/(*N);
      if ( progress >= threshold) {
        PInfo(name.Data(),TString::Format("%5.2f%% (%u/%u)",progress*100,*idx,*N).Data());
        threshold += 1./frequency;
      }
    }
  private:
    unsigned int *idx, *N;
    unsigned int frequency;
    TString name;
    float threshold=0;
};

class Binner {
  public:
    Binner(std::vector<double> bounds_) {
      bounds = bounds_;
      nB = bounds.size();
    }
    ~Binner() {}
    unsigned int bin(double x) {
      for (unsigned int iB=0; iB!=nB; ++iB) {
        if (x<bounds.at(iB))
          return iB;
      }
      return nB-1; // return top bin if out of bounds
    }
    double getMax() { return bounds.at(nB-1); }
    double getMin() { return bounds.at(0);    }
  private:
    std::vector<double> bounds;
    unsigned int nB=0;
}; 

struct EventObj {
  int run, lumi;
  ULong64_t evt;
  bool operator==(const EventObj &other) const {
    return ( run==other.run && lumi==other.lumi && evt==other.evt);
  }
};

namespace std{
  template<> struct hash<EventObj> {
    std::size_t operator()(const EventObj &e) const {
      using std::size_t;
      using std::hash;
      return  ( ( ( hash<int>()(e.run) ^ (hash<int>()(e.lumi) << 1) ) >> 1 ) ^ (hash<ULong64_t>()(e.evt) << 1) ); 
    }
  };
}

#endif
