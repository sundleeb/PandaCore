#include "../interface/PlotUtility.h"
#include "THStack.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TFile.h"
#include "TLatex.h"
#include "TSystem.h"
#include "TTreeFormula.h"
#include <math.h>
#include <iostream>
#include <fstream>

using namespace std;

TString sanitize(TString sIn) { 
  TString sOut = sIn; 
  return sOut.ReplaceAll(".","_").ReplaceAll("(","").ReplaceAll(")","").ReplaceAll("/","Over").ReplaceAll("*","Times").ReplaceAll("+","Plus");
}

PlotUtility::PlotUtility(double x, double y):
  HistogramDrawer(x,y)
{
  processes = new Process*[nProcesses];
  owned = new bool[nProcesses];
  for (unsigned int iP=0; iP!=nProcesses; ++iP) {
    processes[iP]=0;
    owned[iP]=false;
  }
}

PlotUtility::~PlotUtility() {
  for (unsigned int iP=0; iP!=nProcesses; ++iP) {
    if (owned[iP])
      delete processes[iP];
  }
  delete processes;
  delete owned;
}

void PlotUtility::AddProcess(Process *p) {
  ProcessType iP = p->processtype;
  if (owned[iP])
    delete processes[iP];
  owned[iP] = false;
  processes[iP] = p;
  order.push_back((unsigned int)iP);
}

void PlotUtility::AddFile(ProcessType pt,TString filePath,TString name) {
  if (processes[pt]==NULL) {
    owned[pt] = true;
    Process *tmp = new Process(name,pt);
    processes[pt] = tmp;
  }
  processes[pt]->AddFile(filePath);
}

void PlotUtility::ClearProcess(ProcessType pt) {
  if (owned[pt])
    delete processes[pt];
  processes[pt]=0;
  owned[pt] = false;
  order.clear(); // could probs implement this better
}

void PlotUtility::Reset(bool clearPlotLabels) {
  // overriding because there was something I wanted
  // to implement here, but I have forgotten what
  // that is
  HistogramDrawer::Reset(clearPlotLabels);
}

TString convertName(TString filename) {
  TString tmpName(filename);
  tmpName.ReplaceAll("/","Over");
  tmpName.ReplaceAll("*","Times");
  tmpName.ReplaceAll("+","Plus");
  tmpName.ReplaceAll("-","Minus");
  tmpName.ReplaceAll("TMath::","");
  tmpName.ReplaceAll(")","");
  tmpName.ReplaceAll("(","");
  tmpName.ReplaceAll(".","_");
  return tmpName; 
}

void divideBinWidth(TH1D *h) {
  // I trust this more than TH1::Scale(1,"width")
  int nBins = h->GetNbinsX();
  for (int iB=1; iB!=nBins+1; ++iB) {
    float currentVal = h->GetBinContent(iB);
    float currentErr = h->GetBinError(iB);
    float binWidth = h->GetBinWidth(iB);
    h->SetBinContent(iB,currentVal/binWidth);
    h->SetBinError(iB,currentErr/binWidth);
  }
}

