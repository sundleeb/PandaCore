#include "Common.h"
#include "TChain.h"

#ifndef TREETOOLS_H
#define TREETOOLS_H

/** \file TreeTools.h
 * \brief Common functions for dealing with TTrees 
 */

//////////////////////////////////////////////////////////////////////////////////

/**
 * \param t input tree
 * \param s formula containing dependencies
 * \brief Figures out any substrings of s that are branch dependencies
 * and sets the status of those branches to 1
 */
inline void turnOnBranches(TTree*t,TString s) {
  std::vector<TString> deps = getDependencies(s);
  for (auto d : deps) {
    TBranch *b = (TBranch*)t->GetListOfBranches()->FindObject(d);
    if (b)
      t->SetBranchStatus(d,1);
  }
}

/**
 * \param t input chain
 * \param s formula containing dependencies
 * \brief Figures out any substrings of s that are branch dependencies
 * and sets the status of those branches to 1. This is meant for
 * TChains becacuse SetBranchStatus behaves differently
 */
inline void turnOnBranchesChain(TChain*t,TString s) {
  std::vector<TString> deps = getDependencies(s);
  for (auto d : deps) {
    TBranch *b = (TBranch*)t->GetListOfBranches()->FindObject(d);
    if (b)
      t->SetBranchStatus(d,1);
  }
}

//////////////////////////////////////////////////////////////////////////////////

#endif
