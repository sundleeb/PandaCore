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

  void AddHistogram(TH1F *h, TString label, ProcessType pt=nProcesses, int cc = -1);
  void AddHistogram(TString hname, TString label, ProcessType pt=nProcesses, TString fname="");
  void AddAdditional(TObject *o, TString opt="", TString aname="") { std::pair<TObject*,TString> p(o,opt); additionals.push_back(p); anames.push_back(aname); }
  void SetInputFile(TString fname);
  void SetInputFile(TFile *f);
  void SetAbsMin(double f) { absMin = f; }
  void Reset();

  virtual void Draw(TString outDir, TString baseName) override;

  protected:
    bool fileIsOwned=false;
    TFile *centralFile=0;
    std::vector<TH1F*> internalHistos;
    std::vector<ProcessType> ptypes;
    std::vector<TString> labels;
    std::vector<int> customColors;
    std::vector<std::pair<TObject*,TString>> additionals;
    std::vector<TString> anames;
    double absMin=-999;
};

#endif
