#!/usr/bin/env python
'''@package docstring
Just a giant list of processes and properties
'''

from processes.data import processes as data
from processes.BSM import processes as BSM
from processes.SM_Leptonic import processes as SM_Leptonic

processes = {}
for d in [data,BSM,SM_Leptonic]:
    processes.update(d)


if __name__=='__main__':
    import pprint
    pp = pprint.PrettyPrinter(width=160)
    for d in [data,BSM,SM_Leptonic]:
        print 
        pp.pprint(d)
