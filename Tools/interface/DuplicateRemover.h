#ifndef PANDACORE_TOOLS_DUPLICATEREMOVER
#define PANDACORE_TOOLS_DUPLICATEREMOVER

#include "TTree.h"
#include "TFile.h"
#include "TString.h"
#include "TH1F.h"
#include "TEntryList.h"
#include "TList.h"
#include "Common.h"
#include <unordered_set>

/**
 * \brief Removes duplicate entries given two input trees
 */
class DuplicateRemover
{
public:
	DuplicateRemover() { }
	~DuplicateRemover() { }

	/*
	 * \brief Merges two input trees and puts them into a new file
	 *
	 * First checks all events in t1, removes them from t2, and 
	 * merges the trees into a single tree in fOutPath. Note that
	 * both input trees are left alone.
	 */
	void Merge(TTree *t1, TTree *t2, TString fOutPath);

	/*
	 * \brief Merges two input trees given the files that contain them
	 */
	void Merge(TString f1Path, TString f2Path, TString fOutPath);

	bool verbose=false; /**< verbosity */
	TString runName = "runNumber"; /**< name of run branch */
	TString eventName = "eventNumber"; /**< name of event branch */
	TString lumiName = "lumiNumber"; /**< name of lumi branch */
	TString treeName = "events"; /**< name of input and output trees */
};
#endif