void PlotUtility::DrawAll(TString outDir) {
  if (!legend)
    InitLegend();
  TFile *fOut = new TFile(outDir+"hists.root","UPDATE");
	if (fOut->IsZombie()) {
		fOut->Close();
		fOut = new TFile(outDir+"hists.root","RECREATE");
	}
	//TFile *fBuffer = new TFile(TString::Format("/tmp/buffer_%i.root",gSystem->GetPid()).Data(),"RECREATE");
  TFile *fBuffer = new TFile(TString::Format("/uscmst1b_scratch/lpc1/3DayLifetime/buffer_%i.root",gSystem->GetPid()).Data(),"RECREATE");
  fBuffer->cd();
  if (order.size()==0) {
    for (unsigned int iP=0; iP!=nProcesses; ++iP) 
      order.push_back(iP);
  }
  

  for (unsigned int iP : order) {
    Process *p = processes[iP];
    if (p==NULL)
      continue;

    for (TTree *t : p->trees) {
      t->SetBranchStatus("*",0);

      turnOnBranches(t,eventnumber);
      turnOnBranches(t,cut.GetTitle());
      if (p->processtype!=kData && p->useCommonWeight) {
        turnOnBranches(t,mcWeight.GetTitle());
      }
      for(Distribution *d : distributions) {
        turnOnBranches(t,d->name);
      }
      turnOnBranches(t,p->additionalCut.GetTitle());
      turnOnBranches(t,p->additionalWeight.GetTitle());
      for (unsigned int iS=0; iS!=systNames.size(); ++iS) {
        turnOnBranches(t,mcWeightUp[iS].GetTitle());
        turnOnBranches(t,mcWeightDown[iS].GetTitle());
			}
    }
  }

  map<Distribution*,PlotWrapper> pws;
  // first we declare all the necessary histograms
  for (Distribution *d: distributions) {
    PlotWrapper pw;
    TH1D *hbase=0;
    if (d->binEdges==0)
      hbase = new TH1D("hbase","hbase",d->nBins,d->min,d->max);
    else
      hbase = new TH1D("hbase","hbase",d->nBins,d->binEdges);
    // hbase->Sumw2();
    TString tmpname = convertName(d->filename);
    // these are the systematic shifts
    for (unsigned int iS=0; iS!=systNames.size(); ++iS) {
      TH1D *hsystup = (TH1D*)hbase->Clone(TString::Format("h_%s_%s_Up",
                                                          tmpname.Data(),systNames[iS].Data()));
      TH1D *hsystdown = (TH1D*)hbase->Clone(TString::Format("h_%s_%s_Down",
                                                          tmpname.Data(),systNames[iS].Data()));
      hsystup->Sumw2(); hsystdown->Sumw2();
      hsystup->SetLineColor(systColors[iS]); hsystup->SetLineWidth(2);
      hsystdown->SetLineColor(systColors[iS]); hsystdown->SetLineWidth(2);
      pw.hSystUp.push_back(hsystup);
      pw.hSystDown.push_back(hsystdown);
    }
    // one histogram for each process
    for (unsigned int iP : order) {
      Process *p = processes[iP];
      if (p==NULL)
        continue;
      TH1D *h = (TH1D*)hbase->Clone(TString::Format("h_%s_%s",tmpname.Data(),p->name.Data()));
      h->Sumw2();
      pw.histos[p] = h;
      pw.ownedHistos.push_back(h);
    }
    pws[d] = pw;
    hbase->Delete();
  }

  // now we loop through each process
  TimeReporter tr("PlotUtility::DrawAll",true);
  tr.Start();
  for (unsigned int iP : order) {
    Process *p = processes[iP];
    if (p==NULL)
      continue;
    // build the cut and weight formulae
    TCut finalCut = cut;
    TCut finalWeight = "1";
    if (p->processtype!=kData && p->useCommonWeight)
      finalWeight = mcWeight;
    if (p->processtype<=kSignal3 && p->processtype!=kData)
      finalWeight *= TCut(TString::Format("%f",signalScale));
    if (eventmod!=0) {
      if (p->processtype==kData)
        finalCut += TCut(TString::Format("(%s%%%i)==0",eventnumber.Data(),eventmod).Data());
      else
        finalWeight *= TCut(TString::Format("%f",1./eventmod).Data());
    }
    finalCut += p->additionalCut;
    finalWeight *= p->additionalWeight;

    // loop through each subprocess
    for (TTree *drawTree : p->trees) {
      TTreeFormula fcut(finalCut.GetTitle(),finalCut.GetTitle(),drawTree); fcut.SetQuickLoad(true); fcut.GetNdata();
      TTreeFormula fweight(finalWeight.GetTitle(),finalWeight.GetTitle(),drawTree); fweight.SetQuickLoad(true); fweight.GetNdata();

      // build the systematic shift weights
      vector<TTreeFormula*> fsystups, fsystdowns;
      vector<double> syst_up_weights(systNames.size());
      vector<double> syst_down_weights(systNames.size());
			if (p->processtype!=kData && p->useCommonWeight) {
				for (unsigned int iS=0; iS!=systNames.size(); ++iS) {
					TCut upWeight = mcWeightUp[iS];
					upWeight *= p->additionalWeight;
					if (eventmod!=0)
						upWeight *= TCut(TString::Format("%f",1./eventmod).Data());
					TTreeFormula *fup = new TTreeFormula(upWeight.GetTitle(),upWeight.GetTitle(),drawTree); fup->SetQuickLoad(true);
					fsystups.push_back(fup);

					TCut downWeight = mcWeightDown[iS];
					downWeight *= p->additionalWeight;
					if (eventmod!=0)
						downWeight *= TCut(TString::Format("%f",1./eventmod).Data());
					TTreeFormula *fdown = new TTreeFormula(downWeight.GetTitle(),downWeight.GetTitle(),drawTree); fdown->SetQuickLoad(true);
					fsystdowns.push_back(fdown);
				}
			} else {
				for (unsigned int iS=0; iS!=systNames.size(); ++iS) {
					fsystups.push_back(&fweight);
					fsystdowns.push_back(&fweight);
				}
			}
      
      // build the distribution formulae
      for (auto *d : distributions) {
        delete pws[d].tf;
        pws[d].tf = new TTreeFormula(d->name,d->name,drawTree);
        pws[d].tf->SetQuickLoad(true);
      }
      
      // cut, evaluate, and fill
      unsigned int nEntries = drawTree->GetEntries();
      for (unsigned int iE=0; iE!=nEntries; ++iE) {
        drawTree->GetEntry(iE);
        if (!fcut.EvalInstance())
          continue;
        double weight = fweight.EvalInstance();
				if (p->processtype!=kData) {
					for (unsigned int iS=0; iS!=systNames.size(); ++iS) {
						syst_up_weights[iS] = fsystups[iS]->EvalInstance();
						syst_down_weights[iS] = fsystdowns[iS]->EvalInstance();
					}
				}
        for (auto *d : distributions) {
          PlotWrapper &pw = pws[d];
          TH1D *h = pw.histos[p];
          double val = pw.tf->EvalInstance();
          double minX = d->min, maxX = d->max;
          if (doOverflow && val>maxX)
            val = h->GetBinCenter(d->nBins);
          else if (doUnderflow && val<minX)
            val = h->GetBinCenter(1);
          h->Fill(val,weight);
					if (p->processtype!=kData) {
						for (unsigned int iS=0; iS!=systNames.size(); ++iS) {
							pw.hSystUp.at(iS)->Fill(val,syst_up_weights[iS]);
							pw.hSystDown.at(iS)->Fill(val,syst_down_weights[iS]);
						}
					}
        }
      }

			if (p->processtype!=kData && p->useCommonWeight) {
				for (unsigned int iS=0; iS!=systNames.size(); ++iS) {
					delete fsystups[iS]; delete fsystdowns[iS];
					fsystups[iS]=0; fsystdowns[iS]=0;
				}
			}

    }
    tr.TriggerEvent(p->name.Data());
  }

  // now that all histograms are filled,
  // draw each plot
  for (Distribution *d : distributions) {
    PlotWrapper &pw = pws[d];

    bool doScaleBins = d->binEdges!=NULL;

    if (d->name=="1") {
      double bgTotal=0, bgErr=0;
      double sigTotal=0, sigErr=0;
      double dataTotal=0, dataErr=0;
      std::ofstream fyields; fyields.open(outDir+"yields.txt");

      TString stable = TString::Format("%-25s | %15s \\pm %15s","Process","Yield","Error");
      fyields << stable << std::endl;  PInfo("PlotUtility::Dump",stable);

      stable = "=================================================================";
      fyields << stable << std::endl;  PInfo("PlotUtility::Dump",stable);

      for (unsigned int iP : order) {
        Process *p = processes[iP];
        if (p==NULL)
          continue;
        TH1D *h = pw.histos[p];
        double integral=0,error=0;
        integral = h->IntegralAndError(1,h->GetNbinsX(),error);
        TString syield = TString::Format("%-25s | %15f \\pm %15f",p->name.Data(),integral,error);
        fyields << syield.Data() << std::endl;
        PInfo("PlotUtility::Dump",syield);
        if (p->processtype==kData) {
          dataTotal=integral; dataErr=error;
        } else if (p->processtype<=kSignal3) {
          sigTotal += integral;
          sigErr += pow(error,2); 
        } else {
          bgTotal += integral;
          bgErr += pow(error,2);
        }
      }
      fyields << stable << std::endl;  PInfo("PlotUtility::Dump",stable);

      sigErr = sqrt(sigErr);
      bgErr = sqrt(bgErr);
      
      TString syield = TString::Format("%-25s | %15f \\pm %15f","MC(bkg)",bgTotal,bgErr);
      fyields << syield.Data() << std::endl;      PInfo("PlotUtility::Dump",syield);
      syield = TString::Format("%-25s | %15f \\pm %15f","MC(sig)",sigTotal,sigErr);
      fyields << syield.Data() << std::endl;      PInfo("PlotUtility::Dump",syield);
      syield = TString::Format("%-25s | %15f \\pm %15f","Data",dataTotal,dataErr);
      fyields << syield.Data() << std::endl;      PInfo("PlotUtility::Dump",syield);
      fyields << stable << std::endl;             PInfo("PlotUtility::Dump",stable);

      syield = TString::Format("S/B=%.3f, S/sqrtB=%.3f",sigTotal/bgTotal,sigTotal/sqrt(bgTotal));
      fyields << syield.Data() << std::endl; PInfo("PlotUtility::Dump",syield);

      fyields.close();
    } 
    for (unsigned int iP : order) {
      Process *p = processes[iP];
      if (p==NULL)
        continue;
      TH1D *h = pw.histos[p];
      if (doScaleBins)
        divideBinWidth(h);
      if (d->maxY>-999)
        h->SetMaximum(d->maxY);
      if (d->minY<999)
        h->SetMinimum(d->minY);
      h->GetXaxis()->SetTitle(d->xLabel);
      h->GetYaxis()->SetTitle(d->yLabel);
      h->SetTitle("");
      if (!doStack && p->processtype!=kData)
        h->SetLineWidth(2);
      if (p->processtype<=kSignal3 && p->processtype!=kData && signalScale!=1)
        AddHistogram(h,TString::Format("%.1f#times%s",signalScale,p->name.Data()).Data(),p->processtype);
      else  
        AddHistogram(h,p->name.Data(),p->processtype);  
    }
    for (unsigned int iS=0; iS!=systNames.size(); ++iS) {
      if (doScaleBins) {
        divideBinWidth(pw.hSystUp[iS]);
        divideBinWidth(pw.hSystDown[iS]);
      }
      AddSystematic(pw.hSystUp[iS],"hist",systNames[iS]);
      AddSystematic(pw.hSystDown[iS],"hist");
    }
    TString tmpname = convertName(d->filename);
    Logy(false);
    HistogramDrawer::Draw(outDir,tmpname);
    tmpname += "_logy";
    legend->Clear();
    Logy(true);
    HistogramDrawer::Draw(outDir,tmpname);
    Reset(false);
    for (auto h : pw.ownedHistos) {
      fOut->WriteTObject(h);
      delete h;
    }
    for (auto h : pw.hSystUp) {
      fOut->WriteTObject(h);
      delete h;
    }
    for (auto h : pw.hSystDown) {
      fOut->WriteTObject(h);
      delete h;
    }
    pw.ownedHistos.clear();
    pw.hSystUp.clear();
    pw.hSystDown.clear();
    delete pw.tf; pw.tf=0;
  }

  distributions.clear();
  fOut->Close(); // do not write or cloned trees will get saved
  fBuffer->Close();
  delete fBuffer;
  delete fOut;

  
} 
