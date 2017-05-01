#!/usr/bin/env python
'''@package docstring
'''
from re import sub
from condor import classad,htcondor
import cPickle as pickle
import time
from os import getenv,getuid,system,path,environ
from Misc import PInfo,PDebug,PWarning,PError
from collections import namedtuple
from sys import exit 

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


#############################################################
# HTCondor interface for job submission and tracking
#############################################################


base_job_properties = {
    "Cmd" : "WORKDIR/exec.sh",
    "WhenToTransferOutput" : "ON_EXIT",
    "ShouldTransferFiles" : "YES",
    "Requirements" : classad.ExprTree('UidDomain == "mit.edu" && Arch == "X86_64" && OpSysAndVer == "SL6"'),
    "AcctGroup" : "group_t3mit.urgent",
    "AccountingGroup" : "group_t3mit.urgent.snarayan",
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

schedd_server ='t3home000.mit.edu'

def environ_to_condor():
    s = '' 
    for k,v in environ.iteritems():
        s += '%s=%s '%(k,v)
    return s 

class _BaseSubmission(object):
    def __init__(self, cache_filepath):
        self.cache_filepath = cache_filepath
        try: # figure out which (re-)submission attempt this is
            with open(cache_filepath,'rb') as fcache:
                self.sub_id = len(pickle.load(fcache))
        except:
            self.sub_id = 0
        self.submission_time = -1
        self.cluster_id = None # HTCondor ClusterID
        self.proc_ids = None # ProcID of each job
        self.coll = htcondor.Collector()
        self.schedd = htcondor.Schedd(self.coll.locate(htcondor.DaemonTypes.Schedd,
                                                       schedd_server))
        self.custom_job_properties = {}


    def query_status(self):
        if not self.cluster_id:
            PError(self.__name__+".status",
                   "This submission has not been executed yet (ClusterId not set)")
        results = self.schedd.query(
            'Owner =?= "%s" && ClusterId =?= %i'%(getenv('USER'),self.cluster_id))
        jobs = {x:[] for x in ['running','idle','held','other']}
        for job in results:
            proc_id = job['ProcId']
            status = job['JobStatus']
            sample = self.arguments[self.proc_ids[proc_id]]
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
                                                       schedd_server))


    def save(self):
        try: 
            with open(self.cache_filepath,'rb') as fcache:
                cache = pickle.load(fcache)
        except:
            cache = []
        cache.append(self)
        with open(self.cache_filepath,'wb') as fcache:
            pickle.dump(cache,fcache,2)


class SimpleSubmission(_BaseSubmission):
    def __init__(self,cache_dir,executable=None,arguments=None):
        super(SimpleSubmission,self).__init__(cache_dir+'/cache.pkl')
        self.__name__='SimpleSubmission'
        self.cache_dir = cache_dir
        if executable and arguments:
            self.executable = executable
            self.arguments = arguments
        else:
            try:
                pkl = pickle.load(open(self.cache_filepath))
                last_sub = pkl[-1]
                self.executable = last_sub.executable
                self.arguments = last_sub.arguments
            except:
                PError(self.__name__+'.__init__',
                       'Must provide a valid cache or arguments!')
                exit(1)
        self.cmssw = getenv('CMSSW_BASE')
        self.workdir = cache_dir + '/workdir/'
        self.logdir = cache_dir + '/logdir/'
        for d in [self.workdir,self.logdir]:
            system('mkdir -p '+d)
    def execute(self,njobs=None):
        self.submission_time = time.time()
        runner = '''
#!/bin/bash
cd {0} 
cmsenv 
{1} $@ && echo $@ >> {2}'''.format(self.cmssw,self.executable,self.workdir+'/progress.log')
        with open(self.workdir+'exec.sh','w') as frunner:
            frunner.write(runner)
        repl = {'WORKDIR' : self.workdir,
                'LOGDIR' : self.logdir,
                'SUBMIT_ID' : str(self.sub_id)}
        cluster_ad = classad.ClassAd()

        job_properties = base_job_properties.copy()
        for k in ['X509UserProxy','TransferInput']:
            del job_properties[k]
        job_properties['Environment'] = environ_to_condor()
        for key,value in job_properties.iteritems():
            if type(value)==str:
                for pattern,target in repl.iteritems():
                    value = value.replace(pattern,target)
            cluster_ad[key] = value
        print cluster_ad 

        proc_properties = {
            'UserLog' : 'LOGDIR/SUBMITID.log',
            'Out' : 'LOGDIR/SUBMITID_PROCID.out',
            'Err' : 'LOGDIR/SUBMITID_PROCID.err',
        }
        proc_id=0
        procs = []
        for idx in sorted(self.arguments):
            args = self.arguments[idx]
            repl['PROCID'] = '%i'%idx
            proc_ad = classad.ClassAd()
            for key,value in proc_properties.iteritems():
                if type(value)==str:
                    for pattern,target in repl.iteritems():
                        value = value.replace(pattern,target)
                proc_ad[key] = value
            proc_ad['Arguments'] = args 
            procs.append((proc_ad,1))
            proc_id += 1
            if njobs and proc_id>=njobs:
                break

        PInfo(self.__name__+'.execute','Submitting %i jobs!'%(len(procs)))
        self.submission_time = time.time()
        results = []
        self.cluster_id = self.schedd.submitMany(cluster_ad,procs,ad_results=results)
        self.proc_ids = {}
        for result,idx in zip(results,sorted(self.arguments)):
            self.proc_ids[result['ProcId']] = idx
        PInfo(self.__name__+'.execute','Submitted to cluster %i'%(self.cluster_id))

    def check_missing(only_failed=True):
        finished = map(lambda x : x.strip(), open(self.workdir+'/progress.log').readlines())
        status = self.query_status()
        missing = {}
        for idx,args in self.arguments.iteritems():
            if args in finished:
                continue 
            if only_failed and any([args in status[x] for x in ['running','idle']]):
                continue 
            missing[idx] = args 
        return missing 


class Submission(_BaseSubmission):
    '''Submission 
    
    This class is used specifically for heavy analysis-specific submission
    with robust re-packaging and re-submission of failures.
    
    Extends:
        _BaseSubmission
    '''
    def __init__(self,sample_configpath,cache_filepath):
        super(Submission,self).__init__(cache_filepath)
        self.__name__='Submission'
        self.arguments = read_sample_config(sample_configpath)
        self.configpath = sample_configpath
        

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
        print cluster_ad

        proc_properties = {
            "Arguments" : "PROCID SUBMITID",
            'UserLog' : 'LOGDIR/SUBMITID.log',
            'Out' : 'LOGDIR/SUBMITID_PROCID.out',
            'Err' : 'LOGDIR/SUBMITID_PROCID.err',
        }
        proc_id=0
        procs = []
        for name in sorted(self.arguments):
            sample = self.arguments[name]
            repl['PROCID'] = '%i'%sample.get_id() #name.split('_')[-1] 
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
        for result,name in zip(results,sorted(self.arguments)):
            self.proc_ids[result['ProcId']] = name
#            print 'Mapping %i->%s'%(result['ProcId'],name)
        PInfo('Submission.execute','Submitted to cluster %i'%(self.cluster_id))

