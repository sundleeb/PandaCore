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

/** \file Common.h
 * \brief Common function header file
 *
 * Just some stuff that are used all over the place
 */


//////////////////////////////////////////////////////////////////////////////////

/**
 * \param v1  reference to vector that is lengthened
 * \param v2  vector to add to v1
 * \brief Function to concatenate two vectors
 */
template <typename T> inline
void concat(std::vector<T> &v1,std::vector<T> v2) {
  v1.insert(v1.end(),v2.begin(),v2.end());
}

/**
 * \param x  value to be cleaned
 * \param d  default value
 * \brief Returns a default value if input is NaN
 */
inline double clean(double x, double d=-1) {
  return (x==x) ? x : d;
}

/**
 * \param t  input tree
 * \param bname  name of branch
 * \param address  address of branch
 * \brief Turns on a branch in a TTree and sets the address to a specified location
 */
inline void activateBranch(TTree *t, const char *bname, void *address) {                                  
  t->SetBranchStatus(bname,1);                                                                   
  t->SetBranchAddress(bname,address);                                                            
}                      

/**
 * \param module  name of caller
 * \param module  msg message to be printed
 * \param newline  newline to use at end of message
 * \brief Prints to info stdout
 */
inline void PInfo(const char *module, const char *msg, const char *newline="\n") {
          printf("INFO    [%-40s]: %s%s",module,msg,newline);
}

/**
 * \param module  name of caller
 * \param module  msg message to be printed
 * \param newline  newline to use at end of message
 * \brief Prints debug to  stderr
 */
inline void PDebug(const char *module, const char *msg, const char *newline="\n") {
  fprintf(stderr,"DEBUG   [%-40s]: %s%s",module,msg,newline);
}

/**
 * \param module  name of caller
 * \param module  msg message to be printed
 * \param newline  newline to use at end of message
 * \brief Prints warning to  stderr
 */
inline void PWarning(const char *module, const char *msg, const char *newline="\n") {
  fprintf(stderr,"WARNING [%-40s]: %s%s",module,msg,newline);
}

/**
 * \param module  name of caller
 * \param module  msg message to be printed
 * \param newline  newline to use at end of message
 * \brief Prints error to  stderr
 */
inline void PError(const char *module, const char *msg, const char *newline="\n") {
  fprintf(stderr,"ERROR   [%-40s]: %s%s",module,msg,newline);
}

/**
 * \param h  histogram to read
 * \param val  x-value
 * \brief Returns the value of a 1D histogram given an x-val
 */
inline double getVal(TH1D*h,double val) {
  return h->GetBinContent(h->FindBin(val));
}

/**
 * \param h  histogram to read
 * \param val1  x-value
 * \param val2  y-value
 * \brief Returns the value of a 2D histogram given an x-val,y-val
 */
inline double getVal(TH2D*h,double val1, double val2) {
  return h->GetBinContent(h->FindBin(val1,val2));
}

/**
 * \param val  input value
 * \param low  low value
 * \param high  high value
 * \brief Bounds a value within a range
 */
inline double bound(double val, double low, double high) {
  return TMath::Max(low,TMath::Min(high,val));
}

/**
 * \param cut  string to parse
 * \brief Finds all substrings in a string that might be branch dependencies
 */
