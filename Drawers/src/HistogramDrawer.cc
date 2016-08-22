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

void HistogramDrawer::AddHistogram(TH1F *h, TString label, ProcessType pt, int cc) {
  if (h!=NULL) {
    h->SetStats(0);
    h->SetTitle("");
    internalHistos.push_back(h);
    labels.push_back(label);
    if (pt==nProcesses) 
      ptypes.push_back((ProcessType)(ptypes.size()+2));
    else
      ptypes.push_back(pt);
    if (cc<0) 
      customColors.push_back(PlotColors[ptypes.back()]);
    else
      customColors.push_back(cc);
  }
}

void HistogramDrawer::AddHistogram(TString hname, TString label, ProcessType pt, TString fname) {
  TFile *tmpFile;
  if (fname!="") 
    tmpFile = TFile::Open(fname);
  else 
    tmpFile = centralFile;

  TH1F *h = (TH1F*)tmpFile->Get(hname);
  AddHistogram(h,label,pt);
}

void HistogramDrawer::SetInputFile(TFile *f) {
  centralFile = f;
}

void HistogramDrawer::SetInputFile(TString fname) {
  fileIsOwned = true;
  centralFile = TFile::Open(fname);
}

void HistogramDrawer::Reset() {
  internalHistos.clear();
  ptypes.clear();
  labels.clear();
  customColors.clear();
  additionals.clear();
  if (legend!=NULL)
    legend->Clear();
  if (c!=NULL)
    c->Clear();
}

