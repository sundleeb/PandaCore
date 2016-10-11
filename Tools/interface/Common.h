#include "TH1F.h"
#include "TTree.h"
#include "TH2F.h"
#include "TString.h"
#include "TStopwatch.h"
#include <vector>
#include "Functions.h"

#ifndef COMMONUTILITIES_H
#define COMMONUTILITIES_H

#define PI 3.141592654
/*
 * Things defined in this header:
 * activateBranch:  activates a branch in a TTree for reading
 * Info,Warning,Error: fprintf messages
 * getVal: find scalefactor, etc from 1d or 2d histogram
 * bound: bound a value between min and max
 * ProgressReporter: class to report progress of a loop 
 * TimeReporter : class to report time taken for events
 * Binner: class to discretely bin a continuous value
 * EventObj: hashable class that contains run,lumi,event
 * */

//////////////////////////////////////////////////////////////////////////////////

inline void activateBranch(TTree *t, const char *bname, void *address) {                                  
  t->SetBranchStatus(bname,1);                                                                   
  t->SetBranchAddress(bname,address);                                                            
}                      

inline void PInfo(const char *module, const char *msg) {
          printf("INFO    [%-40s]: %s\n",module,msg);
}

inline void PDebug(const char *module, const char *msg) {
  fprintf(stderr,"DEBUG   [%-40s]: %s\n",module,msg);
}

inline void PWarning(const char *module, const char *msg) {
  fprintf(stderr,"WARNING [%-40s]: %s\n",module,msg);
}

inline void PError(const char *module, const char *msg) {
  fprintf(stderr,"ERROR   [%-40s]: %s\n",module,msg);
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

//////////////////////////////////////////////////////////////////////////////////

class ProgressReporter {
  public:
    ProgressReporter(const char *n, unsigned int *iE, unsigned int *nE, unsigned int nR=100) {
      name = n; name+="::Progress";
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

//////////////////////////////////////////////////////////////////////////////////

class TimeReporter {
  public:
    TimeReporter(const char *n, bool on_) {
      on=on_;
      name = n; name += "::Time";
      sw = new TStopwatch();
      subsw = new TStopwatch();
    }
    ~TimeReporter() { delete sw; }
    void Start() {
      if (on) {
        sw->Start(true);
        subsw->Start(true);
      }
      currentEvent=0;
      currentSubEvent=1;
    }
    void TriggerEvent(const char *s,bool reset=true) {
      if (!on)
        return;
      currentSubEvent=1;
      PDebug(name,TString::Format("%2i   : %.3f (%s)",currentEvent,sw->RealTime()*1000,s).Data());
      sw->Start();
      subsw->Start();
      if (reset)
        currentEvent+=1;
    }      
    void TriggerSubEvent(const char *s) {
      if (!on)
        return;
      PDebug(name,TString::Format("%2i.%-2i: %.3f (%s)",currentEvent,currentSubEvent,subsw->RealTime()*1000,s).Data());
      currentSubEvent+=1;
      subsw->Start();
    }
  private:
    TString name;
    bool on;
    TStopwatch *sw, *subsw;
    int currentEvent=0;
    int currentSubEvent=1;
};

//////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////

#endif
