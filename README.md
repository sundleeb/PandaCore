# PandaCore 
A set of common tools on which several of my analysis packages depend

To build:
```bash
PandaCore/bin/genDict -j10 # must be every time a header file changes => builds ROOT dicts
scram b -j12 # use SCRAM to compile the code
```
