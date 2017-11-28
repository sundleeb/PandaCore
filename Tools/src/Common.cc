#include "../interface/Common.h"


template <typename T> 
void concat(std::vector<T> &v1,std::vector<T> v2) {
  v1.insert(v1.end(),v2.begin(),v2.end());
}

void activateBranch(TTree *t, const char *bname, void *address) {
  t->SetBranchStatus(bname,1);
  t->SetBranchAddress(bname,address);
}


void PInfo(const char *module, const char *msg, const char *newline) {
  if (isatty(fileno(stdout)))
    fprintf(stdout,"\033[0;32mINFO\033[0m    [%-40s]: %s%s",module,msg,newline);
  else
    fprintf(stderr,"INFO    [%-40s]: %s%s",module,msg,newline); // redirect colorless output to stderr to preserve stream in log
}

void PDebug(const char *module, const char *msg, const char *newline) {
  if (isatty(fileno(stderr)))
    fprintf(stderr,"\033[0;36mDEBUG\033[0m   [%-40s]: %s%s",module,msg,newline);
  else
    fprintf(stderr,"DEBUG   [%-40s]: %s%s",module,msg,newline);
}

void PWarning(const char *module, const char *msg, const char *newline) {
  if (isatty(fileno(stdout)))
    fprintf(stdout,"\033[0;91mWARNING\033[0m [%-40s]: %s%s",module,msg,newline);
  else
    fprintf(stderr,"WARNING [%-40s]: %s%s",module,msg,newline);
}

void PError(const char *module, const char *msg, const char *newline) {
  if (isatty(fileno(stderr)))
    fprintf(stderr,"\033[0;41m\033[1;37mERROR\033[0m   [%-40s]: %s%s",module,msg,newline);
  else
    fprintf(stderr,"ERROR   [%-40s]: %s%s",module,msg,newline);
}

double getVal(TH1*h,double val) {
  return h->GetBinContent(h->FindBin(val));
}

double getVal(TH2D*h,double val1, double val2) {
  return h->GetBinContent(h->FindBin(val1,val2));
}

double getError(TH1*h,double val) {
  return h->GetBinError(h->FindBin(val));
}

double getError(TH2D*h,double val1, double val2) {
  return h->GetBinError(h->FindBin(val1,val2));
}

std::vector<TString> getDependencies(TString cut) {
  std::vector<TString> deps;
  int nChars = cut.Length();
  TString tmpString="";
  for (int iC=0; iC!=nChars; ++iC) {
    const char c = cut[iC];
    if ( c==' ' || c=='&' || c=='|' || c=='(' || c==')'
        || c=='*' || c=='+' || c=='-' || c=='/' || c=='!'
        || c=='<' || c=='>' || c=='=' || c=='.' || c==','
        || c=='[' || c==']' || c==':') {
      if (tmpString != "" && !tmpString.IsDigit() &&
          // tmpString!="Pt" && tmpString!="Eta" && tmpString!="Phi" &&
          !(tmpString.Contains("TMath")) && !(tmpString=="fabs")) {
        deps.push_back(tmpString);
      }
      tmpString = "";
    } else {
        tmpString.Append(c);
    }
  }
  if (tmpString != "" && !tmpString.IsDigit() &&
      // tmpString!="Pt" && tmpString!="Eta" && tmpString!="Phi" &&
      !tmpString.Contains("TMath")) {
    deps.push_back(tmpString);
  }
  return deps;
}

ProgressReporter::ProgressReporter(const char *n, unsigned int *iE, unsigned int *nE, unsigned int nR) 
{
  name = n; name+="::Progress";
  idx = iE;
  N = nE;
  frequency = nR;
}

void ProgressReporter::Report() {
  float progress = 1.*(*idx)/(*N);
  if ( progress >= threshold) {
    PInfo(name.Data(),
        TString::Format("%-40s",TString::Format("%5.2f%% (%u/%u)      ",progress*100,*idx,*N).Data()).Data(),
        // "\r");
         "\n");
    threshold += 1./frequency;
  }
}


TimeReporter::TimeReporter(TString n, int on_) 
{
  on=on_;
  name = n; name += "::Time";
  sw = new TStopwatch();
  subsw = new TStopwatch();
}

TimeReporter::~TimeReporter() { delete sw; delete subsw; }

void 
TimeReporter::Start() 
{
  if (on) {
    sw->Start(true);
    subsw->Start(true);
  }
  currentEvent=0;
  currentSubEvent=1;
}

void 
TimeReporter::TriggerEvent(TString s,bool reset) 
{
  if (!on)
    return;
  currentSubEvent=1;
  double interval = sw->RealTime()*1000;
  if (on > 1) 
    PDebug(name,TString::Format("%2i   : %.3f (%s)",currentEvent,interval,s.Data()).Data());
  if (s.Contains("GetEntry"))
    s = "GetEntry";
  if (totalTime.find(s) == totalTime.end()) {
    totalTime[s] = 0;
    nCalls[s] = 0;
    callOrders.push_back(s);
  } else { // always ignore the first call, frequently initialization is slow
    nCalls[s] += 1;
    totalTime[s] += interval;
  }
  sw->Start();
  subsw->Start();
  if (reset)
    currentEvent+=1;
}

void 
TimeReporter::TriggerSubEvent(TString s) 
{
  if (!on)
    return;
  double interval = subsw->RealTime()*1000;
  if (on > 1)
    PDebug(name,
        TString::Format("%2i.%-2i: %.3f (%s)",currentEvent,currentSubEvent,interval,s.Data()).Data());
  if (totalTime.find(s) == totalTime.end()) {
    totalTime[s] = 0;
    nCalls[s] = 0;
    callOrders.push_back(s);
  } else { 
    nCalls[s] += 1;
    totalTime[s] += interval;
  }
  currentSubEvent+=1;
  subsw->Start();
}

void 
TimeReporter::Summary()
{
  if (!on)
    return;
  for (TString s : callOrders) {
    PDebug(name,
        TString::Format("Task %20s called %5u times, average time = %.3f",
          s.Data(), nCalls[s], totalTime[s]/nCalls[s]));
  }
}
