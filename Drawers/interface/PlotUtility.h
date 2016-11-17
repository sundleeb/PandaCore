#ifndef PANDACORE_TOOLS_PLOTUTILITY
#define PANDACORE_TOOLS_PLOTUTILITY

#include "TTree.h"
#include "TChain.h"
#include "TString.h"
#include "vector"
#include "TROOT.h"
#include "TCut.h"
#include "HistogramDrawer.h"

class Process {
public:
  Process(TString n, ProcessType c, ProcessType customColor=nProcesses) { 
    name = n; 
    processtype = c; 
    chain = NULL;
    if (customColor==nProcesses) 
      color=c;
    else
      color=customColor;
  }
  ~Process() {
    delete chain;
  }
  void Init(const char *treeName = "events") {
    chain = new TChain(treeName);
  }
  void AddFile(TString s) {
    if (chain==NULL) Init();
    chain->AddFile(s);
    for (TChain *f : friends)
      f->AddFile(s);
  }
  void AddFriend(const char *s) {
    friends.push_back(new TChain(s));
    chain->AddFriend(friends.back());
  }
  bool dashed=false;
  bool useCommonWeight=true;
  bool useCommonCut=true;
  TCut additionalCut = "1==1"; // anded with whatever cut is being applied
  TCut additionalWeight = "1"; // multiplied with whatever cut is being applied
  TString name;
  ProcessType processtype;
  ProcessType color;
  TChain *chain;
  TTree *clonedTree=0;
  std::vector<TChain*>friends;
};

class Distribution {
public:
  Distribution(TString n, int nb, TString x, TString y, double minY_=999, double maxY_=-999, TString fn="") {
    // this is the variable bin constructor
    name = n;
    nBins = nb;
    xLabel = x;
    yLabel = y;
    maxY = maxY_;
    minY = minY_;
    binEdges = new float[nBins+1];
    filename = (fn!="") ? fn : n;
  }
  Distribution(TString n, float m, float M, int nb, TString x, TString y, double minY_=999, double maxY_=-999, TString fn="") {
    name = n;
    min = m;
    max = M;
    maxY = maxY_;
    minY = minY_;
    nBins = nb;
    xLabel = x;
    yLabel = y;
    filename = (fn!="") ? fn : n;
  }
  ~Distribution() { delete binEdges; }
  void AddBinEdge(float e) {
    binEdges[counter] = e;
    if (counter==0)
      min=e;
    else if (counter==nBins)
      max=e;
    ++counter;
  }
  TString name, filename;
  float min; 
  float max;
  float minY;
  float maxY;
  float *binEdges=0;
  int counter=0;
  int nBins;
  TString xLabel;
  TString yLabel;
};

class PlotUtility : public HistogramDrawer
{
public:
  PlotUtility(double x=-1, double y=-1);
  ~PlotUtility();

  void AddProcess(Process *p);
  void AddFile(ProcessType pt,TString filePath,TString name="");
  void ClearProcess(ProcessType pt);

  void AddDistribution(Distribution *d) { distributions.push_back(d); }

  void SetCut(const char *s)	{ cut = TCut(s); }
  void SetCut(TCut s)	{ cut = s; }
  void SetMCWeight(const char *s) { mcWeight = TCut(s); }
  void SetMCWeight(TCut s) { mcWeight = s; }
  void AddSyst(TString up, TString down, TString name, int color=1) { AddSyst(TCut(up),TCut(down),name,color); }
  void AddSyst(TCut up, TCut down, TString name, int color=1) { mcWeightUp.push_back(up);
                                                                mcWeightDown.push_back(down);
                                                                systNames.push_back(name);
                                                                systColors.push_back(color); }
  void SetPlotLabel(const char *s)  { plotLabel = s;  }
  void SetSignalScale(double d) { signalScale = d; }
  void CloneTrees(bool b) { cloneTrees = b; }
  void Reset(bool clearPlotLabels=true);

  void DrawAll(TString outDir);
  void Draw(TString outDir, TString baseName) { return; } // not implemented
  
private:
  struct PlotWrapper {
    std::vector<TH1D*> ownedHistos; // for garbage collection
    std::vector<TH1D*> hSystUp, hSystDown;
    std::map<Process*,TH1D*> histos;
    TTreeFormula *tf=0;
  };
  Process **processes; 
  bool *owned;
  std::vector<Distribution*> distributions;
  std::vector<unsigned int> order;
  TCut cut;
  TCut mcWeight;
  bool cloneTrees=false;
  TString plotLabel="#it{CMS Preliminary}";

  std::vector<TCut> mcWeightUp, mcWeightDown;
  std::vector<int> systColors;
  std::vector<TString> systNames;
};
#endif
