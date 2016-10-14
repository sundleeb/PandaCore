#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define PI 3.141592654
/*
 * This header simply has numerical functions that are used in many places
 * */

//////////////////////////////////////////////////////////////////////////////////

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
