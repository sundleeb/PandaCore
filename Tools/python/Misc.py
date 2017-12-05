#!/usr/bin/env python
'''@package docstring
Some common python functions
'''

from re import sub
from sys import stdout,stderr
from os import getenv
from collections import namedtuple

_atty_out = stdout.isatty()
_atty_err = stderr.isatty()

def PInfo(module,msg,newline="\n"):
    ''' function to write to stdout'''
    if _atty_out:
        stdout.write('\033[0;32mINFO\033[0m    [%-40s]: %s%s'%(module,msg,newline))
    else:
        stderr.write('INFO    [%-40s]: %s%s'%(module,msg,newline)) # redirect color-less output to stderr to maintain stream in log files

def PWarning(module,msg,newline="\n"):
    ''' function to write to stdout'''
    if _atty_out:
        stdout.write('\033[0;91mWARNING\033[0m [%-40s]: %s%s'%(module,msg,newline))
    else:
        stderr.write('WARNING [%-40s]: %s%s'%(module,msg,newline))

def PDebug(module,msg,newline="\n"):
    ''' function to write to stdout'''
    if _atty_err:
        stderr.write('\033[0;36mDEBUG\033[0m   [%-40s]: %s%s'%(module,msg,newline))
    else:
        stderr.write('DEBUG   [%-40s]: %s%s'%(module,msg,newline))

def PError(module,msg,newline="\n"):
    ''' function to write to stdout'''
    if _atty_err:
        stderr.write('\033[0;41m\033[1;37mERROR\033[0m   [%-40s]: %s%s'%(module,msg,newline))
    else:
        stderr.write('ERROR   [%-40s]: %s%s'%(module,msg,newline))


def smart_getenv(v, d=None):
    val = getenv(v)
    if val is None:
        return d
    return val


ModelParams = namedtuple('ModelParams',['m_V','m_DM','gV_DM','gA_DM','gV_q','gA_q','sigma','delta'])

def read_nr_model(mV,mDM,couplings=None,path='non-resonant'):
    tmpl = getenv('PANDA_XSECS')+'/'+path+'/%i_%i_xsec_gencut.dat'
    try:
        fdat = open(tmpl%(mV,mDM))
    except IOError:
        PError('PandaCore.Tools.Misc.read_nr_model','Could not open %s'%(tmpl%(mV,mDM)))
        return None
    for line in fdat:
        if 'med dm' in line:
            continue
        p = ModelParams(*[float(x) for x in line.strip().split()])
        if couplings:
            if couplings==(p.gV_DM,p.gA_DM,p.gV_q,p.gA_q):
                fdat.close()
                return p
            else:
                continue
        else:
            # if not specified, take the first valid model (nominal)
            fdat.close()
            return p

def read_r_model(mV,mDM=100,couplings='nominal'):
    tmpl = getenv('PANDA_XSECS')+'/resonant/%i_%i.dat'
    try:
        fdat = open(tmpl%(mV,mDM))
    except IOError:
        PError('PandaCore.Tools.Misc.read_nr_model','Could not open %s'%(tmpl%(mV,mDM)))
        return None
    for line in fdat:
        line_coupling,sigma = line.split(':')
        if not(line_coupling==couplings):
            continue
        sigma = float(sigma)
        p = ModelParams(mV,mDM,1,1,0.25,0.25,sigma,0)
        fdat.close()
        return p 


def setBins(dist,bins):
    ''' Given a list of bin edges, sets them for a PlotUtility::Distribution DEPRECATED'''
    for b in bins:
        dist.AddBinEdge(b)

def tAND(s1,s2):
    ''' ANDs two strings '''
    if s1 and s2:
        return "(( "+s1+" ) && ( "+s2+" ))"
    if not s1:
        return s2
    if not s2:
        return s1

def tOR(s1,s2):
    ''' ORs two strings'''
    if s1 and s2:
        return "(( "+s1+" ) || ( "+s2+" ))"
    if not s1:
        return s2
    if not s2:
        return s1

def tTIMES(w,s):
    ''' MULTIPLIES two strings'''
    if w and s:
        return "( "+w+" ) * ( "+s+" )"
    if not w:
        return s
    if not s:
        return w

def tNOT(w):
    ''' NOTs two strings'''
    return '!( '+w+' )'

def removeCut(basecut,var):
    ''' 
    Removes the dependence on a particular variable from a formula

    @type basecut: str
    @param basecut: Input formula to modify

    @type var: str
    @param var: Variable to remove from basecut

    @rtype: string
    @return: Returns a formula with the var dependence removed
    '''
    return sub('[0-9\.]*[=<>]*%s'%(var.replace('(','\(').replace(')','\)')),
               '1==1',
                 sub('%s[=<>]+[0-9\.]+'%(var.replace('(','\(').replace(')','\)')),
                     '1==1',
                     basecut)
                 )

