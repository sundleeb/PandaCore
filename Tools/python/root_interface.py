'''PandaCore.Tools.root_interface

Some tools defining interfaces between ROOT and other data structures.
Essentially wrappers around root_numpy
''' 

import numpy as np 
import root_numpy as rnp 
import ROOT as root
from Misc import PInfo,  PWarning,  PError

def rename_dtypes(xarr, repl, old_names = None):
    if old_names:
        for n in xarr.dtypes.names:
            old_names.append(n)
    new_names  =  tuple((repl[x] for x in xarr.dtypes.names))
    xarr.dtypes.names  =  new_names

# FILE INPUT ------------------------------------------------------------
def read_branches(filenames, tree, branches, cut, treename = "events", xargs = ()):
    if not(filenames or treename) or (filenames and tree):
        PError("root_interface.read_branches", "Exactly one of filenames and tree should be specified!")
        return None
    branches_  =  list(set(branches)) # remove duplicates
    if filenames:
        return rnp.root2array(filenames = filenames, 
                              treename = treename, 
                              branches = branches_, 
                              selection = cut, 
                              *xargs)
    else:
        return rnp.tree2array(tree = tree, 
                              branches = branches_, 
                              selection = cut, 
                              *xargs)


def read_file(filenames, branches, cut = None, treename = 'events', xargs = ()):
    return read_branches(filenames = filenames, 
                         tree = None, 
                         branches = branches, 
                         cut = cut, 
                         treename = treename, 
                         xargs = xargs)

def read_tree(tree, branches, cut = None, xargs = ()):
    return read_branches(filenames = None, 
                         tree = tree,  
                         branches = branches,  
                         cut = cut, 
                         xargs = xargs)


# FILE OUTPUT --------------------------------------------------------------
def array_as_tree(xarr, branches = None, treename = None, fcontext = None, xargs = ()):
    # combines array2tree and array2root but leaves TFile manipulation for the user
    if fcontext:
        context  =  root.TDirectory.TContext(fcontext)
    tree  =  rnp.array2tree(xarr, branches, *xargs)
    if fcontext:
        fcontext.WriteTObject(tree, treename)
    return tree


# HISTOGRAM MANIPULATION ---------------------------------------------------
def draw_hist(hist, xarr, fields, weight = None):
    warr  =  xarr[weight] if weight else None
    if len(fields) =  = 1:
        return rnp.fill_hist(hist = hist, array = xarr[fields[0]], warr)
    else:
        varr  =  np.array([xarr[f] for f in fields])
        varr  =  varr.transpose()
        return rnp.fill_hist(hist = hist, array = varr, warr)
