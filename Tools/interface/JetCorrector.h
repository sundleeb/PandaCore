#ifndef PANDACORE_TOOLS_JetCorrector
#define PANDACORE_TOOLS_JetCorrector

#include "Common.h"
#include <map>
#include <string>
#include "TString.h"
#include "PandaUtilities/Common/interface/DataTools.h"
#include "PandaProd/Objects/interface/PJet.h"
#include "PandaProd/Objects/interface/PMET.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "CondFormats/JetMETObjects/interface/FactorizedJetCorrector.h"

/**
 * \brief Corrects a jet collection and optionally propagates to MET 
 *
 */
class JetCorrector
{
public:
	JetCorrector();
	~JetCorrector();

	void RunCorrection(bool isData, panda::VJet *injets_, panda::PMET *inmet_=0, int runNumber = 0);
	panda::VJet *GetCorrectedJets();
	panda::PMET *GetCorrectedMET();

	void SetMCCorrector(TString fpath);
	void SetDataCorrector(TString fpath, TString iov = "all");

private:
		FactorizedJetCorrector *mMCJetCorrector;	 
		std::map<TString,FactorizedJetCorrector *> mDataJetCorrectors;	// map from era to corrector

		panda::VJet *outjets = 0;
		panda::PMET *outmet = 0;

		EraHandler *era = 0;	
};
#endif
