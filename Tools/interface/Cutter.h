#ifndef PANDACORE_TOOLS_Cutter
#define PANDACORE_TOOLS_Cutter

#include "TTree.h"
#include "TFile.h"
#include "TBranch.h"
#include "TString.h"
#include "TH1F.h"
#include "Common.h"

/**
 * \brief Just a wrapper around TTree::CopyTree
 */
class Cutter
{
public:
	Cutter() { }
	~Cutter() { }

	/**
	 * \param t input tree
	 * \param cut cut to apply
	 * \brief Applies cut to a tree
	 */
	TTree *CutTree(TTree *t,TString cut);
	/**
	 * \param finpath input file path
	 * \param foutpath output file path
	 * \param cut to apply
	 * \brief Applies cut to a tree in a file
	 */
	void Cut(TString finpath, TString foutpath, TString cut);

	TString treeName = "events"; /**< name of tree if filepath is provided */
};
#endif
