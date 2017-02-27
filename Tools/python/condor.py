#!/usr/bin/env python

files = {}

try:
    import htcondor
    import classad
except ImportError:
    import imp
    local_path = ['/usr/lib64/python2.6/site-packages/']
    def __get_module(name):
        global files
        found = imp.find_module(name,local_path)
        files[name] = found[0]
        return imp.load_module(name,*found)
    classad = __get_module('classad') 
    htcondor = __get_module('htcondor') 

def __close_files():
    global files
    for k,v in files.iteritems():
        v.close()

import atexit
atexit.register(__close_files)