void HistogramDrawer::Draw(TString outDir, TString baseName) {
  if (c==NULL)
    c = new TCanvas();

//  c->Clear();

  unsigned int nH = internalHistos.size();
  if (nH==0) {
    CanvasDrawer::Draw(outDir,baseName);
    return;
  }
  int nBins = internalHistos[0]->GetNbinsX();
  c->cd();
  TPad *pad1=0, *pad2=0;
  float stackIntegral=0;
  THStack *hs=0;
  TH1F *hData=0;
  TH1F *hSignal[4] = {0,0,0,0};
  TH1F *hRatio=0;
  TH1F *hZero=0;
  TH1F *hSum=0;
  TH1F *hRatioError = 0;
  std::vector<TH1F*> hOthers;

  c->SetLogy(doLogy&&(!doRatio));
  
  if (doStack)
    hs = new THStack("h","");
  
  for (unsigned int iH=0; iH!=nH; ++iH) {
    ProcessType pt = ptypes[iH];
    TH1F *h=internalHistos[iH];
    //h->SetLineColor(PlotColors[pt]);
    if (doStack && pt>kSignal3) {
      h->SetLineColor(1);
    } else {
      h->SetLineColor(customColors[iH]);
      h->SetLineWidth(3);
      // h->SetLineColor(PlotColors[pt]);
    }
    TString legOption = "L";
    if (pt!=kData && (pt==kData || pt>kSignal3 || doStackSignal)) {
      if (doStack) {
        // if it's stackable
        h->SetFillColor(customColors[iH]);
     //   h->SetFillColor(PlotColors[pt]);
        hs->Add(h);
        legOption = "F";
        stackIntegral += h->Integral();
      } else {
        if (doSetNormFactor) 
          h->Scale(1./h->Integral());

      }
      hOthers.push_back(h);
    } else if (pt==kData) {
      // if it's data
      legOption = "ELP";
      hData = h;
      hData->SetMarkerColor(PlotColors[pt]);
      hData->SetMarkerStyle(20);
      if (doSetNormFactor) {
        for (int iB=0; iB!=nBins; ++iB) {
          hData->SetBinError(iB,hData->GetBinError(iB)/hData->Integral());
        }
        hData->Scale(1./hData->Integral());
      }
    } else if (pt<=kSignal3 && pt!=kData) {
      // if it's signal and we're not stacking signal
      hSignal[pt-1] = h;
      hSignal[pt-1]->SetLineWidth(5);
      hSignal[pt-1]->SetFillStyle(0);
//      hSignal[pt-1]->SetFillColor(hSignal[pt-1]->GetLineColor());
//      hSignal[pt-1]->SetFillStyle(3004);
      if (doSetNormFactor)
        hSignal[pt-1]->Scale(1./hSignal[pt-1]->Integral());
    } 
    if (legend) 
      legend->AddEntry(h,labels[iH],legOption);
  }


  // scale stacked histograms and calculate stacked error bands
  if (doStack) {
    if (doSetNormFactor) {
      for (TH1F *hh : hOthers) 
        hh->Scale(1./stackIntegral);
    }
    if (doDrawMCErrors) {
      std::vector<float> vals,errs;
      hSum = (TH1F*)hOthers[0]->Clone("hsum"); 
      hSum->SetFillColorAlpha(kBlack,0.99);
      hSum->SetLineWidth(0);
      hSum->SetFillStyle(3004);
      for (int iB=0; iB!=nBins; ++iB) {
        hSum->SetBinContent(iB+1,0); 
        vals.push_back(0);
        errs.push_back(0);
      }
      
      for (TH1F *hh : hOthers) {
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
  if (doStack) {
    if (doLogy) {
      double sumMin=0;
      for (TH1F *hh : hOthers) {
        double tmp_min=9999;
        // for (int iB=1; iB!=hh->GetNbinsX()+1; ++iB) {
        //   if (hh->GetBinContent(iB)>0)
        //     tmp_min = std::min(tmp_min,hh->GetBinContent(iB));
        // }
        tmp_min = hh->GetMinimum();
        if (tmp_min<9999)
          sumMin += tmp_min;
      }
      minY = std::min(minY,sumMin);
    }
    maxY = hs->GetMaximum();        
  } else {
    for (TH1F *hh : hOthers) {
      maxY = std::max(maxY,hh->GetMaximum());
      if (doLogy)
        minY = std::min(minY,hh->GetMinimum());
    }
  }
  for (unsigned int iS=0; iS!=4; ++iS) {
    if (!doStackSignal && hSignal[iS]!=NULL) {
      maxY = std::max(hSignal[iS]->GetMaximum(),maxY);
      minY = std::min(hSignal[iS]->GetMinimum(),minY);
    }
  }
  if (hData!=NULL)
    maxY = std::max(maxY,hData->GetMaximum());
  if (doLogy) {
    maxY *= maxScale;
    minY *= 0.5;
    if (absMin!=-999)
      minY = std::max(minY,absMin);
    else
      minY = std::max(minY,0.001);
  } else {
    maxY *= maxScale;
    if (absMin!=-999)
      minY = absMin;
    else
      minY=0;
  }

  // set up canvases
  c->cd();
  if (doRatio) {
    pad1 = new TPad("pad1", "pad1", 0, 0.3, 1, 1.0);
    if (doLogy)
      pad1->SetLogy();
    pad1->SetBottomMargin(0);
//        pad1->SetLeftMargin(.15);
    pad1->Draw();
    pad1->cd();
  }
  TString xlabel = internalHistos[0]->GetXaxis()->GetTitle();
  TString ylabel = internalHistos[0]->GetYaxis()->GetTitle();
  for (auto h : internalHistos) {
    h->GetXaxis()->SetTitle("");
  }

  // figure out what to do with the first histogram
  TH1F *firstHist=0;
  if (!doStack) {
    if (hOthers.size()>0) 
      firstHist = hOthers[0];
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
    firstHist->SetMinimum(minY);
    firstHist->SetMaximum(maxY);
    if (firstHist==hData)
      firstHist->Draw("elp");
    else
      firstHist->Draw(drawOption);
    if (!doRatio) {
      firstHist->GetXaxis()->SetTitle(xlabel);
    } else {
      firstHist->GetXaxis()->SetTitle("");
    }
    firstHist->GetYaxis()->SetTitle(ylabel);
    firstHist->GetYaxis()->SetTitleOffset(1.5);
  }

  // plot everything else
  if (doStack) {
    hs->SetMinimum(minY); 
    hs->SetMaximum(maxY);
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
    for (TH1F *hh : hOthers) {
      if (hh==firstHist)
        continue;
      hh->Draw(drawOption+" same");
    }
  }
  for (unsigned int iS=0; iS!=4; ++iS) {
    if (hSignal[iS]!=NULL && firstHist!=hSignal[iS]) {
      hSignal[iS]->Draw(drawOption+" same"); 
    }
  }
  unsigned int nAdd = additionals.size();
  for (unsigned int iA=0; iA!=nAdd; ++iA) {
    auto o = additionals[iA];
    TString className(o.first->ClassName());    
    if (className.Contains("TH1"))
      o.first->Draw(o.second+" same");
    else if (className.Contains("TGraph"))
      o.first->Draw(o.second+" same");
    else if (className.Contains("TF1"))
      o.first->Draw(o.second+" same");
    else {
      printf("Don't know what to do with %s\n",className.Data());
      continue;
    }
    TString aname = anames[iA];
    if (aname.Length()>0) {
      legend->AddEntry(o.first,aname,"l");
    }
  }

  if (hData!=NULL && firstHist!=hData) {
    hData->Draw("elp same"); 
  }

  if (legend)
    legend->Draw();

  c->cd();

  TGraphErrors *gRatioErrors;
  double *xVals=0, *yVals=0, *xErrors=0, *yErrors=0;
  if (doRatio) {
    pad2 = new TPad("pad2", "pad2", 0, 0.05, 1, 0.3);
    pad2->SetTopMargin(0);
    pad2->SetBottomMargin(0.3);
    pad2->Draw();
    pad2->cd();
    hRatio = (TH1F*)hData->Clone("ratio");
    if (doDrawMCErrors)
      hRatioError = (TH1F*)hSum->Clone("sumratio");
    hZero = (TH1F*)hData->Clone("zero"); 
    for (int iB=0; iB!=nBins; ++iB) 
      hZero->SetBinContent(iB+1,1);
    
    float maxVal=0;
    xVals = new double[nBins];
    yVals = new double[nBins];
    xErrors = new double[nBins];
    yErrors = new double[nBins];
    for (int iB=1; iB!=nBins+1; ++iB) {
      float mcVal=0;
      for (TH1F *hh : hOthers)
        mcVal += hh->GetBinContent(iB);
      float dataVal = hData->GetBinContent(iB);
      float err = hData->GetBinError(iB);
      float mcErr = 0;
      if (doDrawMCErrors)
        mcErr = hSum->GetBinError(iB);
      float val,errVal,mcErrVal;
      if (dataVal==0.||mcVal==0.) {
        if (dataVal>0) 
          fprintf(stderr,"WARNING, BIN %i HAS DATA=%.1f, but EXP=%.3f\n",iB,dataVal,mcVal);
        if (mcVal==0) 
          mcErrVal=0;
        else
          mcErrVal = mcErr/mcVal;
        val=0;
        errVal=0;
      } else {
        //val = (dataVal-mcVal)/dataVal;
        //errVal = err/dataVal;
        val = dataVal/mcVal;
        errVal = err/mcVal;
        mcErrVal = mcErr/mcVal;
      }
      if (val==0.) val=-999; // don't plot missing data points
      //maxVal = std::max(maxVal,std::abs(val));
      xVals[iB-1] = hRatio->GetBinCenter(iB);
      yVals[iB-1] = val;
      xErrors[iB-1] = 0;
      yErrors[iB-1] = errVal;
      maxVal = std::max(maxVal,std::abs(val-1));
      maxVal = std::max(maxVal,std::abs(errVal+val-1));
      maxVal = std::max(maxVal,std::abs(1-val-errVal));
      hRatio->SetBinContent(iB,val);
      hRatio->SetBinError(iB,0.0001);
//      printf("hratio %i %f %f\n",iB,val,errVal);
      if (doDrawMCErrors) {
        hRatioError->SetBinContent(iB,1);
        hRatioError->SetBinError(iB,mcErrVal);
      }
    } 
    gRatioErrors = new TGraphErrors(nBins,xVals,yVals,xErrors,yErrors);
    maxVal = std::min((double)maxVal,1.5);
    if (fixRatio && ratioMax>0)
      maxVal = ratioMax;
    maxVal = std::max(double(maxVal),0.6);
    hRatio->SetTitle("");
    hRatio->Draw("elp");
    hRatio->SetMinimum(-1.2*maxVal+1);
    hRatio->SetMaximum(maxVal*1.2+1);
    //hRatio->SetMinimum(-1.2*maxVal);
    //hRatio->SetMaximum(maxVal*1.2);
    hRatio->SetLineColor(1);
    hRatio->SetMarkerStyle(20);
    hRatio->GetXaxis()->SetTitle(xlabel);
    //hRatio->GetYaxis()->SetTitle("(Data-MC)/Data");
    hRatio->GetYaxis()->SetTitle("Data/Exp");
    hRatio->GetYaxis()->SetNdivisions(5);
    hRatio->GetYaxis()->SetTitleSize(15);
    hRatio->GetYaxis()->SetTitleFont(43);
    hRatio->GetYaxis()->SetTitleOffset(1.55);
    hRatio->GetYaxis()->SetLabelFont(43); 
    hRatio->GetYaxis()->SetLabelSize(15);
    hRatio->GetXaxis()->SetTitleSize(20);
    hRatio->GetXaxis()->SetTitleFont(43);
    hRatio->GetXaxis()->SetTitleOffset(4.);
    hRatio->GetXaxis()->SetLabelFont(43);
    hRatio->GetXaxis()->SetLabelSize(15);
    gRatioErrors->SetLineWidth(2);
    gRatioErrors->Draw("p");
    hZero->SetLineColor(1);
    hZero->Draw("hist same");
    if (doDrawMCErrors)
      hRatioError->Draw("e2 same");
  }
  if (doRatio)
    pad1->cd();

  CanvasDrawer::Draw(outDir,baseName);
  delete hSum; hSum=0;
  delete hRatioError; hRatioError=0;
  delete hs; hs=0;
  delete hRatio; hRatio=0;
  delete hZero; hZero=0;
  delete pad1; pad1=0;
  delete pad2; pad2=0;
  delete hs; hs=0;
  hOthers.clear();
}
     

