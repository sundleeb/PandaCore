#include "../interface/HistogramDrawer.h"
#include "algorithm"
#include <math.h>
#include "THStack.h"
#include "TGraphErrors.h"

HistogramDrawer::HistogramDrawer(double x, double y):
  CanvasDrawer(x,y) {
    // done
  }

HistogramDrawer::~HistogramDrawer() {
  if (fileIsOwned)
    delete centralFile;
}

void HistogramDrawer::AddAdditional(TObject *o, TString opt, TString aname) {
  ObjWrapper w;
  w.o = o;
  w.opt = opt;
  w.label = aname;
  internalAdds.push_back(w);
}

void HistogramDrawer::AddSystematic(TH1D *o, TString opt, TString aname) {
  ObjWrapper w;
  w.o = o;
  w.opt = opt;
  w.label = aname;
  internalAdds.push_back(w);
  HistWrapper hw;
  hw.h = o;
  internalSysts.push_back(hw);
}

void HistogramDrawer::AddHistogram(TH1D *h, TString label, ProcessType pt, int cc, TString opt) {
  if (h!=NULL) {
    h->SetStats(0);
    h->SetTitle("");

    HistWrapper w;
    w.h = h;
    w.label = label;
    if (pt==nProcesses) 
      w.pt = (ProcessType)(internalHists.size()+2);
    else
      w.pt = pt;
    if (cc<0) 
      w.cc = Colors[w.pt];
    else
      w.cc = cc;
    if (opt=="") {
      if (pt==kData)
        w.opt = "e0lp";
      else
        w.opt = drawOption;
    } else
      w.opt = opt;

    internalHists.push_back(w);
  }
}

void HistogramDrawer::AddHistogram(TString hname, TString label, ProcessType pt, TString fname) {
  TFile *tmpFile;
  if (fname!="") 
    tmpFile = TFile::Open(fname);
  else 
    tmpFile = centralFile;

  TH1D *h = (TH1D*)tmpFile->Get(hname);
  AddHistogram(h,label,pt);
}

void HistogramDrawer::SetInputFile(TFile *f) {
  centralFile = f;
}

void HistogramDrawer::SetInputFile(TString fname) {
  fileIsOwned = true;
  centralFile = TFile::Open(fname);
}

void HistogramDrawer::Reset(bool clearPlotLabels) {
  internalHists.clear();
  internalAdds.clear();
  internalSysts.clear();
  if (clearPlotLabels)
    plotLabels.clear();
  if (legend!=NULL)
    legend->Clear();
  if (c!=NULL)
    c->Clear();
  if (pad1!=NULL)
    pad1->Clear();
  if (pad2!=NULL)
    pad2->Clear();
}

double getHistMin(TH1D *h, bool ignoreZero=false) {
  unsigned nB = h->GetNbinsX();
  double minY = 0;
  for (unsigned iB=1; iB!=nB+1; ++iB) {
    if (iB==1) {
      minY = h->GetBinContent(iB);
    } else {
      double val = h->GetBinContent(iB);
      if (ignoreZero && minY==0 && val>0)
        minY = val;
      else if (val<=0 && ignoreZero)
        continue;
      else if (val<minY)
        minY = val;
    }
  }
  return minY;
}


