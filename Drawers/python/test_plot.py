#!/usr/bin/env python

import ROOT as root
import numpy as np
import root_numpy as rnp
import PandaAnalysis.Monotop.TestSelection as sel
from PandaCore.Tools.Misc import *
from PandaCore.Tools.Load import Load
import inspect

Load('Drawers','HistogramDrawer')
class testclass():
    def __init__(self):
        self.plot = root.HistogramDrawer()
        self.plot_methods = {}
        methods = [x for x in dir(self.plot) if ('__' not in x)]
        for m in methods:
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

