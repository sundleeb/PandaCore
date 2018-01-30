#include "../interface/CanvasDrawer.h"
#include "TGaxis.h"
#include "TSystem.h"

CanvasDrawer::CanvasDrawer(double x, double y) {
  if (x>0) {
    c = new TCanvas("c","c",x,y);
    canvasIsOwned=true;
  }
  label = new TLatex();
  label->SetNDC(); 
  gErrorIgnoreLevel=kWarning;
  Colors = PlotColors;
}

CanvasDrawer::CanvasDrawer(TCanvas *c0) {
  c = c0;
  canvasIsOwned=false;
  label = new TLatex();
  label->SetNDC();
  gErrorIgnoreLevel=kWarning;
  Colors = PlotColors;
}

CanvasDrawer::~CanvasDrawer() {
//  delete c; c=0;
//  delete label; label=0;
}

void CanvasDrawer::SetCanvas(TCanvas *c0) {
  if (canvasIsOwned)
    delete c;
  c = c0;
  canvasIsOwned=false;
}

void CanvasDrawer::Reset() {
  internalAdds.clear();
  addsDrawn = false;
}

void CanvasDrawer::SplitCanvas(TCanvas *c_) {
  if (!c_ && !c) {
    c = new TCanvas();
    canvasIsOwned=true;
  }
  if (!c)
    c = c_;
  c->cd();

  pad1 = new TPad("pad1","pad1",0,0.3,1,1.0);
  pad1->SetBottomMargin(0);
  pad1->Draw();

  if (whichstyle==1) {
    pad2 = new TPad("pad2","pad2",0,0.05,1,0.3);
    pad2->SetTopMargin(0);
    pad2->SetBottomMargin(0.3);
  } else {
    pad2 = new TPad("pad2","pad2",0,0,1,0.3);
    pad2->SetTopMargin(0);
    pad2->SetBottomMargin(0.3);
    pad2->SetGridy(1);

  }
  pad2->Draw();
}

void CanvasDrawer::AddPlotLabel(const char *s, double x, double y, bool drawImmediately, int font, float textSize, int textAlign) {
  if (drawImmediately) {
    c->cd();
    if (textSize>0)
      label->SetTextSize(textSize);
    label->SetTextAlign(textAlign);
    label->SetTextFont(42);
    label->DrawLatex(x,y,s);
  } else {
    Label l(s,x,y,font,textSize,textAlign);
    plotLabels.push_back(l);
  }
}

void CanvasDrawer::SetTDRStyle(TString opt) {
    whichstyle=1;
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetCanvasColor(0); 
    double ratioHeight = (opt=="vbf") ? 700 : 720;
    gStyle->SetCanvasDefH(doRatio ? ratioHeight : 600); 
    gStyle->SetCanvasDefW(600); 
    gStyle->SetCanvasDefX(0);  
    gStyle->SetCanvasDefY(0);

    gStyle->SetPadBorderMode(0);
    gStyle->SetPadColor(0); 
    gStyle->SetPadGridX(0);
    gStyle->SetPadGridY(0);
    gStyle->SetGridColor(0);
    gStyle->SetGridStyle(3);
    gStyle->SetGridWidth(1);

    gStyle->SetFrameBorderMode(0);
    gStyle->SetFrameBorderSize(1);
    gStyle->SetFrameFillColor(0);
    gStyle->SetFrameFillStyle(0);
    gStyle->SetFrameLineColor(1);
    gStyle->SetFrameLineStyle(1);
    gStyle->SetFrameLineWidth(1);

    gStyle->SetHistMinimumZero();

    gStyle->SetOptStat(0);

    gStyle->SetPadTopMargin(0.07);
    gStyle->SetPadBottomMargin(0.13);
    gStyle->SetPadLeftMargin(0.15);
    gStyle->SetPadRightMargin(0.05);

    gStyle->SetOptTitle(0);

    gStyle->SetLabelColor(1, "XYZ");
    gStyle->SetLabelFont(42, "XYZ");
    gStyle->SetLabelOffset(0.007, "XYZ");
    gStyle->SetLabelSize(0.04, "XYZ");
    gStyle->SetTitleSize(0.05,"XYZ");

    gStyle->SetAxisColor(1, "XYZ");
    gStyle->SetStripDecimals(1); 
    gStyle->SetTickLength(0.025, "XYZ");
    gStyle->SetNdivisions(510, "XYZ");
    gStyle->SetPadTickX(1); 
    gStyle->SetPadTickY(1);

    gStyle->SetOptLogx(0);
    gStyle->SetOptLogy(0);
    gStyle->SetOptLogz(0);

    if (opt=="vbf") {
        whichstyle=3;
        gStyle->SetHistLineColor(1);
        gStyle->SetHistLineStyle(0);
        gStyle->SetHistLineWidth(1);
        gStyle->SetEndErrorSize(2);
        gStyle->SetFuncColor(2);
        gStyle->SetFuncStyle(1);
        gStyle->SetFuncWidth(1);
        gStyle->SetOptDate(0);
        gStyle->SetOptFile(0);
        gStyle->SetOptStat(0);
        gStyle->SetStatColor(0); 
        gStyle->SetStatFont(42);
        gStyle->SetStatFontSize(0.04);
        gStyle->SetStatTextColor(1);
        gStyle->SetStatFormat("6.4g");
        gStyle->SetStatBorderSize(1);
        gStyle->SetStatH(0.1);
        gStyle->SetStatW(0.15);
        gStyle->SetOptTitle(0);
        gStyle->SetTitleFont(42);
        gStyle->SetTitleColor(1);
        gStyle->SetTitleTextColor(1);
        gStyle->SetTitleFillColor(10);
        gStyle->SetTitleFontSize(0.05);
        gStyle->SetPaperSize(20.,20.);
        gStyle->SetPaintTextFormat(".2f");
    }

    TGaxis::SetMaxDigits(4);

    if (canvasIsOwned)
      delete c;

    c = new TCanvas();
    canvasIsOwned=true;

    if (opt=="vbf") {
      c->SetTickx(1);
      c->SetTicky(1);
      c->SetBottomMargin(0.3);
      c->SetRightMargin(0.06);

      Colors = VBFColors;
    }

}

