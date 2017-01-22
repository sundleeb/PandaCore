#include "../interface/JERReader.h"
#include "TMath.h"
#include "iostream"
#include "fstream"

JERReader::JERReader(TString sfPath, TString resPath)
{
	// first load the sf file
	std::ifstream fSF; 
	fSF.open(sfPath.Data());

	float eta0=0, eta1=1, sfcent=1, sflo=1, sfhi=1;

	TString header = "";
	while (!header.EndsWith("}"))
		fSF >> header;

	std::vector<double> sfEtaBounds;
	while(!fSF.eof()) {
		fSF >> eta0 >> eta1 >> header >> sfcent >> sflo >> sfhi;
		sf_central.push_back(sfcent);
		sf_up.push_back(sfhi);
		sf_down.push_back(sflo);
		if (sfEtaBounds.size()==0) 
			sfEtaBounds.push_back(eta0);
		sfEtaBounds.push_back(eta1);
	}
	n_sfEta = sfEtaBounds.size()-1;

	bins_sfEta = new Binner(sfEtaBounds);
	fSF.close();

	// now load the resolution file
	std::ifstream fRes;
	fRes.open(resPath.Data());

	float rho0=0, rho1=1, pt0=0, pt1=1, p0=0, p1=1, p2=2, p3=3;
	float last_eta=-999, last_rho=-999;

	header = "";
	while (!header.EndsWith("}"))
		fRes >> header;

	std::vector<double> resEtaBounds, resRhoBounds;
	while (!fRes.eof()) {
		fRes >> eta0 >> eta1 >> rho0 >> rho1;
		fRes >> header;
		fRes >> pt0 >> pt1;
		fRes >> p0 >> p1 >> p2 >> p3;

		res_ptLo.push_back(pt0); res_ptHi.push_back(pt1);
		res_param0.push_back(p0);
		res_param1.push_back(p1);
		res_param2.push_back(p2);
		res_param3.push_back(p3);

		if (resEtaBounds.size()==0) {
			resEtaBounds.push_back(eta0);
			last_eta = eta0;
		}
		if (resRhoBounds.size()==0) {
			resRhoBounds.push_back(rho0);
			last_rho = rho0;
		}

		if (eta1!=last_eta) {
			resEtaBounds.push_back(eta1);
			last_eta = eta1;
		}
		if (rho1>last_rho) {
			resRhoBounds.push_back(rho1);
			last_rho = rho1;
		}
	}
	n_resEta = resEtaBounds.size()-1;
	n_resRho = resRhoBounds.size()-1;

	bins_resEta = new Binner(resEtaBounds);
	bins_resRho = new Binner(resRhoBounds);
	fRes.close();

}

void JERReader::getStochasticSmear(double pt, double eta, double rho, 
																	 double &smear, double &smearUp, double &smearDown) 
{
	//PDebug("JERReader::getStochasticSmear",
	//		TString::Format("pT=%f, eta=%f, rho=%f",pt,eta,rho));
	// first determine the scale factor
	unsigned idxSFEta = bins_sfEta->bin(eta);
	float sf_val = sf_central.at(idxSFEta);
	float sf_valUp = sf_up.at(idxSFEta);
	float sf_valDown = sf_down.at(idxSFEta);
	//PDebug("JERReader::getStochasticSmear",
	//		TString::Format("sf=%f, sfUp=%f, sfDown=%f",sf_val,sf_valUp,sf_valDown));

	// now get the resolution parameters
	unsigned idxResEta = bins_resEta->bin(eta);
	unsigned idxResRho = bins_resRho->bin(rho);
	unsigned idxRes = idxResRho + n_resRho*idxResEta;
	fres.SetParameter(0,res_param0[idxRes]);
	fres.SetParameter(1,res_param1[idxRes]);
	fres.SetParameter(2,res_param2[idxRes]);
	fres.SetParameter(3,res_param3[idxRes]);
	//PDebug("JERReader::getStochasticSmear",
	//		TString::Format("%u=%u+%i*%u",idxRes,idxResRho,n_resRho,idxResEta));
	//PDebug("JERReader::getStochasticSmear",
	//		TString::Format("p0=%f, p1=%f, p2=%f",res_param0[idxRes],res_param1[idxRes],res_param2[idxRes]));

	float pt_bound = bound(pt,res_ptLo[idxRes],res_ptHi[idxRes]);
	float sigma_res = fres.Eval(pt_bound);
	//PDebug("JERReader::getStochasticSmear",
	//		TString::Format("sigma(%f) = %f",pt_bound,sigma_res));

	double sf_scale = TMath::Sqrt( TMath::Max( pow(sf_val,2)-1 , (double)0. ) );
	double sf_scaleUp = TMath::Sqrt( TMath::Max( pow(sf_valUp,2)-1 , (double)0. ) );
	double sf_scaleDown = TMath::Sqrt( TMath::Max( pow(sf_valDown,2)-1 , (double)0. ) );
	//PDebug("JERReader::getStochasticSmear",
	//		TString::Format("scale=%f, scaleUp=%f, scaleDown=%f",sf_scale,sf_scaleUp,sf_scaleDown));

	float toy = rng.Gaus(0,sigma_res);

	smear = 1 + toy*sf_scale; 
	smearUp = 1 + toy*sf_scaleUp; 
	smearDown = 1 + toy*sf_scaleDown; 

	//PDebug("JERReader::getStochasticSmear",
	//		TString::Format("smear=%f, smearUp=%f, smearDown=%f",smear,smearUp,smearDown));
	return;
}

