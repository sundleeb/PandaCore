# only generates ROOT dictionaries! compilation is left to SCRAM
PACKAGE=$(CMSSW_BASE)/src/PandaCore/

.PHONY: clean all

all: Tools/src/dict.cc Drawers/src/dict.cc Statistics/src/dict.cc Learning/src/dict.cc

clean:
	rm -f */src/dict.cc

Tools/src/dict.cc: $(wildcard $(PACKAGE)/Tools/interface/*.h) $(PACKAGE)/Tools/LinkDef.h
	rootcling -f $(PACKAGE)/Tools/src/dict.cc $(CMSSW_BASE)/src/PandaCore/Tools/interface/*.h $(CMSSW_BASE)/src/PandaCore/Tools/LinkDef.h
	cp Tools/src/dict_rdict.pcm $(CMSSW_BASE)/lib/$(SCRAM_ARCH)/

Drawers/src/dict.cc: $(wildcard $(PACKAGE)/Drawers/interface/*.h) $(PACKAGE)/Drawers/LinkDef.h
	rootcling -f $(PACKAGE)/Drawers/src/dict.cc $(CMSSW_BASE)/src/PandaCore/Drawers/interface/*.h $(CMSSW_BASE)/src/PandaCore/Drawers/LinkDef.h
	cp Drawers/src/dict_rdict.pcm $(CMSSW_BASE)/lib/$(SCRAM_ARCH)/

Statistics/src/dict.cc: $(wildcard $(PACKAGE)/Statistics/interface/*.h) $(PACKAGE)/Statistics/LinkDef.h
	rootcling -f $(PACKAGE)/Statistics/src/dict.cc $(CMSSW_BASE)/src/PandaCore/Statistics/interface/*.h $(CMSSW_BASE)/src/PandaCore/Statistics/LinkDef.h
	cp Statistics/src/dict_rdict.pcm $(CMSSW_BASE)/lib/$(SCRAM_ARCH)/

Learning/src/dict.cc: $(wildcard $(PACKAGE)/Learning/interface/*.h) $(PACKAGE)/Learning/LinkDef.h
	rootcling -f $(PACKAGE)/Learning/src/dict.cc $(CMSSW_BASE)/src/PandaCore/Learning/interface/*.h $(CMSSW_BASE)/src/PandaCore/Learning/LinkDef.h
	cp Learning/src/dict_rdict.pcm $(CMSSW_BASE)/lib/$(SCRAM_ARCH)/

