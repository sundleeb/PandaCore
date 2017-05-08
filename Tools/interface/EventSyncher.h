#ifndef PANDACORE_TOOLS_EventSyncher
#define PANDACORE_TOOLS_EventSyncher

#include "Common.h"
#include <unordered_map>
#include <string>
#include <vector>
#include "TTree.h"
#include "TFile.h"

class EventSyncher
{
public:
	EventSyncher() { }
	~EventSyncher() { }

	void RunFiles(std::string fpath1, std::string fpath2, std::string treeName="events", int maxEntries=-1);
	void RunTrees(TTree *t1, TTree *t2, int maxEntries=-1);
	void AddFormula(std::string f, float lo, float hi, std::string n="", bool relative=true) {
		formulae.push_back(f);
		if (n.size()==0)
			names.push_back(f);
		else
			names.push_back(n);
		los.push_back(lo);
		his.push_back(hi);
		relatives.push_back(relative);
	}
	std::vector<TH2D*> PlayViolin(float range=1);

	std::string runName="runNumber";
	std::string lumiName="lumiNumber";
	std::string eventName="eventNumber";
	std::string preselection = "1==1";

private:
	void BuildMap(TTree *t, int which, int maxEntries);

	std::unordered_map<EventObj,std::vector<float>> map1;
	std::unordered_map<EventObj,std::vector<float>> map2;

	std::vector<std::string> formulae;
	std::vector<std::string> names;

	std::vector<float> los, his;

	std::vector<float> relatives;
};
#endif
