#include "RooFit.h"

#include "Riostream.h"
#include <math.h>

#include "../interface/RooExpErf.h"
#include "RooAbsReal.h"
#include "RooRealVar.h"
#include "RooMath.h"
#include "TMath.h"

//ClassImp(RooExpErf);


//_____________________________________________________________________________
RooExpErf::RooExpErf(const char *name, const char *title, RooAbsReal& _x, 
                      RooAbsReal& _a, RooAbsReal& _b, RooAbsReal& _c) :
       RooAbsPdf(name, title),
       x("x", "Dependent", this, _x),
       a("a", "a", this, _a),
       b("b", "b", this, _b),
       c("c", "c", this, _c)
{
}


//_____________________________________________________________________________
RooExpErf::RooExpErf(const RooExpErf& other, const char* name) :
      RooAbsPdf(other, name), 
      x("x", this, other.x), 
      a("a", this, other.a), 
      b("b", this, other.b), 
      c("c", this, other.c)
{
}

//_____________________________________________________________________________
Double_t RooExpErf::evaluate() const {

  return ExpErf(x,a,b,c);
  
}

