#!/usr/bin/env python
'''@package docstring
Just a giant list of processes and properties
'''

from processes.data import processes as data
from processes.BSM import processes as BSM
from processes.SM import processes as SM

processes = {}
for d in [data, BSM, SM]:
    processes.update(d)


if __name__=='__main__':
    import pprint
    pp = pprint.PrettyPrinter(width=160)
    for d in [data, BSM, SM]:
        print 
        pp.pprint(d)
