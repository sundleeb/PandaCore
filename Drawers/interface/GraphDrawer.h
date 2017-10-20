
/**
 * \file GraphDrawer.h
 * \brief Defines the BaseGraphDrawaer class and its inherited classes
 * \author S. Narayanan
 */

#ifndef PANDACORE_TOOLS_GDRAWER
#define PANDACORE_TOOLS_GDRAWER

#include "CanvasDrawer.h"
#include "TMarker.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TFile.h"
#include <iostream>

/**
 * \brief Implements a base class for a generic graph drawer 
 */
template <typename T> class BaseGraphDrawer : public CanvasDrawer {
  public:
    BaseGraphDrawer(double x=-1, double y=-1); //!< Constructor
    virtual ~BaseGraphDrawer(); //!< Destructor

  /**
   * \brief Add a graph to be plotted
   * \param g the graph
   * \param label the label to be used in the legend
   * \param color the color
   * \param style the style
   * \param drawOption the draw option
   */
  void AddGraph(T *g, TString label,unsigned int color,int style=1, TString drawOption="L");
  /**
   * \brief Add a graph to be plotted from file
   * \param gname the key of the graph
   * \param label the label to be used in the legend
   * \param color the color
   * \param style the style
   * \param drawOption the draw option
   */
  void AddGraph(TString gname, TString label,unsigned int color,int style=1, TString drawOption="L");
  /**
   * \brief Add a single TMarker to the plot
   * \param m the marker
   * \param label the legend label
   */
  void AddMarker(TMarker *m, TString label);
  void SetInputFile(TString fname); //!< Set input file by path
  void SetInputFile(TFile *f); //!< Set input file
  void SetLineWidth(int l0) { lineWidth = l0; } //!< Set line width
  virtual void Draw(TString outDir, TString baseName) override; //!< Draw canvas
  
  /**
   * \brief Reset the object
   */
  void Clear() { graphs.clear(); labels.clear(); colors.clear();
                 styles.clear(); drawOptions.clear();  Reset();    } 

  protected:
    bool fileIsOwned=false; 
    int lineWidth=2;
    TFile *centralFile=0;
    std::vector<T*> graphs;
    std::vector<TString> labels;
    std::vector<int> colors;
    std::vector<int> styles;
    std::vector<TString> drawOptions;

    std::vector<TMarker*> markers;
    std::vector<TString> marker_labels;
};

template <typename T> BaseGraphDrawer<T>::BaseGraphDrawer(double x, double y):
  CanvasDrawer(x,y) {
    // done
}

template <typename T> BaseGraphDrawer<T>::~BaseGraphDrawer() {
  if (fileIsOwned)
    delete centralFile;
}

template <typename T>
void BaseGraphDrawer<T>::AddMarker(TMarker *m, TString label) {
  markers.push_back(m);
  marker_labels.push_back(label);
}

template <typename T> 
void BaseGraphDrawer<T>::AddGraph(T* g, TString label, unsigned int color, int style, TString drawOption) {
  if (g!=NULL) {
    g->SetTitle("");
    graphs.push_back(g);
    labels.push_back(label);
    colors.push_back(GraphColors[color]);
    styles.push_back(style);
    drawOptions.push_back(drawOption);
  }
}

template <typename T> 
void BaseGraphDrawer<T>::AddGraph(TString gname, TString label, unsigned int color, int style, TString drawOption) {
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

  unsigned int nMarkers = markers.size();
  for (unsigned int iM=0; iM!=nMarkers; ++iM) {
    TMarker *m = markers[iM];
    m->Draw("p same");
    if (legend) {
      legend->AddEntry(m,marker_labels[iM],"p");
    }
  }

  CanvasDrawer::Draw(outDir,baseName);
}

typedef BaseGraphDrawer<TGraph> GraphDrawer;
typedef BaseGraphDrawer<TGraphErrors> GraphErrDrawer;
typedef BaseGraphDrawer<TGraphAsymmErrors> GraphAsymmErrDrawer;

#endif
