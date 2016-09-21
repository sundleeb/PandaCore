#ifndef PANDACORE_TOOLS_HDRAWER
#define PANDACORE_TOOLS_HDRAWER

#include "CanvasDrawer.h"
#include "TH1F.h"
#include "TObject.h"
#include "TFile.h"
#include <utility>

class HistogramDrawer : public CanvasDrawer {
  public:
    HistogramDrawer(double x=-1, double y=-1);
    ~HistogramDrawer();

  void AddHistogram(TH1F *h, TString label, ProcessType pt=nProcesses, int cc = -1, TString opt="");
  void AddHistogram(TString hname, TString label, ProcessType pt=nProcesses, TString fname="");
  void AddAdditional(TObject *o, TString opt="", TString aname="");
  void SetInputFile(TString fname);
  void SetInputFile(TFile *f);
  void SetAbsMin(double f) { absMin = f; }
  void Reset();

  virtual void Draw(TString outDir, TString baseName) override;

  protected:
    class HistWrapper {
      public:
        HistWrapper() {}
        ~HistWrapper() {}
        TH1F *h;
        TString label;
        ProcessType pt;
        int cc;
        TString opt;
    };
    class ObjWrapper {
      public:
        ObjWrapper() {}
        ~ObjWrapper() {}
        TObject *o;
        TString label;
        TString opt;
    };
    bool fileIsOwned=false;
    TFile *centralFile=0;
    std::vector<HistWrapper> internalHists;
    std::vector<ObjWrapper> internalAdds;
    std::vector<std::pair<TObject*,TString>> additionals;
    std::vector<TString> addnames;
    double absMin=-999;
};

#endif
