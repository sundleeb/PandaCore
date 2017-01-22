#ifndef ROO_EXPERF
#define ROO_EXPERF

#include "RooAbsPdf.h"
#include "RooRealProxy.h"
#include "PandaCore/Tools/interface/Functions.h"

class RooRealVar;

class RooExpErf : public RooAbsPdf {
public:
  RooExpErf() {}
  RooExpErf(const char *name, const char *title, RooAbsReal& _x,
            RooAbsReal& _a, RooAbsReal& _b, RooAbsReal& _c);

  RooExpErf(const RooExpErf& other, const char* name = 0);
  virtual TObject* clone(const char* newname) const { return new RooExpErf(*this,newname); }

  inline virtual ~RooExpErf() { }

protected:

  RooRealProxy x;
  RooRealProxy a;
  RooRealProxy b;
  RooRealProxy c;
  Double_t evaluate() const;
private:
  ClassDef(RooExpErf,1) 
};

#endif
