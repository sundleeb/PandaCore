#ifndef PANDACORE_TOOLS_BranchAdder
#define PANDACORE_TOOLS_BranchAdder

#include "TTree.h"
#include "TFile.h"
#include "TBranch.h"
#include "TString.h"
#include "TTreeFormula.h"
#include "Common.h"
#include "TreeTools.h"

/**
 * \brief Adds a branch to a tree
 *
 * Can be used to add a branch to a tree based on a generic formula
 * or based on a histogram
 */
class BranchAdder
{
public:
	BranchAdder() { }
	~BranchAdder() { }

	/**
	 * \param t tree to modify
	 * \brief Adds a branch based on a generic formula
	 *
	 * formula should be set before this is called
	 */
	void AddBranchFromFormula(TTree *t);

	/**
	 * \param f path to file containing tree to modify
	 * \brief Adds a branch based on a generic formula
	 *
	 * formula should be set before this is called
	 */
	void AddBranchFromFormula(TString fpath);

	/**
	 * \param t tree to modify
	 * \param h histogram containing binned values
	 * \brief Adds a branch based on a histogram
	 */
	void AddBranchFromHistogram(TTree *t, TH1 *h);

	/**
	 * \param f path to file containing tree to modify
	 * \param h histogram containing binned values
	 * \brief Adds a branch based on a histogram
	 */
	void AddBranchFromHistogram(TString fpath, TH1 *h);

	/**
	 * \param t tree to modify
	 * \param h histogram containing binned values
	 * \brief Adds a branch based on a histogram
	 */
	void AddBranchFromHistogram2D(TTree *t, TH2 *h);

	/**
	 * \param f path to file containing tree to modify
	 * \param h histogram containing binned values
	 * \brief Adds a branch based on a histogram
	 */
	void AddBranchFromHistogram2D(TString fpath, TH2 *h);

	TString formula = ""; /**< formula to be compiled on input tree */
	TString formulaY = ""; /**< formula to be compiled on input tree if 2D */
	TString newBranchName = ""; /**< name of output branch */
	TString treeName = "events"; /**< name of input tree, if file path is provided */
	double defaultValue = 1; /**< Default value */
	TString cut = "1==1";
	bool verbose = true;
	
};
#endif
