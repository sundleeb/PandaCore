#ifndef PANDACORE_TOOLS_THINGDRAWER
#define PANDACORE_TOOLS_THINGDRAWER

#include "TTree.h"
#include "TChain.h"
#include "TString.h"
#include "vector"
#include "TROOT.h"
#include "TCut.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TLegend.h"
#include  <vector>
#include "TStyle.h"
#include "algorithm"
#include "PandaCore/Tools/interface/Common.h"

enum ProcessType {
  kData=0,
  kSignal,
  kSignal1,
  kSignal2,
  kSignal3,
  kQCD,
  kTTbar,
  kWjets,
  kZjets,
  kST,
  kDiboson,
  kGjets,
  kExtra1,   // used for things like fakes
  kExtra2,   
  kExtra3,   
  kExtra4,   
  kExtra5,   
  kExtra6,   
  nProcesses
};

class CanvasDrawer
{
public:
  CanvasDrawer(double x=-1, double y=-1);
  CanvasDrawer(TCanvas *c0);
  ~CanvasDrawer();

  void SetCanvas(TCanvas *c0);

  void Stack(bool b)  {doStack = b;}
  void StackSignal(bool b)  {doStackSignal = b; if (b) doStack=true;}
  void Logy(bool b=true)	{ doLogy = b; maxScale = (b) ? 100 : 1.5; }
  void SetNormFactor(bool b)  { doSetNormFactor = b;  }
  void SetMaxScale(double d) { maxScale = d; }
  void Ratio(bool b=true)  { doRatio = b;  }
  void FixRatio(double m=2) { fixRatio=true; ratioMax=m; }

  void SetLumi(float f) { lumi = f; }
  void AddPlotLabel(const char *s, double x, double y, bool drawImmediately=true, int font=42, float textSize=-1, int textAlign=11);
  void SetSignalScale(double d) { signalScale = d; }
  void InitLegend(double x0=0.6, double y0=0.55, double x1=0.88, double y1=0.9);

  virtual void Draw(TString outDir,TString baseName);
  void SetDrawOption(const char *s) { drawOption=s; }
  void DrawMCErrors(bool b) { doDrawMCErrors=b; }
  void SetEvtNum(const char *s) { eventnumber=s; }
  void SetEvtMod(int i) { eventmod=i; }

  void SetTDRStyle();
  void SetRatioStyle();
  void AddCMSLabel();
  void AddLumiLabel(bool fb=true, double customLumi=-1);
  void ClearCanvas() { c->Clear(); }
  void cd() { c->cd(); }

  bool HasLegend() { return legend!=0; }

protected:
  class Label {
    public:
      Label(const char *n, double x0, double y0, int f0=42, float s0=-1, int a0=11) {
        strcpy(name,n);
        x = x0;
        y = y0;
        font = f0;
        size = s0;
        align = a0;
      }
      ~Label() { };
    char name[500];
    double x;
    double y;
    int font,align;
    float size;
  };

  int PlotColors[20] = {1,
                        kBlack,
                        kBlue,
                        kRed,
                        kGreen+3,
                        kMagenta-10,
                        kOrange-3,
                        kGreen-10,
                        kCyan-4,
                        kRed-9,
                        kYellow-9,
                        kCyan-4,
                        kMagenta+1,
                        kCyan+1,
                        kGreen+1,
                        kOrange+8,
                        kTeal+10,
                        kViolet,
                        kAzure,
                        kSpring+8};
  int GraphColors[18] = {
                        1, //black
                        2,  //red
                        4,  //purple
                        kAzure+3,  
                        kGreen+2,
                        kBlue-3,
                        6,  //magenta
                        kRed-7,
                        kMagenta+1,
                        kViolet,
                        kCyan+1,
                        kOrange+8,
                        kTeal+10,
                        kOrange+10,
                        kAzure,
                        kRed+3,
                        kMagenta+3,
                        kCyan+3
  };

  TCanvas *c=0;
  bool canvasIsOwned=false;
  bool doStack=false; // option to stack everything but data
  bool doStackSignal=false; // option to stack signal as well
  bool doLogy=false;
  bool doDrawMCErrors=false; // option to draw MC errors
  bool doSetNormFactor=false; // option to scale mc to match data (if doStack) else SetNormFactor() everything
  bool doRatio=false;
  bool fixRatio=false;
  double ratioMax=2;
  TCut mcWeight;
  float lumi=-1;
  TLatex *label=0;
  TLegend *legend=0;
  double signalScale=1;
  TString drawOption="hist";
  std::vector<Label> plotLabels; // labels to be plotted right before saving the canvas
  TString eventnumber="eventNumber";
  int eventmod=0;
  int whichstyle=0;
  double maxScale=1.5;
};
#endif
