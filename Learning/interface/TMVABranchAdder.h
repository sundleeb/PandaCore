#ifndef PANDACORE_LEARNING_TMVABranchAdder
#define PANDACORE_LEARNING_TMVABranchAdder

#include "TFile.h"
#include "TTree.h"
#include "TCut.h"
#include "TString.h"
#include "TTreeFormula.h"
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"
#include "vector"
#include "PandaCore/Tools/interface/Common.h"
#include "PandaCore/Tools/interface/TreeTools.h"

/**
 * \brief Parses a TMVA method and applies it to a tree
 *
 * Simple way to apply a TMVA method to a tree post-hoc.
 */
class TMVABranchAdder
{
public:
  /**
   * \brief Used internally to organize a variable
   */
  class xvar {
    public:
      xvar(TString n_, TString f_) { name=n_; formula=f_; val = new float(0); }
      ~xvar() { delete val;  }
      TString name; /**< name of this variable ("title") */
      TString formula; /**< formula, to be compiled from the tree */
      float *val; /**< address of value, owned by this object */
  };

  /**
   * \brief Constructor
   * \param bookExtraVar if true, reader will add event and run as spectators
   */
  TMVABranchAdder(bool bookExtraVars=true):
    spectator(0)
  { 
    reader = new TMVA::Reader("!Silent"); 
    if (bookExtraVars) {
      reader->AddSpectator("eventNumber",&spectator);
      reader->AddSpectator("runNumber",&spectator);
    }
  } 

  /**
   * \brief Destructor
   */
  ~TMVABranchAdder() { 
    delete reader; 
    for (auto *x : variables)
      delete x;
    for (auto *x : formulae)
      delete x;
  }

  /**
   * \brief Adds an input variable
   * \param vname name of variable in training
   * \param formula branch name of variable in tree
   */
  void AddVariable(TString vname, TString formula);
  /**
   * \brief Adds an input formula
   * \param vname name of variable in training
   * \param formula formula to be compiled from input tree 
   */
  void AddFormula(TString vname, TString formula);
  void AddSpectator(TString vname); //!< adds a dummy spectator to be TMVA-compliant
  void BookMVA(TString branchname,TString fpath); //!< books a TMVA object
  void RunFile(TString fpath); //!< Processes a file and adds a new branch containing the MVAs output

  TString treename=""; /**< name of input tree */
  TString presel=""; /**< a preselection on input events, if blank then no presel is applied */
  float defaultValue=-2; /**< default value to be used if failing presel or if output is NaN */
private:
  TMVA::Reader *reader=0; /**< the underlying reader */
  std::vector<xvar*> variables; /**< variables that can be directly read from the tree*/
  std::vector<xvar*> formulae; /**< variables that must be compiled from the tree */
  std::vector<TString> bnames; /**< names of output branches */
  float spectator=0; /**< dummy spectator member */
  TFile *fin; /**< file to process */
  TTree *tin; /** tree to process */
};
#endif
