#ifndef SynchroSelector_h
#define SynchroSelector_h

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"

class TH1;
class RPCDetId;
class TrajectoryStateOnSurface;

namespace edm { class Event; class EventSetup; }
 

class SynchroSelector {
public:
  public: SynchroSelector( const edm::ParameterSet & cfg = edm::ParameterSet() );
  virtual ~SynchroSelector(){}

  void setHistoDxy(TH1* h) { hDxy=h; }
  void setHistoNum(TH1* h) { hNum=h; }
  void setHistoEta(TH1* h) { hDeltaEta=h; }
  void setHistoPhi(TH1* h) { hDeltaPhi=h; } 
  void setHistoPullX(TH1* h) { hPullX=h;}
  void setHistoDistX(TH1* h) { hDistX=h;}

  bool takeIt(const RPCDetId & det, const edm::Event&ev, const edm::EventSetup& es);

  void resetPos() { thePos.clear(); }
  std::vector<GlobalPoint> positions() { return thePos;}

private:
  bool checkL1RpcMatching( const TrajectoryStateOnSurface & tsos,  const edm::Event&ev, const edm::EventSetup& es);
  bool checkRpcDetMatching( const TrajectoryStateOnSurface & tsos,  const RPCDetId & det, const edm::Event&ev, const edm::EventSetup& es);
  bool checkUniqueRecHitMatching( const TrajectoryStateOnSurface & tsos,  const RPCDetId & det, const edm::Event&ev, const edm::EventSetup& es);
  
  
private:
  std::vector<GlobalPoint> thePos;
  edm::ParameterSet theConfig;
  TH1 *hDxy, *hNum, *hDeltaEta, *hDeltaPhi, *hPullX, *hDistX;
  float mindeta, mindphi;

};

#endif
