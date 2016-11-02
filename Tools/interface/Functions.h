#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "TLorentzVector.h" //ugh

#define PI 3.141592654
/*
 * This header simply has numerical functions that are used in many places
 * */

//////////////////////////////////////////////////////////////////////////////////

inline double Mxx(double pt1, double eta1, double phi1, double m1, double pt2, double eta2, double phi2, double m2) {
  TLorentzVector v1,v2;
  v1.SetPtEtaPhiM(pt1,eta1,phi1,m1);
  v2.SetPtEtaPhiM(pt2,eta2,phi2,m2);
  return (v1+v2).M();
}

inline double SignedDeltaPhi(double phi1, double phi2) {
  double dPhi = phi1-phi2;
  if (dPhi<-PI)
    dPhi = 2*PI+dPhi;
  else if (dPhi>PI)
    dPhi = -2*PI+dPhi;
  return dPhi;
}

inline double DeltaR2(double eta1, double phi1, double eta2, double phi2) {
  float dEta2 = (eta1-eta2); dEta2 *= dEta2;
  float dPhi = SignedDeltaPhi(phi1,phi2);
  return dEta2 + dPhi*dPhi;
}


#endif
