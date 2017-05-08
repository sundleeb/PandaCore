#include "../interface/EventSyncher.h"
#include "../interface/TreeTools.h"

#include "TFile.h"
#include "TTreeFormula.h"
#include "TString.h"
#include "TH2D.h"

void EventSyncher::RunFiles(std::string fpath1, std::string fpath2, std::string treeName, int maxEntries) 
{
	TFile *f1 = TFile::Open(fpath1.c_str());
	TFile *f2 = TFile::Open(fpath2.c_str());

	TTree *t1 = (TTree*) f1->FindObjectAny(treeName.c_str());
	TTree *t2 = (TTree*) f2->FindObjectAny(treeName.c_str());

	RunTrees(t1,t2,maxEntries);

	f1->Close(); f2->Close();
}

void EventSyncher::RunTrees(TTree *t1, TTree *t2, int maxEntries) 
{

	BuildMap(t1,1,maxEntries);
	BuildMap(t2,2,maxEntries);	

}

/**
 * This function builds a map:event->properties that is later used
 * to compare events
 */
void EventSyncher::BuildMap(TTree *t, int which, int maxEntries) {
	auto *m = (which==1) ? &map1 : &map2; // choose a map

	// reduce reading load
	std::vector<std::string> extradeps = {runName, lumiName, eventName, preselection};
	t->SetBranchStatus("*",0);
	for (auto d : extradeps)
		turnOnBranches(t,d.c_str());
	for (auto d : formulae)
		turnOnBranches(t,d.c_str());


	// event identifiers
	int run, lumi;
	ULong64_t event;
	t->SetBranchAddress(runName.c_str(),&run);
	t->SetBranchAddress(lumiName.c_str(),&lumi);
	t->SetBranchAddress(eventName.c_str(),&event);
	

	// formulae to evaluate
	TTreeFormula fpresel(preselection.c_str(),preselection.c_str(),t); fpresel.SetQuickLoad(true);
	std::vector<TTreeFormula *> tformulae;
	unsigned nF = formulae.size();
	for (unsigned iF=0; iF!=nF; ++iF) {
		TTreeFormula *tf = new TTreeFormula(names[iF].c_str(),formulae[iF].c_str(),t);
		tf->SetQuickLoad(true);
		tformulae.push_back(tf);
	}

	// set up loop
	std::vector<float> vals(nF);
	unsigned nEntries = (maxEntries<0) ? t->GetEntriesFast() : maxEntries;
	unsigned iE = 0;
	ProgressReporter pr(TString::Format("EventSyncher::BuildMap (%i)",which).Data(),
			                &iE, &nEntries, 10);


	for (iE=0; iE!=nEntries; ++iE) {
		pr.Report();

		t->GetEntry(iE);

		if (!(fpresel.EvalInstance()))
			continue;

		EventObj eo;
		eo.run = run;
		eo.lumi = lumi;
		eo.evt = event;

		for (unsigned iF=0; iF!=nF; ++iF) {
			float val = tformulae[iF]->EvalInstance();
			vals[iF] = val;
		}

		(*m)[eo] = vals;

	}

	for (auto *tf : tformulae)
		delete tf;
	tformulae.clear();

}

std::vector<TH2D*> EventSyncher::PlayViolin(float range) 
{
	unsigned nF = formulae.size();
	std::vector<TH2D*> hists;
	hists.reserve(nF);
	for (unsigned iF=0; iF!=nF; ++iF) {
		hists.push_back( new TH2D(("h"+names[iF]).c_str(),"",25,los[iF],his[iF],100,-1*range,range) );
		hists[iF]->GetXaxis()->SetTitle(names[iF].c_str());
		hists[iF]->GetYaxis()->SetTitle("(t1-t2)/t1");
		hists[iF]->SetLineColor(15);
		hists[iF]->SetFillColor(15);
		hists[iF]->SetMarkerStyle(20);
	}

	// loop through first map, find corresponding events in second map
	for (auto e1 : map1) {
		auto e2 = map2.find(e1.first);
		if (e2==map2.end())
			continue;
		std::vector<float> &v1 = e1.second;
		std::vector<float> &v2 = e2->second;
		for (unsigned iF=0; iF!=nF; ++iF) {
			float val1 = v1[iF], val2 = v2[iF];
			float pull = val1 - val2;
			if (relatives[iF])
				pull /= val1;
			hists[iF]->Fill(val1,pull);
		}
	}

	return hists;

}
