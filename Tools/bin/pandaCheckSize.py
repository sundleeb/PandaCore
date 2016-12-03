#!/usr/bin/env python

import PandaCore.Tools.Size as Size
import sys

checker = Size.SizeChecker()
checker.addFile('/afs/cern.ch/user/s/snarayan/eos/cms/store/group/phys_exotica/monotop/pandaprod/v_8022_0/MET/MET_Run2016C-23Sep2016-v1/161127_213446/0000/panda_10.root','MET')
checker.addFile('/afs/cern.ch/user/s/snarayan/eos/cms/store/group/phys_exotica/monotop/pandaprod/v_8022_0/ZJetsToNuNu_HT-200To400_13TeV-madgraph/ZJetsToNuNu_HT-200To400_13TeV-madgraph/161127_222156/0000/panda_10.root','Zvv')
checker.addFile('/afs/cern.ch/user/s/snarayan/eos/cms/store/group/phys_exotica/monotop/pandaprod/v_8022_0/TT_TuneCUETP8M1_13TeV-powheg-pythia8/TT_TuneCUETP8M1_13TeV-powheg-pythia8_ext4/161127_221215/0000/panda_10.root','TT')
checker.dumpInfo(True,'/afs/cern.ch/user/s/snarayan/www/pandasize/')