inline std::vector<TString> getDependencies(TString cut) {
  std::vector<TString> deps;
  int nChars = cut.Length();
  TString tmpString="";
  for (int iC=0; iC!=nChars; ++iC) {
    const char c = cut[iC];
    if ( c==' ' || c=='&' || c=='|' || c=='(' || c==')' 
        || c=='*' || c=='+' || c=='-' || c=='/' || c=='!' 
        || c=='<' || c=='>' || c=='=' || c=='.' || c==','
        || c=='[' || c==']') {
      if (tmpString != "" && !tmpString.IsDigit() && 
          // tmpString!="Pt" && tmpString!="Eta" && tmpString!="Phi" &&
          !(tmpString.Contains("TMath")) && !(tmpString=="fabs")) {
        deps.push_back(tmpString);
      }
      tmpString = "";
    } else {
        tmpString.Append(c);
    }
  }
  if (tmpString != "" && !tmpString.IsDigit() && 
      // tmpString!="Pt" && tmpString!="Eta" && tmpString!="Phi" &&
      !tmpString.Contains("TMath")) {
    deps.push_back(tmpString);
  }
  return deps;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Reports progress of some process
 *
 * Given a total length of a task and an integer to track the progress,
 * this class prints the progress at regular intervals
 */
class ProgressReporter {
  public:
    /**
     * \param n  name of this task
     * \param iE  pointer to progress tracker
     * \param nE  pointer to total length of task
     * \param nR  number of reports desired
     * \brief Constructor
     */
    ProgressReporter(const char *n, unsigned int *iE, unsigned int *nE, unsigned int nR=100) {
      name = n; name+="::Progress";
      idx = iE;
      N = nE;
      frequency = nR;
    }
    /**
     * \brief Destrucctor
     */
    ~ProgressReporter() { }
    /**
     * \brief Reports if threshold has been reached
     */
    void Report() {
      float progress = 1.*(*idx)/(*N);
      if ( progress >= threshold) {
        PInfo(name.Data(),
            TString::Format("%-40s",TString::Format("%5.2f%% (%u/%u)",progress*100,*idx,*N).Data()).Data(),
            "\n");
        threshold += 1./frequency;
      }
    }
  private:
    unsigned int *idx, *N; /**< addresses of progress trackers */
    unsigned int frequency; /**< how often to print */
    TString name; /**< name of this task */
    float threshold=0; /**< internal tracker of next threshold */
};

//////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Keeps track of events and real time
 *
 * Reports the occurence of a user-specified event
 * and the amount of time that has passed between
 * consecutive events. Sub-events can also be tracked,
 * which do not reset the event stopwatch
 */
class TimeReporter {
  public:
    /**
     * \param n  name of the calling object
     * \param on_  whether this reporter is on
     * \brief Constructor
     */
    TimeReporter(const char *n, bool on_) {
      on=on_;
      name = n; name += "::Time";
      sw = new TStopwatch();
      subsw = new TStopwatch();
    }
    /**
     * \brief Destructor
     */
    ~TimeReporter() { delete sw; }
    /**
     * \brief Starts all stopwatches and resets counters
     */
    void Start() {
      if (on) {
        sw->Start(true);
        subsw->Start(true);
      }
      currentEvent=0;
      currentSubEvent=1;
    }
    /**
     * \param s  name of event
     * \param reset  whether to reset the stopwatch for the next event
     * \brief Triggers a top-level event
     */
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
    /**
     * \param s  name of sub-event
     * \brief Triggers a sub-level event
     */
    void TriggerSubEvent(const char *s) {
      if (!on)
        return;
      PDebug(name,TString::Format("%2i.%-2i: %.3f (%s)",currentEvent,currentSubEvent,subsw->RealTime()*1000,s).Data());
      currentSubEvent+=1;
      subsw->Start();
    }
  private:
    TString name; /**< name of this task */
    bool on; /**< whether this reporter is active */
    TStopwatch *sw, *subsw; /**< internal stopwatches */
    int currentEvent=0; /**< internal tracker of the index of the current event */
    int currentSubEvent=1; /**< internal tracker of the index of the sub-event */
};

//////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Bins a continuous variable
 *
 */
class Binner {
  public:
    /**
     * \param bounds_  boundaries of the variable
     * \brief Constructor
     */
    Binner(std::vector<double> bounds_) {
      bounds = bounds_;
      nB = bounds.size();
    }
    /**
     * \brief Destructor 
     */
    ~Binner() {}
    /**
     * \param x  value to bin
     * \brief Returns a bin
     */
    unsigned int bin(double x) {
      for (unsigned int iB=0; iB!=nB; ++iB) {
        if (x<bounds.at(iB))
          return iB;
      }
      return nB-1; // return top bin if out of bounds
    }
    /**
     * \brief Returns max boundary
     */
    double getMax() { return bounds.at(nB-1); }
    /**
     * \brief Returns min boundary
     */
    double getMin() { return bounds.at(0);    }
  private:
    std::vector<double> bounds; /**< internal boundary structure */
    unsigned int nB=0; /**< number of bins */
}; 

//////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Wrapper for run,lumi,event
 *
 * Simple struct containing run,lumi,event.
 * Hashing and == are implemented for use in
 * ordered and unordered containers.
 */
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
