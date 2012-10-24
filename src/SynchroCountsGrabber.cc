#include "UserCode/L1RpcTriggerAnalysis/interface/SynchroCountsGrabber.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESTransientHandle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESWatcher.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "CondFormats/RPCObjects/interface/RPCEMap.h"
#include "CondFormats/DataRecord/interface/RPCEMapRcd.h"
#include "CondFormats/RPCObjects/interface/RPCReadOutMapping.h"
#include "CondFormats/RPCObjects/interface/LinkBoardSpec.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"

#include "DataFormats/MuonReco/interface/Muon.h"

#include "UserCode/L1RpcTriggerAnalysis/interface/TrackAtSurface.h"
#include "Geometry/RPCGeometry/interface/RPCGeometry.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/deltaPhi.h"






SynchroCountsGrabber::SynchroCountsGrabber(const edm::ParameterSet& cfg)
 : theCabling(0), theSelector(cfg.getParameter<edm::ParameterSet>("synchroSelectorMuon")), theNoSynchroWarning(false)
{}

SynchroCountsGrabber::~SynchroCountsGrabber()
{
if (theNoSynchroWarning) std::cout <<" **** SynchroCountsGrabber      **** WARNING - NoSynchroWarning was set!" << std::endl;

}

RPCRawSynchro::ProdItem SynchroCountsGrabber::counts(const edm::Event &ev, const edm::EventSetup &es)
{
  RPCRawSynchro::ProdItem result;

  if (theMapWatcher.check(es)) {
    delete theCabling;
    edm::ESTransientHandle<RPCEMap> readoutMapping;
    es.get<RPCEMapRcd>().get(readoutMapping);
    theCabling = readoutMapping->convert();
    LogTrace("") << "SynchroCountsGrabber - record has CHANGED!!, read map, VERSION: " << theCabling->version();
  }

  edm::Handle<RPCRawSynchro::ProdItem> synchroCounts;
  ev.getByType(synchroCounts);
  if (!synchroCounts.isValid()) {
    theNoSynchroWarning = true;
    return result;
  }

  TrackAtSurface trackAtSurface(theMuon, ev,es);
  edm::ESHandle<RPCGeometry> rpcGeometry;
  es.get<MuonGeometryRecord>().get(rpcGeometry);


  for(RPCRawSynchro::ProdItem::const_iterator it = synchroCounts->begin(); it != synchroCounts->end(); ++it) {
    const LinkBoardElectronicIndex & path = it->first;
    const  std::vector<FebConnectorSpec> & febs = theCabling->location(path)->febs();
    std::map<uint32_t,bool> dets;
    for (std::vector<FebConnectorSpec>::const_iterator iif = febs.begin(); iif != febs.end(); ++iif) dets[iif->rawId()] = true;
    for ( std::map<uint32_t,bool>::const_iterator im = dets.begin(); im != dets.end(); ++im) {
      RPCDetId rpcDet(im->first);
      const GeomDet *geomDet = rpcGeometry->idToDet(rpcDet);
      GlobalPoint detPosition = geomDet->position();
      if (deltaR(theMuon->eta(), theMuon->phi(), detPosition.eta(), detPosition.phi()) > 0.5) continue;
      TrajectoryStateOnSurface stateAtDet = trackAtSurface.atDetFromClose(rpcDet,detPosition);

      if (!stateAtDet.isValid()) continue;
      if (! (geomDet->surface().bounds().inside(stateAtDet.localPosition()))) continue;
      if (!theSelector.checkTraj(stateAtDet, rpcDet, ev, es)) continue;
      result.push_back(*it);
    }
  }
  return result;
}
