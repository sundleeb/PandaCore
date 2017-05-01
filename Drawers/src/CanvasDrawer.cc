#include "../interface/CanvasDrawer.h"
#include "TGaxis.h"

CanvasDrawer::CanvasDrawer(double x, double y) {
  if (x>0) {
    c = new TCanvas("c","c",x,y);
    canvasIsOwned=true;
  }
  label = new TLatex();
  label->SetNDC(); 
  gErrorIgnoreLevel=kWarning;
}

CanvasDrawer::CanvasDrawer(TCanvas *c0) {
  c = c0;
  canvasIsOwned=false;
  label = new TLatex();
  label->SetNDC();
  gErrorIgnoreLevel=kWarning;
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

  pad2 = new TPad("pad2","pad2",0,0.05,1,0.3);
  pad2->SetTopMargin(0);
  pad2->SetBottomMargin(0.3);
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

void CanvasDrawer::SetTDRStyle() {
    whichstyle=1;
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetCanvasColor(0); 
    gStyle->SetCanvasDefH(doRatio ? 720 : 600); 
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

void CanvasDrawer::AddCMSLabel(double x, double y) {
  float textsize = doRatio ? 0.06 : 0.05;
  if (whichstyle==2) textsize = 0.06;
  float xshift = (whichstyle==1) ? 0.1 : 0.055;
  AddPlotLabel("CMS",x,y,false,62,textsize);
  AddPlotLabel("Preliminary",x+xshift,y,false,52,textsize); 
  //AddPlotLabel("CMS",.18,.85,false,62,textsize);
  //AddPlotLabel("Preliminary",.28,.85,false,52,textsize); 
}

void CanvasDrawer::AddLumiLabel(bool fb, double customLumi) {
  if (customLumi>0) 
    lumi = customLumi;
  float textsize = doRatio ? 0.05 : 0.05;
  TString units = (fb) ? "fb" : "pb";
  AddPlotLabel(TString::Format("%.3g %s^{-1} (13 TeV)",(float)lumi,units.Data()).Data(),0.9,0.94,false,42,0.8*textsize,31);
}

void CanvasDrawer::InitLegend(double x0, double y0, double x1, double y1, int ncolumns) { 
  delete legend;
  legend = new TLegend(x0,y0,x1,y1);
  legend->SetNColumns(ncolumns);
  legend->SetFillStyle(0);
  legend->SetBorderSize(0);
}

void CanvasDrawer::Draw(TString outDir, TString baseName) {
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
