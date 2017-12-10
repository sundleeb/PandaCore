#include "../interface/ROCTool.h"
#include <algorithm>

#define DEBUG 0

ROCTool::~ROCTool() {
  delete c; c=0; 
}

void ROCTool::InitCanvas(double x1, double y1, double x2, double y2, bool isPrelim) {
  c = new GraphDrawer();
  c->SetTDRStyle();
  c->SetGrid();
  if (isPrelim)
    c->AddCMSLabel(0.15,0.94," Simulation Preliminary"); 
  else
    c->AddCMSLabel(0.15,0.94," Simulation"); 
  c->AddSqrtSLabel();
  if (doLogy)
    c->Logy();
  if (x1>0)
    c->InitLegend(x1,y1,x2,y2);
  else if (doLogy)
    c->InitLegend(0.6,.15,.94,.5);
  else
    c->InitLegend(.15,.4,.43,.83);
} 

void ROCTool::SetFile(TFile *f) {
  if (fileIsOwned)
    delete centralFile;
  centralFile = f;
  fileIsOwned=false;
}

void ROCTool::SetFile(TString fpath) {
  if (fileIsOwned)
    delete centralFile;
  centralFile = TFile::Open(fpath);
  fileIsOwned=true;
}

TMarker *ROCTool::CalcWP(TTree *ts, TTree *tb, int color, TString sigcut, TString sigweight, TString bgcut, TString bgweight, TString cut, TString title) {
  TH1F *hst = new TH1F("hst","hst",2,-2,2);
  TH1F *hsn = (TH1F*)hst->Clone("hsn");
  TH1F *hbt = (TH1F*)hst->Clone("hbt");
  TH1F *hbn = (TH1F*)hst->Clone("hbn");

  if (DEBUG) {
    PDebug("","sigcut:    "+sigcut);
    PDebug("","sigweight: "+sigweight);
    PDebug("","bgcut:     "+bgcut);
    PDebug("","bgweight:  "+bgweight);
    PDebug("","cut:       "+cut);
  }
  
  ts->Draw("1>>hst","("+sigweight+")*("+sigcut+")");
  double sigtot = hst->Integral();

  tb->Draw("1>>hbt","("+bgweight+")*("+bgcut+")");
  double bgtot = hbt->Integral();

  ts->Draw("1>>hsn","("+sigweight+")*(("+sigcut+")&&("+cut+"))");
  double signum = hsn->Integral();

  tb->Draw("1>>hbn","("+bgweight+")*(("+bgcut+")&&("+cut+"))");
  double bgnum = hbn->Integral();

  double eff = signum/sigtot; double fake = bgnum/bgtot;
  PInfo("ROCTool::CalcWP",TString::Format("eff=%.2f => fakerate=%.2f",eff,fake));

  TMarker *m = new TMarker(eff,fake,1);
  m->SetMarkerColor(color);
  m->SetMarkerSize(2);
  m->SetMarkerStyle(29);

  if (c) {
    c->AddMarker(m,title);
  }

  delete hst; delete hsn; delete hbt; delete hbn;

  return m;
}


TGraph *ROCTool::CalcROC(TH1F *hs, TH1F *hb, const char *title, unsigned int color, int style, int nCuts) {
  if (hs && hb)
    SetHists(hs,hb);
  TGraph *graph;
  if (nCuts==1) 
    graph = CalcROC1Cut();   
  else
    graph = CalcROC2Cut();

  if (c) {
    c->AddGraph(graph,title,color,style,"C");
  }

  return graph;
}

TGraph *ROCTool::CalcROC(TString hspath, TString hbpath, const char *title, unsigned int color, int style, int nCuts) {
  SetHists(hspath,hbpath);
  return CalcROC(0,0,title,color,style,nCuts);
}

