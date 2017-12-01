#!/usr/bin/env python
from re import sub
import cPickle as pickle
import time
from os import getenv,getuid,system,path,environ
from Misc import PInfo,PDebug,PWarning,PError
from collections import namedtuple
from sys import exit 
# module *must* remain independent of htcondor to run on T2

#############################################################
# DataSample and associated functions
#############################################################

class DataSample:
    def __init__(self,name,dtype,xsec):
        self.name = name
        self.dtype = dtype
        self.xsec = xsec
        self.files = []
    def get_id(self):
        try:
            return int(self.name.split('_')[-1])
        except:
            return -1
    def add_file(self,fname):
        self.files.append(fname)
    def get_config(self,nfiles,suffix=''):
        nfiles = int(nfiles)
        rlist = []
        nall_files = len(self.files)
        if nfiles<0:
            nfiles = nall_files+1
        for i in xrange(nall_files/nfiles+1):
            rstr = '[CONFIG%s]\n'%(suffix)
            rstr += '{0:<25} {1:<10} {2:<15}\n'.format('%s%s'%(self.name,suffix),self.dtype,self.xsec)
            for f in self.files[i*nfiles:min((i+1)*nfiles,nall_files)]:
                rstr += '\t%s\n'%f
            rlist.append(rstr)
        return rlist

def read_sample_config(fpath,as_dict=True):
    f = open(fpath)
    samples = []
    class State:
        def __init__(self):
            return
    NULL,CONFIG,DATASET,FILE = [State() for _ in xrange(4)]
    state=NULL
    current_sample = None
    for line in f:
        if "CONFIG" in line:
            if current_sample:
                samples.append(current_sample)
            state=CONFIG
            continue
        if state==CONFIG:
            ll = line.split()
            current_sample = DataSample(ll[0],ll[1],float(ll[2]))
            state=DATASET
            continue
        if state==DATASET or state==FILE:
            ll = line.strip()
            current_sample.add_file(ll)
            state=FILE
    if state==FILE:
        samples.append(current_sample)
    
    if as_dict:
        return { x.name:x for x in samples }
    else:
        return samples

def merge_config_samples(sample_list,as_dict=True):
    if type(sample_list)==dict:
        sample_list = [v for k,v in sample_list.iteritems()]
    samples = {}
    for s in sample_list:
        rename = sub('_[0-9]+$','',s.name)
        if rename not in samples:
            samples[rename] = DataSample(rename,s.dtype,s.xsec)
        for f in s.files:
            samples[rename].add_file(f)
    if as_dict:
        return samples
    else:
        return [v for k,v in samples.iteritems()]

def convert_catalog(file_list,as_dict=True):
    samples = {}
    for line in file_list:
        ll = line.split()
        sample_name = sub('_[0-9]+$','',ll[0])
        if sample_name not in samples:
            samples[sample_name] = DataSample(sample_name,ll[1],float(ll[2]))
        samples[sample_name].add_file(ll[3])
    if as_dict:
        return samples
    else:
        return [v for k,v in samples.iteritems()]


