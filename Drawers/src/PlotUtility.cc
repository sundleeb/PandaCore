#include "../interface/PlotUtility.h"
#include "THStack.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TFile.h"
#include "TLatex.h"
#include "TSystem.h"
#include <math.h>

std::vector<TString> GetDependencies(TString cut) {
  std::vector<TString> deps;
  int nChars = cut.Length();
  TString tmpString="";
  for (int iC=0; iC!=nChars; ++iC) {
    const char c = cut[iC];
    if ( c=='&' || c=='|' || c=='(' || c==')' 
        || c=='*' || c=='+' || c=='-' || c=='/' 
        || c=='<' || c=='>' || c=='=' || c=='.' ) {
      if (tmpString != "" && !tmpString.IsDigit() && 
          tmpString!="Pt" && tmpString!="Eta" && tmpString!="Phi" &&
          !tmpString.Contains("TMath")) {
        deps.push_back(tmpString);
      }
      tmpString = "";
    } else {
        tmpString.Append(c);
    }
  }
  return deps;
}

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
  HistogramDrawer::Reset(clearPlotLabels);
}

void PlotUtility::DrawAll(TString outDir) {
  if (!legend)
    InitLegend();
  TFile *fOut = new TFile(outDir+"hists.root","UPDATE");
  TFile *fBuffer = new TFile(TString::Format("/tmp/buffer_%i.root",gSystem->GetPid()).Data(),"RECREATE");
  fBuffer->cd();
  TH1F *h=0;
  std::vector<TH1F> histograms;
  if (order.size()==0) {
    for (unsigned int iP=0; iP!=nProcesses; ++iP) 
      order.push_back(iP);
  }

  
  std::vector<TString> deps;
  for (TString dep : GetDependencies(cut.GetTitle()))
    deps.push_back(dep);
  for (TString dep : GetDependencies(mcWeight.GetTitle()))
    deps.push_back(dep);
  for(Distribution *d : distributions) {
    for (TString dep : GetDependencies(d->name))
      deps.push_back(dep);
  }

  for (unsigned int iP : order) {
    Process *p = processes[iP];
    if (p==NULL)
      continue;

    std::vector<TString> pDeps;
/*
    p->chain->Scan("metfilter");
    for (TString dep : GetDependencies(p->additionalCut.GetTitle()))
      pDeps.push_back(dep);
    for (TString dep : GetDependencies(p->additionalWeight.GetTitle()))
      pDeps.push_back(dep);
    p->chain->SetBranchStatus("*",0);
    for (TString dep : deps)
      p->chain->SetBranchStatus(dep.Data(),1);
    for (TString dep : pDeps)
      p->chain->SetBranchStatus(dep.Data(),1);
*/

    TCut pCut(cut);
    pCut += p->additionalCut;
    if (cloneTrees) {
      printf("Cloning %s\n",p->name.Data());
      p->clonedTree = (TTree*)p->chain->CopyTree(pCut.GetTitle());
    } else {
      p->clonedTree = 0;
    }
  }


  for (Distribution *d: distributions) {
    if (d->name!="1")
      printf("Plotting %s\n",d->name.Data());
    
    float bgTotal=0;
    float sigTotal=0;

    
    TString tmpName(d->filename);
    tmpName.ReplaceAll("/","Over");
    tmpName.ReplaceAll("*","Times");
    tmpName.ReplaceAll("+","Plus");
    tmpName.ReplaceAll("-","Minus");
    tmpName.ReplaceAll("TMath::","");
    tmpName.ReplaceAll(")","");
    tmpName.ReplaceAll("(","");
    tmpName.ReplaceAll(".","_");

    std::vector<TH1F*> ownedHistos; // for garbage collection
    std::vector<TH1F*> hSystUp, hSystDown;
    for (unsigned int iS=0; iS!=systNames.size(); ++iS) {
      TH1F *hsyst; TString hname;
      
      hname = TString::Format("h_%s_Up",systNames[iS].Data());
      if (d->binEdges==0) {
        hsyst = new TH1F(hname.Data(),hname.Data(),d->nBins,d->min,d->max);
      } else {
        hsyst = new TH1F(hname.Data(),hname.Data(),d->nBins,d->binEdges);
      }
      hsyst->SetLineColor(systColors[iS]); hsyst->SetLineWidth(2);
      hSystUp.push_back(hsyst); ownedHistos.push_back(hsyst);

      hname = TString::Format("h_%s_Down",systNames[iS].Data());
      if (d->binEdges==0) {
        hsyst = new TH1F(hname.Data(),hname.Data(),d->nBins,d->min,d->max);
      } else {
        hsyst = new TH1F(hname.Data(),hname.Data(),d->nBins,d->binEdges);
      }
      hsyst->SetLineColor(systColors[iS]); hsyst->SetLineWidth(2);
      hSystDown.push_back(hsyst); ownedHistos.push_back(hsyst);
    }
    for (unsigned int iP : order) {
      Process *p = processes[iP];
      if (p==NULL) // not using this process
        continue;
      TTree *drawTree;
      TCut pWeight = p->additionalWeight;
      if (p->processtype!=kData && p->useCommonWeight)
        pWeight *= mcWeight;
      if (p->processtype<=kSignal3 && p->processtype!=kData)
        pWeight *= TCut(TString::Format("%f",signalScale));
      TCut thisCut(cut);
      if (eventmod!=0) {
        if (p->processtype==kData) // filter events
          thisCut += TCut(TString::Format("(%s%%%i)==1",eventnumber.Data(),eventmod).Data());
        else // scale events
          pWeight *= TCut(TString::Format("%f",1./eventmod).Data());
      }
      if (cloneTrees) {
        drawTree = p->clonedTree; 
      } else {
        drawTree = p->chain;
        pWeight *= (p->additionalCut + thisCut);
      }
      if (d->binEdges==0) {
        h = new TH1F(TString::Format("h_%s",p->name.Data()),TString::Format("h_%s",p->name.Data()),d->nBins,d->min,d->max);
        drawTree->Draw(TString::Format("%s>>h_%s",d->name.Data(),p->name.Data()),pWeight);
      } else {
        h = new TH1F(TString::Format("h_%s",p->name.Data()), TString::Format("h_%s",p->name.Data()), d->nBins,d->binEdges);
        drawTree->Draw(TString::Format("%s>>h_%s",d->name.Data(),p->name.Data()),pWeight);
        // should divide by bin width
        for (int iB=1; iB!=d->nBins+1; ++iB) {
          float currentVal = h->GetBinContent(iB);
          float currentErr = h->GetBinError(iB);
          float binWidth = d->binEdges[iB]-d->binEdges[iB-1];
          h->SetBinContent(iB,currentVal/binWidth);
          h->SetBinError(iB,currentErr/binWidth);
        }
      }
      if (p->processtype!=kData && p->processtype>kSignal3) {
        for (unsigned int iS=0; iS!=systNames.size(); ++iS) {
          TCut upWeight = mcWeightUp[iS];
          upWeight *= p->additionalWeight;
          if (eventmod!=0)
            upWeight *= TCut(TString::Format("%f",1./eventmod).Data());
          upWeight *= (p->additionalCut + thisCut);
          drawTree->Draw(TString::Format("%s>>+%s",d->name.Data(),hSystUp[iS]->GetName()).Data(),upWeight);

          TCut downWeight = mcWeightDown[iS];
          downWeight *= p->additionalWeight;
          if (eventmod!=0)
            downWeight *= TCut(TString::Format("%f",1./eventmod).Data());
          downWeight *= (p->additionalCut + thisCut);
          drawTree->Draw(TString::Format("%s>>+%s",d->name.Data(),hSystDown[iS]->GetName()).Data(),downWeight);
        }
      }
      if (d->maxY>-999)
        h->SetMaximum(d->maxY);
      if (d->minY<999)
        h->SetMinimum(d->minY);
      if (h==NULL) {
         fprintf(stderr,"bad variable\n");
         exit(1);
      }
      h->GetXaxis()->SetTitle(d->xLabel);
      h->GetYaxis()->SetTitle(d->yLabel);
      h->SetTitle("");
      if (!doStack && p->processtype!=kData)
        h->SetLineWidth(2);
      if (p->processtype<=kSignal3 && p->processtype!=kData && signalScale!=1)
        AddHistogram(h,TString::Format("%.1f#times%s",signalScale,p->name.Data()).Data(),p->processtype);
      else  
        AddHistogram(h,p->name.Data(),p->processtype);  
      ownedHistos.push_back(h);

      if (d->name=="1") {
          printf("%-10s %15f %15.0f\n",p->name.Data(),h->Integral(),h->GetEntries());
          if (iP<=kSignal3 && iP!=kData)
            sigTotal += h->Integral();
          else
            bgTotal += h->Integral();
      } else {
            fOut->WriteTObject(h,TString::Format("h_%s_%s",sanitize(d->filename).Data(),p->name.Data()),"Overwrite");
      }
    } // process loop
    for (unsigned int iS=0; iS!=systNames.size(); ++iS) {
      if (d->binEdges!=0) {
        hSystUp[iS]->Scale(1,"width");
        hSystDown[iS]->Scale(1,"width");
      }
      AddAdditional(hSystUp[iS],"hist",systNames[iS]);
      AddAdditional(hSystDown[iS],"hist");
    }
    
    if (tmpName!="1") {
      if (doLogy)
        tmpName += "_logy";
      HistogramDrawer::Draw(outDir,tmpName);
    } 
    Reset(false);
    for (auto h : ownedHistos) 
      delete h;
  } // distribution loop
  distributions.clear();
  fOut->Close(); // do not write or cloned trees will get saved
  fBuffer->Close();
  delete fBuffer;
  delete fOut;
} 
