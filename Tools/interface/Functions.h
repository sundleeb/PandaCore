#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "TLorentzVector.h" 
#include "TMath.h"

#define PI 3.141592654

/** \file Functions.h
 * \brief Some numerical functions
 * \author S. Narayanan
 */

//////////////////////////////////////////////////////////////////////////////////

/**
 * \param x	value to be cleaned
 * \param d	default value
 * \brief Returns a default value if input is NaN
 */
double clean(double x, double d=-1);

/**
 * \param val	input value
 * \param low	low value
 * \param high	high value
 * \brief Bounds a value within a range
 */
double bound(double val, double low, double high);

template <typename T>
int sign(T x) {
    return (x<0) ? -1 : 1;
}

// need a non-templated instance to be able to 
// load in interactive ROOT => use in TTreeFormula
int dsign(double x);

/**
 * \brief Mass of a pair of particles
 */
double Mxx(double pt1, double eta1, double phi1, double m1, double pt2, double eta2, double phi2, double m2);

double MT(double pt1, double phi1, double pt2, double phi2);

/**
 * \brief Signed delta-phi
 */
double SignedDeltaPhi(double phi1, double phi2);

/**
 * \brief Calculates the delta-R-squared metric
 */
double DeltaR2(double eta1, double phi1, double eta2, double phi2);

/**
 * \brief Exponential times erf, aka CMSShape
 */
double ExpErf(double x, double a, double b, double c);

#endif
