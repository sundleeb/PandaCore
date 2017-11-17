#ifndef PANDACORE_LEARNING_TMVATrainer
#define PANDACORE_LEARNING_TMVATrainer

#include "TFile.h"
#include "TTree.h"
#include "TCut.h"
#include "TString.h"
#include "TTreeFormula.h"
#include "TMVA/Tools.h"
#include "TMVA/Factory.h"
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
#include "TMVA/DataLoader.h"
#endif
#include "vector"
#include "PandaCore/Tools/interface/Common.h"
#include "PandaCore/Tools/interface/TreeTools.h"

/**
 * \brief Wrapper around TMVA::Factory
 *
 * Contains some predefined options for training MVAs.
 */
class TMVATrainer
{
public:
  /**
   * \brief Pre-defined BDT options
   */
  enum BDTType {
    kAda, /**< ada-boost with bagging */
    kGradWide, /**< many relatively shallow trees, gradient boosting */
    kGradDeep /**< few deep trees, gradient boosting */
  };
  /**
   * \brief Constructor
   * \param name name of MVA
   * \param workdir_ location of outputs
   */
  TMVATrainer(TString name, TString workdir_); 
  ~TMVATrainer();

  /**
   * \brief Adds an input variable
   * \param v branch or formula from input tree
   * \param t can be 'f' or 'i' for float or int
   * \param title optional name for variable 
   */
  void AddVariable(TString v, char t, TString title=""); 
  /**
   * \brief Adds a spectator variable
   * \param v branch or formula from input tree
   * \param t can be 'f' or 'i' for float or int
   * \param title optional name for variable 
   */
  void AddSpectator(TString v, char t, TString title="");
  void SetFiles(TString sigpath, TString bgpath); //!< Sets input signal and background files
  void BookBDT(BDTType t); //!< Book a BDT given a pre-defined type
  void BookBDT(TString opt=""); //!< Book a BDT given a user-defined set of options
  void BookCuts(TString opt=""); //!< Book a rectangular cut optimizer
  void TrainAll(); //!< Trian all booked methods

  TString treename=""; /**< name of tree to load from input files */
  TString sigcut="", bgcut=""; /**< cuts to be applied to signal and background trees */
  TString sigweight="", bgweight=""; /**< weights to be applied to signal and background events */
private:
  TMVA::Factory *factory=0; /**< the real workhorse, used to book and train methods */
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,8,0)
  TMVA::DataLoader *dataloader=0; /**< the real workhorse #2 */
#else
  TMVA::Factory *dataloader=factory; /**< the real workhorse, used to book and train methods */
#endif
  TFile *sigfile=0, *bgfile=0; /**<  pointers to the input files */
  TTree *sigtree=0, *bgtree=0; /**< input trees */
  TFile *outfile; /**< pointer to output files, owned by this object */
  TString workdir; /**< directory to which outputs should go */
   
};
#endif
