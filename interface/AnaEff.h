#ifndef UserCode_L1RpcTriggerAnalysis_AnaEff_H
#define UserCode_L1RpcTriggerAnalysis_AnaEff_H

#include "TObject.h"

class TObjArray;
class TH1D;
class TrackObj;
class HitSpecObj;
class L1ObjColl;
class L1Obj;
class TTree;
class TFile;
class EventData;

#include <vector>
#include <map>
#include <string>

#include "FWCore/ParameterSet/interface/ParameterSet.h"

class AnaEff {
public: 
  AnaEff(const edm::ParameterSet & cfg) :  debug(false), theConfig(cfg) {}
  ~AnaEff();
  void init(TObjArray& histos); 
  void run(const TrackObj* muon, const L1ObjColl *l1Coll, const HitSpecObj * hitSpec);

  bool debug;

private:
  double maxPt(const std::vector<L1Obj> & l1Objs) const; 
  TH1D* hEfficMuPt_D, *hEfficRpcNoCut_N, *hEfficRpcPtCut_N;
  std::map< std::string, TH1D* > hm;

  const static unsigned int nPtCuts= 30;
  const static double ptCuts[];

  edm::ParameterSet theConfig;

  TFile *file;
  TTree *tree;
  EventData *myEvent;

};
#endif
