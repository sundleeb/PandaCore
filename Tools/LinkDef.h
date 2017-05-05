#include "PandaCore/Tools/interface/Common.h"
#include "PandaCore/Tools/interface/Functions.h"
#include "PandaCore/Tools/interface/TreeTools.h"
#include "PandaCore/Tools/interface/DuplicateRemover.h"
#include "PandaCore/Tools/interface/Normalizer.h"
#include "PandaCore/Tools/interface/Cutter.h"
#include "PandaCore/Tools/interface/BranchAdder.h"
#include "PandaCore/Tools/interface/EventSyncher.h"

#ifdef __CLING__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclass;
#pragma link C++ nestedtypedef;

#pragma link C++ struct EventObj;
#pragma link C++ enum SmearShift;

#pragma link C++ class DuplicateRemover;
#pragma link C++ class Normalizer;
#pragma link C++ class JERReader;
#pragma link C++ class Cutter;
#pragma link C++ class EventSyncher;
#pragma link C++ class BranchAdder;
#pragma link C++ class ProgressReporter;
#pragma link C++ class TimeReporter;
#pragma link C++ class Binner;

// numerical functions not bound to a class
#pragma link C++ function bound;
#pragma link C++ function clean;
#pragma link C++ function sign;
#pragma link C++ function dsign;
#pragma link C++ function Mxx;
#pragma link C++ function MT;
#pragma link C++ function SignedDeltaPhi;
#pragma link C++ function DeltaR2;
#pragma link C++ function ExpErf;

#endif 