TGraph *ROCTool::CalcROC1Cut() {
  bool normal = (sigHist->GetMean() > bgHist->GetMean());
  
  unsigned int nB = sigHist->GetNbinsX();
  float *effs = new float[nB+1], *rejs = new float[nB+1], *cuts = new float[nB+1];
  float sigIntegral = sigHist->Integral(1,nB);
  float bgIntegral = bgHist->Integral(1,nB);

  float zeroRej = 0;
  for (unsigned int iB=1; iB!=nB+1; ++iB) {
    if (normal) {
      float eff = sigHist->Integral(iB,nB)/sigIntegral;
      float rej = bgHist->Integral(iB,nB)/bgIntegral;
      effs[iB-1] = eff; rejs[iB-1] = rej;
      cuts[iB-1] = sigHist->GetBinLowEdge(iB);
    } else {
      float eff = sigHist->Integral(1,iB)/sigIntegral;
      float rej = bgHist->Integral(1,iB)/bgIntegral;
      effs[iB] = eff; rejs[iB] = rej;
      cuts[iB-1] = sigHist->GetBinLowEdge(iB)+sigHist->GetBinWidth(iB);
    } 
    if (invertFake && rejs[iB-1] > 0.) {
      rejs[iB-1] = 1./rejs[iB-1];
      zeroRej = std::max(10*rejs[iB-1],zeroRej);
    }
  }

  if (normal) { effs[nB]=0; rejs[nB]=zeroRej; }
  else { effs[0]=0; rejs[0]=zeroRej; }
  for (unsigned int iB=0; iB!=nB; ++iB) {
    if (rejs[iB] == 0)
      rejs[iB] = zeroRej;
  }

  TGraph *roc = new TGraph(nB+1,effs,rejs);
  roc->GetXaxis()->SetTitle("Signal efficiency");
  if (invertFake)
    roc->GetYaxis()->SetTitle("1 / Background acceptance"); 
  else
    roc->GetYaxis()->SetTitle("Background acceptance"); 
  roc->GetYaxis()->SetTitleOffset(1.5);
  roc->SetMinimum(minval); roc->SetMaximum(maxval);
  roc->GetXaxis()->SetLimits(0,1);

  TGraph *gcuts = new TGraph(nB+1,effs,cuts);

  PInfo("ROCTool::CalcROC",TString::Format("eff=0.3 => fakerate=%.3g, at x=%.2f",roc->Eval(0.3),gcuts->Eval(0.3)));

  return roc;
}

TGraph *ROCTool::CalcROC2Cut() {
  unsigned int nB = sigHist->GetNbinsX();
  float *effs = new float[nB], *rejs = new float[nB];
  float sigIntegral = sigHist->Integral(1,nB);
  float bgIntegral = bgHist->Integral(1,nB);

  std::vector<TGraph*>rocs;
  for (unsigned int iB=1; iB!=nB+1; ++iB) {
     // accept entries > iB
    for (unsigned int jB=1; jB!=nB+1; ++jB) {
      // accept entries < jB
      float eff=1,rej=1;
      if (iB<jB) {
        // this is a window
        eff = sigHist->Integral(iB,jB)/sigIntegral;
        rej = bgHist->Integral(iB,jB)/bgIntegral;
      } else {
        // we are rejecting the window
        eff = (sigHist->Integral(1,jB)+sigHist->Integral(iB,nB))/sigIntegral;
        rej = (bgHist->Integral(1,jB)+bgHist->Integral(iB,nB))/bgIntegral;
      }
      effs[jB-1] = eff; rejs[jB-1]=rej;
    }
    TGraph *roc = new TGraph(nB,effs,rejs);
    rocs.push_back(roc);
  }

  float *effs2D = new float[nB], *rejs2D = new float[nB];
  for (unsigned int iB=0; iB!=nB; ++iB) {
    effs2D[iB] = (1.*iB)/(nB);
    float bestRej=1;
    for (auto *roc : rocs) {
      bestRej = std::min((float)bestRej,(float)roc->Eval(effs2D[iB]));
    }
    rejs2D[iB] = bestRej;
  }

  TGraph *roc = new TGraph(nB,effs2D,rejs2D);
  roc->GetXaxis()->SetTitle("signal efficiency");
  roc->GetYaxis()->SetTitle("background acceptance"); 
  roc->GetYaxis()->SetTitleOffset(1.5);
  roc->SetMinimum(minval); roc->SetMaximum(maxval);
  roc->GetXaxis()->SetLimits(0,1);

  for (auto *r:rocs)
    delete r;
  return roc;
}

void ROCTool::DrawAll(TString outDir, TString basePath) {
  c->Draw(outDir,basePath);
}
