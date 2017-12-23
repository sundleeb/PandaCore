/**
 * \file PlotUtility.h
 * \brief Defines the PlotUtility class
 * \author S. Narayanan
 */

#ifndef PANDACORE_TOOLS_PLOTUTILITY
#define PANDACORE_TOOLS_PLOTUTILITY

#include "TTree.h"
#include "TChain.h"
#include "TString.h"
#include "vector"
#include "TROOT.h"
#include "TCut.h"
#include "HistogramDrawer.h"
#include "PandaCore/Tools/interface/TreeTools.h"

/**
 * \brief Defines a single process to be plotted. Can have several input trees
 */
class Process {
public:
  /**
   * \brief Constructor
   * \param n name of this process
   * \param c the defining ProcessType
   * \param customColor a custom choice of color, if none is provided one is set automatically
   */
  Process(TString n, ProcessType c, ProcessType customColor=nProcesses) { 
    name = n; 
    processtype = c; 
    if (customColor==nProcesses) 
      color=c;
    else
      color=customColor;
  }
  /**
   * \brief Destructor
   */
  ~Process() {
    for (auto *f : files)
      f->Close();
  }
  /**
   * \brief Initialize the process
   * \param treeName name of the tree to harvest from files
   */
  void Init(const char *treeName = "events") {
    treename = treeName;
  }
  /**
   * \brief Add a file to this process
   * \param s file path
   */
  void AddFile(TString s) {
    TFile *f = new TFile(s);
    files.push_back(f);

    TTree *t = (TTree*)f->FindObjectAny(treename);
    trees.push_back(t);

    for (auto fn : friendnames) {
      TTree *tf = (TTree*)f->FindObjectAny(fn);
      friends.push_back(tf);
      t->AddFriend(tf);
    }
  }

  /**
   * \brief Add a friend tree to the main tree
   * \param s name of friend tree
   */
  void AddFriend(const char *s) {
    friendnames.push_back(s);
  }
  bool dashed=false; //!< dashed lines?
  bool useCommonWeight=true; //!< if an externally-defined weight should be applied
  bool useCommonCut=true; //!< if an externally-defined cut should be applied
  TCut additionalCut = "1==1"; //!< anded with whatever cut is being applied
  TCut additionalWeight = "1"; //!< multiplied with whatever weight is being applied
  TString name; //!< name of process
  ProcessType processtype; //!< type of process
  ProcessType color; //!< color of the output histogram
  std::vector<TTree*> trees; //!< the trees contained in this process
private:
  TString treename="events"; //!< name of trees to harvest
  std::vector<TString>friendnames; //!< names of friends to harvest
  std::vector<TTree*>friends; //!< these are not accessed externally; just kept here for GC
  std::vector<TFile*>files; //!< these are not accessed externally; just kept here for GC
};

/**
 * \brief A class to define a distribution to be plotted
 */
class Distribution {
public:
  /**
   * \brief Variable width bin constructor
   * \param n variable to be plotted
   * \param nb number of bins
   * \param x x-label
   * \param y y-label
   * \param minY_ minimum y-bound
   * \param maxY_ maximum y-bound
   * \param fn filename, if none is provided n is used
   */
  Distribution(TString n, int nb, TString x, TString y, double minY_=999, double maxY_=-999, TString fn="") {
    name = n;
    nBins = nb;
    xLabel = x;
    yLabel = y;
    maxY = maxY_;
    minY = minY_;
    binEdges = new float[nBins+1];
    filename = (fn!="") ? fn : n;
  }
  /**
   * \brief Fixed-width bin constructor
   * \param n variable to be plotted
   * \param m low
   * \param M high
   * \param nb number of bins
   * \param x x-label
   * \param y y-label
   * \param minY_ minimum y-bound
   * \param maxY_ maximum y-bound
   * \param fn filename, if none is provided n is used
   */
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
  /**
   * \brief Destructor
   */
  ~Distribution() { delete binEdges; }
  /**
   * \brief Add a bin edge 
   * \param e low edge of bin
   *
   * 
   * Should be replaced by access to std::vector in pyROOT
   */
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
  int nBins;
  TString xLabel;
  TString yLabel;
private:
  int counter=0;
};

/**
 * \brief A class to make analysis plots given TTrees
 */
class PlotUtility : public HistogramDrawer
{
public:
  PlotUtility(double x=-1, double y=-1); //!< Constructor
  virtual ~PlotUtility();

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
  void SetDoOverflow(bool b=true) { doOverflow = b; }
  void SetDoUnderflow(bool b=true) { doUnderflow = b; }
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
  TString plotLabel="#it{CMS Preliminary}";
  bool doOverflow=false;
  bool doUnderflow=false;

  std::vector<TCut> mcWeightUp, mcWeightDown;
  std::vector<int> systColors;
  std::vector<TString> systNames;
};
#endif
