#ifndef PANDACORE_TOOLS_NORMALIZER
#define PANDACORE_TOOLS_NORMALIZER

#include "TTree.h"
#include "TFile.h"
#include "TBranch.h"
#include "TString.h"
#include "TH1F.h"
#include "Common.h"
#include "TreeTools.h"

/**
 * \brief Normalizes a tree
 *
 * Given an input weight, class adds an output weight
 * that is calculated as in_weight*xsec/total_events
 */
class Normalizer
{
public:
    Normalizer() { }
    ~Normalizer() { }

    /**
     * \param t input tree
     * \param totalEvents total number of events for this process
     * \param xsec cross-section for this process
     * \brief Normalizes the tree given total weight of events and 
     * cross-section
     */
    void NormalizeTree(TTree *t, double totalEvts, double xsec);

    /**
     * \param fpath path to input file 
     * \param xsec cross-section for this process
     * \brief Reads an input file and picks up the tree to normalize
     * as well as a histogram containing the weight of events
     */
    void NormalizeTree(TString fpath, double xsec);

    TString inWeightName = "mcWeight"; /**< name of input branch */
    TString outWeightName = "normalizedWeight"; /**< name of output branch */
    TString histName = "hDTotalMCWeight"; /**< name of histogram containing MC weights */
    TString treeName = "events"; /**< name of input tree */
    bool isFloat = true;
};
#endif
