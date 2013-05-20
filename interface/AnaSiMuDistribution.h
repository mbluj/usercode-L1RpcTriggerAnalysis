#ifndef UserCode_L1RpcTriggerAnalysis_AnaSiMuDistribution_H
#define UserCode_L1RpcTriggerAnalysis_AnaSiMuDistribution_H


class TObjArray;
class HitSpecObj;
class TrackObj;
class EventObj;
#include <vector>
#include <cstdint>

namespace edm {class ParameterSet;}

class AnaSiMuDistribution {
public:
  AnaSiMuDistribution (const edm::ParameterSet&);
  void init(TObjArray& histos);
  bool filter(const EventObj* ev, const TrackObj * simu, const HitSpecObj * hitSpec);
private:
  double ptMin, etaMinRef, etaMaxRef, phiMinRef, phiMaxRef;
  bool checkMatchedDets;
  std::vector<uint32_t> matchedDets;
};
#endif

