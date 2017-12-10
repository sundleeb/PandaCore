#ifndef ROC
#define ROC

#include "TH1F.h"
#include "TFile.h"
#include "TGraph.h"
#include "TMarker.h"
#include "TMultiGraph.h"
#include "GraphDrawer.h"

class ROCTool {
  public:
    ROCTool() {}
    ~ROCTool();

    void InitCanvas(double x1=-1, double y1=-1, double x2=-1, double y2=-1, bool isPrelim=false);
    void SetFile(TFile *f);
    void SetFile(TString fpath);
    void SetHists(TH1F *hs, TH1F *hb) { sigHist = hs; bgHist = hb; }
    void SetHists(TString hspath, TString hbpath) { SetHists((TH1F*)centralFile->FindObjectAny(hspath),
                                                             (TH1F*)centralFile->FindObjectAny(hbpath)); }
    TGraph *CalcROC(TH1F *hs, TH1F *hb, const char *title=0, unsigned int color=1, int style=1, int nCuts=1); // 1 or 2
    TGraph *CalcROC(TString hspath, TString hbpath, const char *title=0, unsigned int color=1, int style=1, int nCuts=1);
    TGraph *CalcROC1Cut();
    TGraph *CalcROC2Cut();
    TMarker *CalcWP(TTree *ts, TTree *tb, int color, TString sigcut, TString sigweight, TString bgcut, TString bgweight, TString cut, TString title);

    void DrawAll(TString outDir, TString basePath);
    void Logy(bool b=true) { doLogy=b; }
    void InvertFake(bool b=true) { invertFake = b; }
    void SetPlotRange(float m, float M) { minval=m; maxval=M; }

    GraphDrawer *c=0;
  private:
    bool doLogy=false;
    float minval=0, maxval=1;
    bool fileIsOwned=false;
    bool invertFake = false;
    TFile *centralFile=0;
    TH1F *sigHist=0, *bgHist=0;

};


#endif
