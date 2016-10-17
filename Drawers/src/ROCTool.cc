#include "../interface/ROCTool.h"
#include <algorithm>

ROCTool::~ROCTool() {
  delete c; c=0; 
}

void ROCTool::InitCanvas(double x1, double y1, double x2, double y2) {
  c = new GraphDrawer();
  c->SetTDRStyle();
  c->SetGrid();
  c->AddCMSLabel();
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
  float *effs = new float[nB+1], *rejs = new float[nB+1];
  float sigIntegral = sigHist->Integral(1,nB);
  float bgIntegral = bgHist->Integral(1,nB);

  bool foundHalf=false;
  for (unsigned int iB=1; iB!=nB+1; ++iB) {
    if (normal) {
      float eff = sigHist->Integral(iB,nB)/sigIntegral;
      float rej = bgHist->Integral(iB,nB)/bgIntegral;
      effs[iB-1] = eff; rejs[iB-1] = rej;
      if (!foundHalf && eff<0.5) {
        PInfo("ROCTool::CalcROC",TString::Format("eff=%.2f, rej=%.2f, at x=%.2f",eff,rej,sigHist->GetBinCenter(iB)));
        foundHalf=true;
      }
    } else {
      float eff = sigHist->Integral(1,iB)/sigIntegral;
      float rej = bgHist->Integral(1,iB)/bgIntegral;
      effs[iB] = eff; rejs[iB] = rej;
      if (!foundHalf && eff>0.5) {
        PInfo("ROCTool::CalcROC",TString::Format("eff=%.2f, rej=%.2f, at x=%.2f",eff,rej,sigHist->GetBinCenter(iB)));
        foundHalf=true;
      }
    } 
  }
  if (normal)
  { effs[nB]=0; rejs[nB]=0; }
  else
  { effs[0]=0; rejs[0]=0; }

  TGraph *roc = new TGraph(nB+1,effs,rejs);
  roc->GetXaxis()->SetTitle("signal efficiency");
  roc->GetYaxis()->SetTitle("background acceptance"); 
  roc->GetYaxis()->SetTitleOffset(1.5);
  roc->SetMinimum(minval); roc->SetMaximum(maxval);
  roc->GetXaxis()->SetLimits(0,1);

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
