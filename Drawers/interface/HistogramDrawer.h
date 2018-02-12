/**
 * \file HistogramDrawer.h
 * \brief Defines the HistogramDrawer class
 * \author S. Narayanan
 */

#ifndef PANDACORE_TOOLS_HDRAWER
#define PANDACORE_TOOLS_HDRAWER

#include "CanvasDrawer.h"
#include "TH1D.h"
#include "TGraphErrors.h"
#include "TObject.h"
#include "TFile.h"
#include <utility>

/**
 * \brief Class to draw histograms on a single canvas
 *
 * This class takes a collection of histograms and plots them in various styles.
 * The histograms can be drawn on top of each other, as stacks (with data overlayed),
 * or other combinations. An interface is provided for adding other drawable TObjects on
 * top of the histogram plot. Ratio pads can be computed on the fly.
 */
class HistogramDrawer : public CanvasDrawer {
  public:
    /**
     * \brief Constructor
     * \param x x-width
     * \param y y-width
     */
    HistogramDrawer(double x=-1, double y=-1);
    virtual ~HistogramDrawer(); //!< Destructor

  /**
   * \brief Adds a histogram to be draw
   * \param h the TH1D to be drawn
   * \param label the label to be used if a legend is drawn
   * \param pt the ProcessType (used to internally manage the histogram, set color and order, etc)
   * \param cc a custom color. If none is provided, the one defined by pt is used
   * \param opt the drawing option, "HIST" by default
   */
  void AddHistogram(TH1D *h, TString label, ProcessType pt=nProcesses, int cc = -1, TString opt="", bool ratio=false);
  /**
   * \brief Adds a histogram to be drawn from a TFile
   * \param hname key of the histogram in the file
   * \param label the label to be used if a legend is drawn
   * \param pt the ProcessType (used to internally manage the histogram, set color and order, etc)
   * \param fname path to the file
   */
  void AddHistogram(TString hname, TString label, ProcessType pt=nProcesses, TString fname="", bool ratio=false);
  /**
   * \brief Similar to AddAdditional, but specifically for histograms to be treated as systematic errors
   * \param o a TH1D to draw
   * \param opt option to use when drawing
   * \param aname if provided and legend is created, this label is used in the legend
   */
  void AddSystematic(TH1D *o, TString opt="", TString aname="");
  void SetInputFile(TString fname); //!< Set the input file by name
  void SetInputFile(TFile *f); //!< Set the input file
  void SetAbsMin(double f) { absMin = f; } //!< Set the absolute minimum of the plot (by default 0)
  /**
   * \brief Reset the plot
   * \param clearPlotLabels if true, plot labels are reset too
   */
  void Reset(bool clearPlotLabels=true); 

  /**
   * \brief Draws the histograms and canvas
   * \param outDir output directory paty 
   * \param baseName baseName[.pdf,.png.,C] are created
   */
  virtual void Draw(TString outDir, TString baseName) override;

  protected:
    void BuildRatio(const TH1D* hNum, const TH1D* hDen, TH1D*& hRatio, TGraphErrors*& gRatio, bool denUnc=false);
    /**
     * \brief Internal class for managing histograms and their properties
     */
    struct HistWrapper {
        TH1D *h;
        TString label;
        ProcessType pt;
        int cc;
        TString opt;
        bool ratio;
    };
    bool fileIsOwned=false; //!< if the input file is owned and needs to be deleted
    TFile *centralFile=0; //!< an input file from which to harvest histograms by key
    std::vector<HistWrapper> internalHists; //!< collection of histograms to plot
    std::vector<HistWrapper> internalSysts; //!< Collection of systematic weights (subset of internalAdds)
    double absMin=-999; //!<absolute minimum of the plot
    bool padModified = false;
};

#endif
