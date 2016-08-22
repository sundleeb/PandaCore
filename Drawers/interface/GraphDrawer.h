#ifndef PANDACORE_TOOLS_GDRAWER
#define PANDACORE_TOOLS_GDRAWER

#include "CanvasDrawer.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TFile.h"
#include <iostream>

template <typename T> class BaseGraphDrawer : public CanvasDrawer {
  public:
    BaseGraphDrawer(double x=-1, double y=-1);
    virtual ~BaseGraphDrawer();

  void AddGraph(T *g, const char *label,unsigned int color,int style=1, const char *drawOption="L");
  void AddGraph(TString gname, const char *label,unsigned int color,int style=1, const char *drawOption="L");
  void SetInputFile(TString fname);
  void SetInputFile(TFile *f);
  void SetLineWidth(int l0) { lineWidth = l0; }
  virtual void Draw(TString outDir, TString baseName) override;
  
  void Clear() { graphs.clear(); labels.clear(); colors.clear();
                 styles.clear(); drawOptions.clear();            }

  protected:
    bool fileIsOwned=false;
    int lineWidth=2;
    TFile *centralFile=0;
    std::vector<T*> graphs;
    std::vector<const char*> labels;
    std::vector<int> colors;
    std::vector<int> styles;
    std::vector<const char*> drawOptions;
};

template <typename T> BaseGraphDrawer<T>::BaseGraphDrawer(double x, double y):
  CanvasDrawer(x,y) {
    // done
}

template <typename T> BaseGraphDrawer<T>::~BaseGraphDrawer() {
  if (fileIsOwned)
    delete centralFile;
  for (auto *label : labels)
    delete label;
}

template <typename T> 
void BaseGraphDrawer<T>::AddGraph(T* g, const char *label, unsigned int color, int style, const char *drawOption) {
  if (g!=NULL) {
    g->SetTitle("");
    graphs.push_back(g);
    char *labelCpy = new char[100];
    strcpy(labelCpy,label);
    labels.push_back(labelCpy);
    colors.push_back(GraphColors[color]);
    styles.push_back(style);
    char *drawCpy = new char[10];
    strcpy(drawCpy,drawOption);
    drawOptions.push_back(drawOption);
  }
}

template <typename T> 
void BaseGraphDrawer<T>::AddGraph(TString gname, const char *label, unsigned int color, int style, const char *drawOption) {
  T *g = (T*)centralFile->Get(gname);
  AddGraph(g,label,color,style,drawOption);
}

template <typename T> 
void BaseGraphDrawer<T>::SetInputFile(TFile *f) {
  centralFile = f;
}

template <typename T> 
void BaseGraphDrawer<T>::SetInputFile(TString fname) {
  fileIsOwned = true;
  centralFile = TFile::Open(fname);
}

template <typename T> 
void BaseGraphDrawer<T>::Draw(TString outDir, TString baseName) {
  unsigned int nGraphs = graphs.size();
  if (nGraphs==0)
    return;
  
  if (c==NULL) {
    c = new TCanvas();
    canvasIsOwned=true;
  }

  c->Clear();
  if (doLogy)
    c->SetLogy();

  c->cd();

  for (unsigned int iG=0; iG!=nGraphs; ++iG) {
    T *graph = graphs[iG];
    graph->SetLineColor(colors[iG]);
    graph->SetLineWidth(lineWidth);
    graph->SetLineStyle(styles[iG]);
    TString drawOption(drawOptions[iG]);
    if (iG==0)
      drawOption += " A";  // draw the axes
    else
      drawOption += " same";

    graph->Draw(drawOption);

    if (legend) {
      TString legOption("L");
      if (drawOption.Contains("3"))
        legOption = "F";
      legend->AddEntry(graph,labels[iG],legOption);
    }

  }

  CanvasDrawer::Draw(outDir,baseName);
}

typedef BaseGraphDrawer<TGraph> GraphDrawer;
typedef BaseGraphDrawer<TGraphErrors> GraphErrDrawer;
typedef BaseGraphDrawer<TGraphAsymmErrors> GraphAsymmErrDrawer;

#endif
