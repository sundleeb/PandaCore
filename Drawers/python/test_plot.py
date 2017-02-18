#!/usr/bin/env python

import ROOT as root
import numpy as np
import root_numpy as rnp
import PandaAnalysis.Monotop.TestSelection as sel
from PandaCore.Tools.Misc import *
from PandaCore.Tools.Load import Load

'''
Load('Drawers','HistogramDrawer')
class testclass():
    def __init__(self):
        self.plot = root.HistogramDrawer()
        members = [x for x in dir(self.plot) if ('__' not in x)]
        for m in members:
            if callable(getattr(self.plot,m)):
                self.__getPlotMethod(m)
            else:
                self.__getPlotMember(m)
        return
    def __getPlotMethod(self,x):
        method = getattr(self.plot,x)
        setattr(self,x, lambda *args : method(*args))
    def __getPlotMember(self,x):
        member = getattr(self.plot,x)
        setattr(self,x, member)

test = testclass()
print dir(test)
test.SetLumi(10)
'''

xarr = rnp.root2array(['/data/t3home000/snarayan/store/panda/v_8024_4_3/TTbar.root'],'events',['fj1MSD/fj1Pt'],'fj1Pt>250')

print xarr['fj1MSD/fj1Pt'][:10]