void HistogramDrawer::Draw(TString outDir, TString baseName) {
  if (c==NULL)
    c = new TCanvas();

//  c->Clear();

  unsigned int nH = internalHists.size();
  if (nH==0) {
    CanvasDrawer::Draw(outDir,baseName);
    return;
  }
  int nBins = internalHists[0].h->GetNbinsX();
  c->cd();
  float stackIntegral=0;
  THStack *hs=0;
  TH1D *hData=0;
  TH1D *hSignal[4] = {0,0,0,0};
  TH1D *hRatio=0;
  TH1D *hZero=0;
  TH1D *hSum=0;
  TH1D *hRatioErrorUp = 0;
  TH1D *hRatioErrorDown = 0;
  std::vector<HistWrapper> hOthers;

  c->SetLogy(doLogy&&(!doRatio));
  
  if (doStack)
    hs = new THStack("h","");
  
  for (unsigned int iH=0; iH!=nH; ++iH) {
    HistWrapper w = internalHists[iH];
    TH1D *h = w.h;
    ProcessType pt = w.pt;
    if (doStack && pt>kSignal3 && !drawEmpty) {
      h->SetLineColor(1);
    } else {
      h->SetLineColor(w.cc);
      h->SetLineWidth(3);
    }
    if (pt!=kData && (pt==kData || pt>kSignal3 || doStackSignal)) {
      if (doStack) {
        // if it's stackable
        if (drawEmpty) {
          h->SetFillColor(0);
          h->SetFillStyle(0);
        } else {
          h->SetFillColor(w.cc);
        }
        hs->Add(h);
        stackIntegral += h->Integral();
      } else {
        if (doSetNormFactor) 
          h->Scale(1./h->Integral());
      }
      hOthers.push_back(w);
    } else if (pt==kData) {
      // if it's data
      hData = h;
      if (w.opt.Contains("p")) {
        hData->SetMarkerColor(Colors[pt]);
        hData->SetMarkerStyle(20);
        if (whichstyle==3)
          hData->SetMarkerSize(1.2);
        else
          hData->SetMarkerSize(1);
      }
      if (doSetNormFactor) {
        /*
        for (int iB=0; iB!=nBins; ++iB) {
          hData->SetBinError(iB,hData->GetBinError(iB)/hData->Integral());
        }
        */
        hData->Scale(1./hData->Integral());
      }
    } else if (pt<=kSignal3 && pt!=kData) {
      // if it's signal and we're not stacking signal
      hSignal[pt-1] = h;
      hSignal[pt-1]->SetLineWidth(5);
      hSignal[pt-1]->SetFillStyle(0);
      if (doSetNormFactor)
        hSignal[pt-1]->Scale(1./hSignal[pt-1]->Integral());
    } 
  }
  for (int iH=nH-1; iH!=-1; --iH) {
    HistWrapper w = internalHists[iH];
    TString legOption = "L";
    if (w.opt.Contains("el"))
      legOption = "EL";
    if (doStack || w.opt.Contains("e2"))
      legOption = "F";
    if (w.pt==kData && w.opt=="elp")
      legOption = "ELP";
    if (legend && w.label!="")
      legend->AddEntry(w.h,w.label,legOption);
  }

  // scale stacked histograms and calculate stacked error bands
  if (doStack) {
    if (doSetNormFactor) {
      for (HistWrapper ww : hOthers) 
        ww.h->Scale(1./stackIntegral);
      for (auto w : internalSysts)
        w.h->Scale(1./stackIntegral);
    }
    if (doDrawMCErrors||doRatio) {
      std::vector<float> vals,errs;
      hSum = (TH1D*) (hOthers[0].h->Clone("hsum"));
      hSum->SetFillColorAlpha(kBlack,0.99);
      hSum->SetLineWidth(0);
      hSum->SetFillStyle(3003);
      for (int iB=0; iB!=nBins; ++iB) {
        hSum->SetBinContent(iB+1,0); 
        vals.push_back(0);
        errs.push_back(0);
      }
      
      for (HistWrapper w : hOthers) {
        TH1D *hh = w.h;
        for (int iB=0; iB!=nBins; ++iB) {
          vals[iB] += hh->GetBinContent(iB+1);
          errs[iB] += std::pow(hh->GetBinError(iB+1),2);
        }
      }
      
      for (int iB=0; iB!=nBins; ++iB) {
        hSum->SetBinContent(iB+1,vals[iB]);
        hSum->SetBinError(iB+1,std::sqrt(errs[iB]));
      }
    }

  }

  // figure out min and max 
  double maxY=0, minY=9999;
  if (hData!=NULL) {
    maxY = std::max(maxY,hData->GetMaximum());
    minY = std::min(minY,getHistMin(hData,doLogy));
  }
  if (doStack) {
    if (doLogy) {
      TH1D *hstacked = 0;
      for (HistWrapper w : hOthers) {
        if (!hstacked)
          hstacked = (TH1D*)(w.h->Clone("hstacked"));
        else
          hstacked->Add(w.h);
      }
      if (hstacked) 
        minY = std::min(minY,getHistMin(hstacked,doLogy));
      delete hstacked;
    }
    maxY = hs->GetMaximum();        
  } else {
    for (HistWrapper w : hOthers) {
      TH1D *hh = w.h;
      maxY = std::max(maxY,hh->GetMaximum());
      if (doLogy)
        minY = std::min(minY,getHistMin(hh,doLogy));
    }
  }
  for (unsigned int iS=0; iS!=4; ++iS) {
    if (!doStackSignal && hSignal[iS]!=NULL) {
      maxY = std::max(hSignal[iS]->GetMaximum(),maxY);
      minY = std::min(getHistMin(hSignal[iS],doLogy),minY);
    }
  }
  if (doLogy) {
    maxY *= maxScale;
    minY *= 0.05;
    if (absMin!=-999)
      minY = std::max(minY,absMin);
  } else {
    maxY *= maxScale;
    if (absMin!=-999)
      minY = absMin;
    else
      minY=0;
  }

  // set up canvases
  c->cd();
  if (doRatio && hData) {
    SplitCanvas();
    if (doLogy)
      pad1->SetLogy();
    pad1->cd();
  }
  TString xlabel = internalHists[0].h->GetXaxis()->GetTitle();
  TString ylabel = internalHists[0].h->GetYaxis()->GetTitle();
  for (HistWrapper w : internalHists) {
    w.h->GetXaxis()->SetTitle(xlabel);
    w.h->GetYaxis()->SetTitle(ylabel);
  }

  // figure out what to do with the first histogram
  TH1D *firstHist=0;
  if (!doStack) {
    if (hOthers.size()>0) 
      firstHist = hOthers[0].h;
    else if (hData)
      firstHist = hData;
    else {
      for (unsigned int iS=0; iS!=4; ++iS) {
        if (hSignal[iS]!=NULL) {
          firstHist = hSignal[iS];
          break;
        }
      }
    }
  }
  if (firstHist) {
    if (doAutoRange) {
      firstHist->SetMinimum(minY);
      firstHist->SetMaximum(maxY);
    }
    if (firstHist==hData)
      firstHist->Draw("elp");
    else if (hOthers.size()>0 && firstHist==hOthers[0].h)
      firstHist->Draw(hOthers[0].opt);
    else
      firstHist->Draw(drawOption);
    /*
    if (!doRatio) {
      firstHist->GetXaxis()->SetTitle(xlabel);
    } else {
      firstHist->GetXaxis()->SetTitle("");
    }
    firstHist->GetYaxis()->SetTitle(ylabel);
    */
    firstHist->GetYaxis()->SetTitleOffset(1.5);
  }

  // plot everything else
  if (doStack) {
    if (doAutoRange) {
      hs->SetMinimum(minY); 
      hs->SetMaximum(maxY);
    }
    hs->Draw(drawOption);
    if (!doRatio) {
      hs->GetXaxis()->SetTitle(xlabel);
    } else {
      hs->GetXaxis()->SetTitle("");
    }
    hs->GetYaxis()->SetTitle(ylabel);
    hs->GetYaxis()->SetTitleOffset(1.5);
    if (doDrawMCErrors)
      hSum->Draw("e2 same");
  } else {
    for (HistWrapper w : hOthers) {
      TH1D *hh = w.h;
      if (hh==firstHist)
        continue;
      hh->Draw(w.opt+" same");
    }
  }
  for (unsigned int iS=0; iS!=4; ++iS) {
    if (hSignal[iS]!=NULL && firstHist!=hSignal[iS]) {
      hSignal[iS]->Draw(drawOption+" same"); 
    }
  }

  unsigned int nAdd = internalAdds.size();
  for (unsigned int iA=0; iA!=nAdd; ++iA) {
    ObjWrapper w = internalAdds[iA];
    TObject *o = w.o;
    TString opt = w.opt;
    TString className(o->ClassName());    
    if (className.Contains("TH1") || className.Contains("TProfile"))
      o->Draw(opt+" same");
    else if (className.Contains("TGraph"))
      o->Draw(opt+" same");
    else if (className.Contains("TF1"))
      o->Draw(opt+" same");
    else {
      PWarning("HistogramDrawer::Draw",TString::Format("Don't know what to do with %s",className.Data()));
      continue;
    }

    TString label = w.label;
    if (label.Length()>0 && legend) {
      legend->AddEntry(o,label,"l");
    }
  }

  if (hData!=NULL && firstHist!=hData) {
    hData->Draw("elp same"); 
  }

  if (legend)
    legend->Draw();

  c->cd();

  TGraphErrors *gRatioErrors=0;
  double *xVals=0, *yVals=0, *xErrors=0, *yErrors=0;
  if (doRatio && hData) {
    pad2->cd();
    pad2->SetGridy();
    hRatio = (TH1D*)hData->Clone("ratio");
    hRatio->SetMaximum(-1111); hRatio->SetMinimum(-1111);
    if (doDrawMCErrors) {
      hRatioErrorUp = (TH1D*)hSum->Clone("sumratioup");
      hRatioErrorDown = (TH1D*)hSum->Clone("sumratiodown");
      hRatioErrorUp->SetMaximum(-1111); hRatioErrorUp->SetMinimum(-1111);
      hRatioErrorDown->SetMaximum(-1111); hRatioErrorDown->SetMinimum(-1111);
      hRatioErrorUp->SetFillStyle(3003); hRatioErrorUp->SetLineWidth(1);
      hRatioErrorDown->SetFillStyle(3003); hRatioErrorDown->SetLineWidth(1);
    }
    hZero = (TH1D*)hData->Clone("zero"); 
    hZero->SetMaximum(-1111); hZero->SetMinimum(-1111);
    for (int iB=0; iB!=nBins; ++iB) 
      hZero->SetBinContent(iB+1,0);
    
    float maxVal=0;
    xVals = new double[nBins];
    yVals = new double[nBins];
    xErrors = new double[nBins];
    yErrors = new double[nBins];
    for (int iB=1; iB!=nBins+1; ++iB) {
      float mcVal=0;
      for (HistWrapper w : hOthers) {
        TH1D *hh = w.h;
        mcVal += hh->GetBinContent(iB);
      }
      float dataVal = hData->GetBinContent(iB);
      float err = hData->GetBinError(iB);
      float mcErrUp = 0, mcErrDown = 0;
      if (doDrawMCErrors) {
        mcErrUp = pow(hSum->GetBinError(iB),2);
        mcErrDown = mcErrUp;
        for (auto w : internalSysts) {
          double systerr = w.h->GetBinContent(iB)-hSum->GetBinContent(iB);
          if (systerr>0) {
            mcErrUp += pow(systerr,2);
          } else {
            mcErrDown += pow(systerr,2);
          }
        }
        mcErrUp = TMath::Sqrt(mcErrUp);
        mcErrDown = -1*TMath::Sqrt(mcErrDown);
      }
      float val,errVal,mcErrValUp, mcErrValDown;
      if (dataVal==0.||mcVal==0.) {
        if (dataVal>0) 
          PWarning("HistogramDrawer::Draw",TString::Format("bin %i has DATA=%.1f, but EXP=%.3f",iB,dataVal,mcVal));
        if (mcVal==0) {
          mcErrValUp=0;
          mcErrValDown=0;
        } else {
          mcErrValUp = mcErrUp/mcVal;
          mcErrValDown = mcErrDown/mcVal;
        }
        val=0;
        errVal=0;
      } else {
        val = dataVal/mcVal-1;
        errVal = err/mcVal;
        mcErrValUp = mcErrUp/mcVal;
        mcErrValDown = mcErrDown/mcVal;
      }
      if (dataVal==0) val=-999; // don't plot missing data points
      xVals[iB-1] = hRatio->GetBinCenter(iB);
      yVals[iB-1] = val;
      xErrors[iB-1] = 0;
      yErrors[iB-1] = errVal;
      maxVal = std::max(maxVal,std::abs(val));
      maxVal = std::max(maxVal,std::abs(errVal+val));
      maxVal = std::max(maxVal,std::abs(val+errVal));
      hRatio->SetBinContent(iB,val);
      hRatio->SetBinError(iB,0.0001);
      if (doDrawMCErrors) {
        hRatioErrorUp->SetBinContent(iB,mcErrValUp);
        hRatioErrorDown->SetBinContent(iB,mcErrValDown);
      }
    } 
    gRatioErrors = new TGraphErrors(nBins,xVals,yVals,xErrors,yErrors);
    maxVal = std::min((double)maxVal,.5);
    if (fixRatio && ratioMax>0)
      maxVal = ratioMax;
    maxVal = std::max(double(maxVal),0.1);
    hRatio->SetTitle("");
    hRatio->Draw("elp");
    hRatio->SetMinimum(-1.2*maxVal);
    hRatio->SetMaximum(maxVal*1.2);
    hRatio->SetLineColor(1);
    hRatio->SetMarkerStyle(20);
    hRatio->SetMarkerSize(1);
    hRatio->GetXaxis()->SetTitle(xlabel);
    hRatio->GetYaxis()->SetTitle(ratioLabel);
    hRatio->GetYaxis()->SetNdivisions(5);
    hRatio->GetYaxis()->SetTitleSize(15);
    hRatio->GetYaxis()->SetTitleFont(43);
    hRatio->GetYaxis()->SetTitleOffset(2.5);
    hRatio->GetYaxis()->SetLabelFont(43); 
    hRatio->GetYaxis()->SetLabelSize(15);
    hRatio->GetXaxis()->SetTitleSize(20);
    hRatio->GetXaxis()->SetTitleFont(43);
    hRatio->GetXaxis()->SetTitleOffset(5);
    hRatio->GetXaxis()->SetLabelFont(43);
    hRatio->GetXaxis()->SetLabelSize(20);
    gRatioErrors->SetLineWidth(2);
    hZero->SetLineColor(1);
    hZero->Draw("hist same");
    if (doDrawMCErrors) {
      hRatioErrorUp->Draw("hist same");
      hRatioErrorDown->Draw("hist same");
    }
    gRatioErrors->Draw("pe0");
  }
  if (doRatio && hData)
    pad1->cd();

  CanvasDrawer::Draw(outDir,baseName);
  delete hSum; hSum=0;
  delete hRatioErrorUp; hRatioErrorUp=0;
  delete hRatioErrorDown; hRatioErrorDown=0;
  delete hs; hs=0;
  delete hRatio; hRatio=0;
  delete hZero; hZero=0;
  delete pad1; pad1=0;
  delete pad2; pad2=0;
  delete hs; hs=0;
  delete gRatioErrors; gRatioErrors=0;
  hOthers.clear();
}
     

