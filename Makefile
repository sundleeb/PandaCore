# only generates ROOT dictionaries! compilation is left to SCRAM
PACKAGE=$(CMSSW_BASE)/src/PandaCore/

.PHONY: clean all

all: Tools/src/dictTools.cc Drawers/src/dictDrawers.cc Statistics/src/dictStats.cc Learning/src/dictLearn.cc

clean:
	rm -f */src/dict*.cc */src/dict*pcm

Tools/src/dictTools.cc: $(wildcard $(PACKAGE)/Tools/interface/*.h) $(PACKAGE)/Tools/LinkDef.h
	rootcling -f $(PACKAGE)/Tools/src/dictTools.cc $(CMSSW_BASE)/src/PandaCore/Tools/interface/*.h $(CMSSW_BASE)/src/PandaCore/Tools/LinkDef.h
	mkdir -p $(CMSSW_BASE)/lib/$(SCRAM_ARCH)/
	cp Tools/src/dictTools_rdict.pcm $(CMSSW_BASE)/lib/$(SCRAM_ARCH)/

Drawers/src/dictDrawers.cc: $(wildcard $(PACKAGE)/Drawers/interface/*.h) $(PACKAGE)/Drawers/LinkDef.h
	rootcling -f $(PACKAGE)/Drawers/src/dictDrawers.cc $(CMSSW_BASE)/src/PandaCore/Drawers/interface/*.h $(CMSSW_BASE)/src/PandaCore/Drawers/LinkDef.h
	mkdir -p $(CMSSW_BASE)/lib/$(SCRAM_ARCH)/
	cp Drawers/src/dictDrawers_rdict.pcm $(CMSSW_BASE)/lib/$(SCRAM_ARCH)/

Statistics/src/dictStats.cc: $(wildcard $(PACKAGE)/Statistics/interface/*.h) $(PACKAGE)/Statistics/LinkDef.h
	rootcling -f $(PACKAGE)/Statistics/src/dictStats.cc $(CMSSW_BASE)/src/PandaCore/Statistics/interface/*.h $(CMSSW_BASE)/src/PandaCore/Statistics/LinkDef.h
	mkdir -p $(CMSSW_BASE)/lib/$(SCRAM_ARCH)/
	cp Statistics/src/dictStats_rdict.pcm $(CMSSW_BASE)/lib/$(SCRAM_ARCH)/

Learning/src/dictLearn.cc: $(wildcard $(PACKAGE)/Learning/interface/*.h) $(PACKAGE)/Learning/LinkDef.h
	rootcling -f $(PACKAGE)/Learning/src/dictLearn.cc $(CMSSW_BASE)/src/PandaCore/Learning/interface/*.h $(CMSSW_BASE)/src/PandaCore/Learning/LinkDef.h
	mkdir -p $(CMSSW_BASE)/lib/$(SCRAM_ARCH)/
	cp Learning/src/dictLearn_rdict.pcm $(CMSSW_BASE)/lib/$(SCRAM_ARCH)/