void CanvasDrawer::SetRatioStyle() {
    whichstyle=2;
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetCanvasColor(0); 
    gStyle->SetCanvasDefH(300); 
    gStyle->SetCanvasDefW(600); 
    gStyle->SetCanvasDefX(0);  
    gStyle->SetCanvasDefY(0);

    gStyle->SetPadBorderMode(0);
    gStyle->SetPadColor(0); 
    gStyle->SetPadGridX(0);
    gStyle->SetPadGridY(0);
    gStyle->SetGridColor(0);
    gStyle->SetGridStyle(3);
    gStyle->SetGridWidth(1);

    gStyle->SetFrameBorderMode(0);
    gStyle->SetFrameBorderSize(1);
    gStyle->SetFrameFillColor(0);
    gStyle->SetFrameFillStyle(0);
    gStyle->SetFrameLineColor(1);
    gStyle->SetFrameLineStyle(1);
    gStyle->SetFrameLineWidth(1);

    gStyle->SetHistMinimumZero();

    gStyle->SetOptStat(0);

    gStyle->SetPadTopMargin(0.07);
    gStyle->SetPadBottomMargin(0.13);
    gStyle->SetPadLeftMargin(0.15);
    gStyle->SetPadRightMargin(0.05);

    gStyle->SetOptTitle(0);

    gStyle->SetLabelColor(1, "XYZ");
    gStyle->SetLabelFont(42, "XYZ");
    gStyle->SetLabelOffset(0.007, "XYZ");
    gStyle->SetLabelSize(0.04, "XYZ");
    gStyle->SetTitleSize(0.05,"XYZ");

    gStyle->SetAxisColor(1, "XYZ");
    gStyle->SetStripDecimals(1); 
    gStyle->SetTickLength(0.025, "XYZ");
    gStyle->SetNdivisions(510, "XYZ");
    gStyle->SetPadTickX(1); 
    gStyle->SetPadTickY(1);

    gStyle->SetOptLogx(0);
    gStyle->SetOptLogy(0);
    gStyle->SetOptLogz(0);

    TGaxis::SetMaxDigits(4);

    if (canvasIsOwned)
      delete c;

    c = new TCanvas();
    canvasIsOwned=true;

}

void CanvasDrawer::AddCMSLabel(double x, double y, TString subhead) {
  float textsize = doRatio ? 0.06 : 0.05;
  if (whichstyle==2) textsize = 0.06;
  float xshift = (whichstyle==2) ? 0.055 : 0.1;
  AddPlotLabel("CMS",x,y,false,62,textsize);
  AddPlotLabel(subhead.Data(),x+xshift,y,false,52,textsize); 
  //AddPlotLabel("CMS",.18,.85,false,62,textsize);
  //AddPlotLabel("Preliminary",.28,.85,false,52,textsize); 
}

void CanvasDrawer::AddLumiLabel(bool fb, double customLumi) {
  if (customLumi>0) 
    lumi = customLumi;
  float textsize = doRatio ? 0.05 : 0.05;
  TString units = (fb) ? "fb" : "pb";
  AddPlotLabel(TString::Format("%i %s^{-1} (13 TeV)",(int)lumi,units.Data()).Data(),0.95,0.94,false,42,0.8*textsize,31);
}

void CanvasDrawer::AddSqrtSLabel() {
  float textsize = doRatio ? 0.05 : 0.05;
  AddPlotLabel("(13 TeV)",0.95,0.94,false,42,0.8*textsize,31);
}

void CanvasDrawer::InitLegend(double x0, double y0, double x1, double y1, int ncolumns) { 
  delete legend;
  legend = new TLegend(x0,y0,x1,y1);
  legend->SetNColumns(ncolumns);
  legend->SetFillStyle(0);
  legend->SetBorderSize(0);
}

void CanvasDrawer::AddAdditional(TObject *o, TString opt, TString aname) {
  ObjWrapper w;
  w.o = o;
  w.opt = opt;
  w.label = aname;
  internalAdds.push_back(w);
}

void CanvasDrawer::Draw(TString outDir, TString baseName) {
  gSystem->mkdir(outDir,true);
  if (!addsDrawn) {
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
        PWarning("CanvasDrawer::Draw",TString::Format("Don't know what to do with %s",className.Data()));
        continue;
      }

      TString label = w.label;
      if (label.Length()>0 && legend) {
        legend->AddEntry(o,label,"l");
      }
    }
    addsDrawn = true;
  }
  if (legend) {
    //legend->SetTextSize(std::min((double)legend->GetTextSize(),0.05));
    //legend->SetTextSize(0.05);
    legend->Draw();
  }
  for (Label l : plotLabels) {
    float oldSize = label->GetTextSize();
    label->SetTextFont(l.font);
    if (l.size>0) 
      label->SetTextSize(l.size);
    label->SetTextAlign(l.align);
    label->DrawLatex(l.x,l.y,l.name);
    label->SetTextFont(42);
    label->SetTextSize(oldSize);
  }
  c->SaveAs(outDir+baseName+".png");
  c->SaveAs(outDir+baseName+".pdf");
//  c->SaveAs(outDir+baseName+".C");
  PInfo("CanvasDrawer::Draw",TString::Format("Created %s %s",outDir.Data(),baseName.Data()));
}
