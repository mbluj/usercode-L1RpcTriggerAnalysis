#ifndef TrackToL1ObjMatcher_H
#define TrackToL1ObjMatcher_H

#include "FWCore/ParameterSet/interface/ParameterSet.h"
namespace edm { class Event; class EventSetup; }
namespace reco { class Muon; }
class TrajectoryStateOnSurface;

class TrackToL1ObjMatcher {
public:
  struct LastResult { bool isValid; float deltaEta, deltaPhi; };
public:
  TrackToL1ObjMatcher(const edm::ParameterSet & cfg);
  bool operator()(float l1Eta, float l1Phi, const reco::Muon *mu,  const edm::Event&ev, const edm::EventSetup& es) const;
  bool operator()(float l1Eta, float l1Phi, const TrajectoryStateOnSurface & tsos,  const edm::Event&ev, const edm::EventSetup& es) const;
  const LastResult & lastResult() const { return theLastResult; }
private:
  bool compare(float l1Eta, float l1Phi, float stateEta, float statePhi) const;
  edm::ParameterSet theConfig;
  mutable LastResult theLastResult;
};
#endif 
