#include "../interface/Functions.h"

double clean(double x, double d) {
  return TMath::Finite(x) ? x : d;
}

double bound(double val, double low, double high) {
  return TMath::Max(low,TMath::Min(high,val));
}

int dsign(double x) {
    return sign(x);
}

double Mxx(double pt1, double eta1, double phi1, double m1, double pt2, double eta2, double phi2, double m2) {
    TLorentzVector v1,v2;
    v1.SetPtEtaPhiM(pt1,eta1,phi1,m1);
    v2.SetPtEtaPhiM(pt2,eta2,phi2,m2);
    return (v1+v2).M();
}

double MT(double pt1, double phi1, double pt2, double phi2)
{
    TLorentzVector v1,v2;
    v1.SetPtEtaPhiM(pt1,0,phi1,0);
    v2.SetPtEtaPhiM(pt2,0,phi2,0);
    return (v1+v2).M();
}

double SignedDeltaPhi(double phi1, double phi2) {
    double dPhi = phi1-phi2;
    if (dPhi<-PI)
        dPhi = 2*PI+dPhi;
    else if (dPhi>PI)
        dPhi = -2*PI+dPhi;
    return dPhi;
}

double DeltaR2(double eta1, double phi1, double eta2, double phi2) {
    float dEta2 = (eta1-eta2); dEta2 *= dEta2;
    float dPhi = SignedDeltaPhi(phi1,phi2);
    return dEta2 + dPhi*dPhi;
}

double ExpErf(double x, double a, double b, double c) {
    double exp_ = TMath::Exp(c*x);
    double erf_ = TMath::Erf((x-a)/b);
    return exp_*(1+erf_)/2;
}

