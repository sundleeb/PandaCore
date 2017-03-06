#!/usr/bin/env python
'''@package docstring
'''
from re import sub
from condor import classad,htcondor
import cPickle as pickle
import time
from os import getenv,getuid
from Misc import PInfo,PDebug,PWarning,PError
from collections import namedtuple

#############################################################
# DataSample and associated functions
#############################################################

class DataSample:
    def __init__(self,name,dtype,xsec):
        self.name = name
        self.dtype = dtype
        self.xsec = xsec
        self.files = []
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


#############################################################
# HTCondor interface for job submission and tracking
#############################################################


base_job_properties = {
    "Cmd" : "WORKDIR/exec.sh",
    "WhenToTransferOutput" : "ON_EXIT",
    "ShouldTransferFiles" : "YES",
    "Requirements" : classad.ExprTree('UidDomain == "mit.edu" && Arch == "X86_64" && OpSysAndVer == "SL6"'),
    "AcctGroup" : "group_t3mit.urgent",
    "X509UserProxy" : "/tmp/x509up_uUID",
    "OnExitHold" : classad.ExprTree("( ExitBySignal == true ) || ( ExitCode != 0 )"),
    "In" : "/dev/null",
    "TransferInput" : "WORKDIR/cmssw.tgz,WORKDIR/skim.py,WORKDIR/x509up",
}

job_status = {
           1:'idle',
           2:'running',
           3:'removed',
           4:'completed',
           5:'held',
           6:'transferring output',
           7:'suspended',
        }

# SubProperties = namedtuple('SubProperties',
#                            ['sub_id','cluster_id','submission_time',
#                             'sample_config'])

class Submission:
    def __init__(self,sample_configpath,cache_filepath):
        self.sample_config = read_sample_config(sample_configpath)
        self.configpath = sample_configpath
        self.cache_filepath = cache_filepath
        try: # figure out which (re-)submission attempt this is
            with open(cache_filepath,'rb') as fcache:
                self.sub_id = len(pickle.load(fcache))
        except:
            self.sub_id = 0
        self.submission_time = -1
        self.cluster_id = None # HTCondor ClusterID
        self.proc_ids = None # ProcID of each sample
        self.coll = htcondor.Collector()
        self.schedd = htcondor.Schedd(self.coll.locate(htcondor.DaemonTypes.Schedd,
                                                       "t3home000.mit.edu"))
        self.custom_job_properties = {}


    def execute(self,njobs=None):
        logdir = getenv('SUBMIT_LOGDIR')
        workdir = getenv('SUBMIT_WORKDIR')
        repl = { 'LOGDIR':logdir,
                 'WORKDIR':workdir,
                 'UID':str(getuid()),
                 'USER':getenv('USER'),
                 'SUBMITID':str(self.sub_id), }
        cluster_ad = classad.ClassAd()
        job_properties = base_job_properties.copy()
        job_properties['TransferInput'] += ',%s'%(self.configpath)
        for key,value in job_properties.iteritems():
            if type(value)==str:
                for pattern,target in repl.iteritems():
                    value = value.replace(pattern,target)
            cluster_ad[key] = value
        for key,value in self.custom_job_properties.iteritems():
            if type(value)==str:
                for pattern,target in repl.iteritems():
                    value = value.replace(pattern,target)
            cluster_ad[key] = value

        proc_properties = {
            "Arguments" : "PROCID SUBMITID",
            'UserLog' : 'LOGDIR/SUBMITID.log',
            'Out' : 'LOGDIR/SUBMITID_PROCID.out',
            'Err' : 'LOGDIR/SUBMITID_PROCID.err',
        }
        proc_id=0
        procs = []
        for name in sorted(self.sample_config):
            sample = self.sample_config[name]
            repl['PROCID'] = str(proc_id)
            proc_ad = classad.ClassAd()
            for key,value in proc_properties.iteritems():
                if type(value)==str:
                    for pattern,target in repl.iteritems():
                        value = value.replace(pattern,target)
                proc_ad[key] = value
            procs.append((proc_ad,1))
            proc_id += 1
            if njobs and proc_id>=njobs:
                break

        PInfo('Submission.execute','Submitting %i jobs!'%(len(procs)))
        self.submission_time = time.time()
        results = []
        self.cluster_id = self.schedd.submitMany(cluster_ad,procs,ad_results=results)
        self.proc_ids = {}
        for result,name in zip(results,sorted(self.sample_config)):
            self.proc_ids[result['ProcId']] = name
        PInfo('Submission.execute','Submitted to cluster %i'%(self.cluster_id))


    def query_status(self):
        if not self.cluster_id:
            PError("Submission.status",
                   "This submission has not been executed yet (ClusterId not set)")
        results = self.schedd.query(
            'Owner =?= "%s" && ClusterId =?= %i'%(getenv('USER'),self.cluster_id))
        jobs = {x:[] for x in ['running','idle','held','other']}
        for job in results:
            proc_id = job['ProcId']
            status = job['JobStatus']
            sample = self.sample_config[self.proc_ids[proc_id]]
            if job_status[status] in jobs:
                jobs[job_status[status]].append(sample)
            else:
                jobs['other'].append(sample)
        return jobs

    def __getstate__(self):
        odict = self.__dict__.copy()
        del odict['coll']
        del odict['schedd']
        return odict


    def __setstate__(self,odict):
        self.__dict__.update(odict)
        self.coll = htcondor.Collector()
        self.schedd = htcondor.Schedd(self.coll.locate(htcondor.DaemonTypes.Schedd,
                                                       "t3home000.mit.edu"))

    def save(self):
        try: 
            with open(self.cache_filepath,'rb') as fcache:
                cache = pickle.load(fcache)
        except:
            cache = []
        # props = SubProperties(self.sub_id,self.cluster_id,
        #                       self.submission_time,self.sample_config)
        cache.append(self)
        with open(self.cache_filepath,'wb') as fcache:
            pickle.dump(cache,fcache,2)
