#include "../interface/JetCorrector.h"
#include "TLorentzVector.h"

using namespace panda;

JetCorrector::JetCorrector() 
{ 
	era = new EraHandler(2016);
}


JetCorrector::~JetCorrector()
{
	delete era;
//	delete outjets;
//	delete outmet;
	delete mMCJetCorrector;
	for (auto& iter : mDataJetCorrectors)
		delete iter.second;
}

void JetCorrector::SetMCCorrector(TString fpath)
{
	delete mMCJetCorrector;
	std::vector<TString> levels = {"L1FastJet","L2Relative","L3Absolute","L2L3Residual"};
	std::vector<JetCorrectorParameters> params;
	for (auto &level : levels) {
		params.push_back(
				JetCorrectorParameters(
					TString::Format(fpath,level.Data()).Data()
					)
				);
	}
	mMCJetCorrector = new FactorizedJetCorrector(params);
}

void JetCorrector::SetDataCorrector(TString fpath, TString iov)
{
	std::vector<TString> levels = {"L1FastJet","L2Relative","L3Absolute","L2L3Residual"};
	std::vector<JetCorrectorParameters> params;
	for (auto &level : levels) {
		params.push_back(
				JetCorrectorParameters(
					TString::Format(fpath,level.Data()).Data()
					)
				);
	}
	mDataJetCorrectors[iov] = new FactorizedJetCorrector(params);
}

void JetCorrector::RunCorrection(bool isData, VJet *injets_, PMET *inmet_, int runNumber)
{
	FactorizedJetCorrector *corrector=0;
	if (isData) {
		if (mDataJetCorrectors.find("all") != mDataJetCorrectors.end()) {
			// we have an era-independent corrector. use it
			corrector = mDataJetCorrectors["all"];
		} else {
			TString thisEra = era->getEra(runNumber);
			TString thisEraGroup;
			for (auto &iter : mDataJetCorrectors) {
				if (iter.first.Contains(thisEra)) {
					thisEraGroup = iter.first;
					corrector = iter.second;
					break;
				}
			}
		}
	} else {
		corrector = mMCJetCorrector;
	}
	if (corrector==0) {
		PError("JetCorrector::RunCorrection",
				TString::Format("Could not determine data era for run %i",runNumber)
				);
		assert(corrector!=0);
	}

	TLorentzVector v_outmet;
	if (inmet_) {
		v_outmet.SetPtEtaPhiM(inmet_->raw_pt,0,inmet_->raw_phi,0);
		outmet = new PMET();
	}

	outjets = new VJet();

	TLorentzVector v_j_in, v_j_out;
	for (auto *j_in : *injets_) {
		double jecFactor = 1;
		v_j_in.SetPtEtaPhiM(j_in->rawPt,j_in->eta,j_in->phi,j_in->m);
		if (fabs(j_in->eta)<5.191) {
			corrector->setJetPt(j_in->rawPt);
			corrector->setJetEta(j_in->eta);
			corrector->setJetPhi(j_in->phi);
			corrector->setJetE(v_j_in.E());
			corrector->setRho(0);
			corrector->setJetA(0);
			corrector->setJetEMF(-99);
			jecFactor = corrector->getCorrection();
		}
		v_j_out.SetPtEtaPhiM(jecFactor*j_in->rawPt,j_in->eta,j_in->phi,j_in->m);
		
		PJet *j_out = new PJet();
		j_out->pt = jecFactor*j_in->rawPt;
		j_out->rawPt = j_in->rawPt;
		j_out->eta = j_in->eta;
		j_out->phi = j_in->phi;
		j_out->m = j_in->m;

		if (inmet_) {
			TLorentzVector v_diff = v_j_in - v_j_out;
			v_outmet += v_diff;
		}
	}

	if (inmet_) {
		outmet->raw_pt = inmet_->raw_pt;
		outmet->raw_phi = inmet_->raw_phi;
		outmet->pt = v_outmet.Pt();
		outmet->phi = v_outmet.Phi();
	}
}

panda::VJet *JetCorrector::GetCorrectedJets() { return outjets; outjets = 0; }

panda::PMET *JetCorrector::GetCorrectedMET() { return outmet; outmet = 0; }
