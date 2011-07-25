#include "EfficiencyAnalysis.h"
#include "UserCode/L1RpcTriggerAnalysis/interface/L1Obj.h"
#include "UserCode/L1RpcTriggerAnalysis/interface/EventObj.h"
#include "UserCode/L1RpcTriggerAnalysis/interface/TrackObj.h"
#include "UserCode/L1RpcTriggerAnalysis/interface/MuonObj.h"
#include "UserCode/L1RpcTriggerAnalysis/interface/L1ObjColl.h"


#include <vector>
#include <iostream>
#include <cmath>

#include "TFile.h"
#include "TChain.h"
#include "TTree.h"
#include "TProfile.h"
#include "TObjArray.h"
#include "TH2F.h"
#include "TH1D.h"
#include "TGraphErrors.h"
#include "TF1.h"


using namespace std;
template <class T> T sqr( T t) {return t*t;}

void EfficiencyAnalysis::beginJob()
{
  TObjArray histos;
  histos.SetOwner();

  // RECO muon kinematics (Reco::MuonCollection "muons", after cuts: track, vertex, pt, eta, RPC,CSC,DT hits)   
  TGraphErrors * hGraphLumi = new TGraphErrors();
  TGraphErrors * hGraphRun = new TGraphErrors(); 
  TGraphErrors * hGraphRunBarrel = new TGraphErrors(); 
  TGraphErrors * hGraphRunEndcap = new TGraphErrors(); 

  // variable-size ETA bins corresponding to PAC trigger granularity
  const Int_t nEtaBins=33;
  Double_t EtaBins[nEtaBins+1] = {
    -2.10, -1.97, -1.85, -1.73, -1.61, -1.48, -1.36, -1.24, -1.14, 
    -1.04, -0.93, -0.83, -0.72, -0.58, -0.44, -0.27, -0.07,
    0.07, 0.27, 0.44, 0.58, 0.72, 0.83, 0.93, 1.04, 1.14,
    1.24, 1.36, 1.48, 1.61, 1.73, 1.85, 1.97, 2.10
  };

  // variable-size PT bins corresponding to L1 trigger scale. 
  // Lower bound of bin ipt corresponds to L1Pt(ipt),
  // with exception of ipt=0 and 1 (which should be NAN and 0.)
  const Int_t nPtBins=32;
  Double_t PtBins[nPtBins+1] = {
   0., 0.1, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 6., 7., 8., 10., 
  12., 14., 16., 18., 20., 25., 30., 35., 40., 45., 50., 60., 70., 80., 90., 100., 120., 140., 
  150.   
  };
  
  TH1D * hMuonPt  = new TH1D("hMuonPt","All global muons Pt;Glb.muon p_{T} [GeV];Muons / bin",nPtBins,PtBins);  histos.Add(hMuonPt);
  TH1D * hMuonEta = new TH1D("hMuonEta","All global muons Eta;Glb.muon #eta;Muons / bin",64, -1.6, 1.6);  histos.Add(hMuonEta);
  TH1D * hMuonPhi = new TH1D("hMuonPhi","All global muons Phi;Glb.muon #phi [rad];Muons / bin",90,-M_PI,M_PI);  histos.Add(hMuonPhi);

  //Without affected stations

  TH1D * hMuonPt_good  = new TH1D("hMuonPt_good","All global muons Pt;Glb.muon p_{T} [GeV];Muons / bin",nPtBins,PtBins);  histos.Add(hMuonPt_good);
  TH1D * hMuonEta_good = new TH1D("hMuonEta_good","All global muons Eta;Glb.muon #eta;Muons / bin",64, -1.6, 1.6);  histos.Add(hMuonEta_good);
  TH1D * hMuonPhi_good = new TH1D("hMuonPhi_good","All global muons Phi;Glb.muon #phi [rad];Muons / bin",90,-M_PI,M_PI);  histos.Add(hMuonPhi_good);


  // RECO track kinematics (Reco::TrackCollection "generalTracks", track matched to RECO muon above)
  TH1D * hTrackPt  = new TH1D("hTrackPt","L1Muon-matched track Pt;Track p_{T} [GeV];Tracks / bin",nPtBins,PtBins);  histos.Add(hTrackPt);
  TH1D * hTrackEta = new TH1D("hTrackEta","L1Muon-matched track Eta;Track #eta;Tracks / bin",64, -1.6, 1.6);  histos.Add(hTrackEta);
  TH1D * hTrackPhi = new TH1D("hTrackPhi","L1Muon-matched track Phi;Track #phi [rad];Tracks / bin",90,-M_PI,M_PI);  histos.Add(hTrackPhi);


  // hit histos
  // barrel |eta|<0,8, endcap 1.25<|eta|<1.55 
  TH1D *hEfficDetB_N[6],  *hEfficHitB_N[6], *hEfficDetE_N[3], *hEfficHitE_N[3]; 
  TH1D *hEfficHitDet_D= new TH1D( "hEfficHitDet_D", "hEfficHitDet_D", 64, -1.6, 1.6);  histos.Add( hEfficHitDet_D);
  for (unsigned int i=1; i<=6; ++i) {
    std::stringstream name;
    name.str(""); name<<"hEfficDetB_N"<< i; hEfficDetB_N[i-1]= new TH1D( name.str().c_str(), name.str().c_str(), 64, -1.6, 1.6);  histos.Add( hEfficDetB_N[i-1]);
    name.str(""); name<<"hEfficHitB_N"<< i; hEfficHitB_N[i-1]= new TH1D( name.str().c_str(), name.str().c_str(), 64, -1.6, 1.6);  histos.Add( hEfficHitB_N[i-1]);
    if (i<=3) {
    name.str(""); name <<"hEfficDetE_N"<< i; hEfficDetE_N[i-1]= new TH1D( name.str().c_str(), name.str().c_str(), 64, -1.6, 1.6);  histos.Add( hEfficDetE_N[i-1]);
    name.str(""); name <<"hEfficHitE_N"<< i; hEfficHitE_N[i-1]= new TH1D( name.str().c_str(), name.str().c_str(), 64, -1.6, 1.6);  histos.Add( hEfficHitE_N[i-1]);
    }
  }

  TH1D * hHitsB = new TH1D("hHitsB","Layers with RPC hits matching propagated muon - Barrel;No of layers;Muons",7,-0.5,6.5);  histos.Add(hHitsB);
  TH1D * hHitsE = new TH1D("hHitsE","Layers with RPC hits matching propagated muon - Endcap;No of layers;Muons",4,-0.5,3.5);  histos.Add(hHitsE);
  TH1D * hDetsB_100 = new TH1D("hDetsB_100","Layers with RPCs crossed by propagated muon - Barrel;No of layers;Muons",7,-0.5,6.5);  histos.Add(hDetsB_100);
  TH1D * hDetsE_100 = new TH1D("hDetsE_100","Layers with RPCs crossed by propagated muon - Endcap;No of layers;Muons",4,-0.5,3.5);  histos.Add(hDetsE_100);

  // efficiency for hits
  // barrel |eta|<0,8, endcap 1.25<|eta|<1.55 
  TH1D * hEfficGeom_M = new TH1D("hEfficGeom_M","Propaged muons matching RPC Geom;Glb.muon #eta;Muons /bin", 64, -1.6, 1.6);  histos.Add(hEfficGeom_M);
  TH1D * hEfficGeom_H = new TH1D("hEfficGeom_H","Propaged muons matching RPC Geom;Glb.muon #eta;Muons / bin", 64, -1.6, 1.6);  histos.Add(hEfficGeom_H);
  TH1D * hEfficGeom_D = new TH1D("hEfficGeom_D","RPC triggers;Glb.muon #eta;Muons / bin",64, -1.6, 1.6);  histos.Add(hEfficGeom_D);
  TH1D * hEfficGeom_T = new TH1D("hEfficGeom_T","Propaged muons crossing RPCs;Glb.muon #eta;Muons / bin",64, -1.6, 1.6);  histos.Add(hEfficGeom_T);

  TH1D * hEfficGeom_H_3z6 = new TH1D("hEfficGeom_H_3z6","Propaged muons matching RPC Geom;Glb.muon #eta;Muons / bin", 64, -1.6, 1.6);  histos.Add(hEfficGeom_H_3z6);
  TH1D * hEfficGeom_D_3z6 = new TH1D("hEfficGeom_D_3z6","RPC triggers;Glb.muon #eta;Muons / bin",64, -1.6, 1.6);  histos.Add(hEfficGeom_D_3z6);
  TH1D * hEfficGeom_T_3z6 = new TH1D("hEfficGeom_T_3z6","Propaged muons crossing RPCs;Glb.muon #eta;Muons / bin",64, -1.6, 1.6);  histos.Add(hEfficGeom_T_3z6);

  //without affected stations

  TH1D * hEfficGeom_M_good = new TH1D("hEfficGeom_M_good","Propaged muons matching RPC Geom;Glb.muon #eta;Muons /bin", 64, -1.6, 1.6);  histos.Add(hEfficGeom_M_good);
  TH1D * hEfficGeom_H_good = new TH1D("hEfficGeom_H_good","Propaged muons matching RPC Geom;Glb.muon #eta;Muons / bin", 64, -1.6, 1.6);  histos.Add(hEfficGeom_H_good);
  TH1D * hEfficGeom_D_good = new TH1D("hEfficGeom_D_good","RPC triggers;Glb.muon #eta;Muons / bin",64, -1.6, 1.6);  histos.Add(hEfficGeom_D_good);
  TH1D * hEfficGeom_T_good = new TH1D("hEfficGeom_T_good","Propaged muons crossing RPCs;Glb.muon #eta;Muons / bin",64, -1.6, 1.6);  histos.Add(hEfficGeom_T_good);

  TH1D * hEfficGeom_H_3z6_good = new TH1D("hEfficGeom_H_3z6_good","Propaged muons matching RPC Geom;Glb.muon #eta;Muons / bin", 64, -1.6, 1.6);  histos.Add(hEfficGeom_H_3z6_good);
  TH1D * hEfficGeom_D_3z6_good = new TH1D("hEfficGeom_D_3z6_good","RPC triggers;Glb.muon #eta;Muons / bin",64, -1.6, 1.6);  histos.Add(hEfficGeom_D_3z6_good);
  TH1D * hEfficGeom_T_3z6_good = new TH1D("hEfficGeom_T_3z6_good","Propaged muons crossing RPCs;Glb.muon #eta;Muons / bin",64, -1.6, 1.6);  histos.Add(hEfficGeom_T_3z6_good);



 // efficiency for hits
  // full region
  TH1D * hEfficGeomTot_M = new TH1D("hEfficGeomTot_M","Propaged muons matching RPC Geom;Glb.muon #eta;Muons /bin", 128, -1.6, 1.6);  histos.Add(hEfficGeomTot_M);
  TH1D * hEfficGeomTot_H = new TH1D("hEfficGeomTot_H","Propaged muons matching RPC Geom;Glb.muon #eta;Muons / bin", 128, -1.6, 1.6);  histos.Add(hEfficGeomTot_H);
  TH1D * hEfficGeomTot_D = new TH1D("hEfficGeomTot_D","RPC triggers;Glb.muon #eta;Muons / bin",128, -1.6, 1.6);  histos.Add(hEfficGeomTot_D);
  TH1D * hEfficGeomTot_T = new TH1D("hEfficGeomTot_T","Propaged muons crossing RPCs;Glb.muon #eta;Muons / bin",128, -1.6, 1.6);  histos.Add(hEfficGeomTot_T);


  // in order to get numbewr of crossed layers
  TH1D * hEfficChambBar_N = new TH1D("hEfficChambBar_N","Propaged muons matching RPC hits - Barrel;Layer;Muons",6,0.5,6.5); histos.Add(hEfficChambBar_N); 
  TH1D * hEfficChambBar_D = new TH1D("hEfficChambBar_D","Propaged muons crossing RPCs - Barrel;Layer;Muons",6,0.5,6.5); histos.Add(hEfficChambBar_D); 
  TH1D * hEfficChambEnd_N = new TH1D("hEfficChambEnd_N","Propaged muons matching RPC hits - Endcap;Layer;Muons",3,0.5,3.5); histos.Add(hEfficChambEnd_N);
  TH1D * hEfficChambEnd_D = new TH1D("hEfficChambEnd_D","Propaged muons crossing RPCs - Endcap;Layer;Muons",3,0.5,3.5); histos.Add(hEfficChambEnd_D); 

  // efficiency for trigger wrt Mu 
  TH1D * hEfficMu_N =  new TH1D("hEfficMu_N","hEfficMu_N",64, -1.6, 1.6);  histos.Add(hEfficMu_N);
  TH1D * hEfficMu_D =  new TH1D("hEfficMu_D","hEfficMu_D",64, -1.6, 1.6);  histos.Add(hEfficMu_D);

  //Without Affected stations

  TH1D * hEfficMu_N_good =  new TH1D("hEfficMu_N_good","hEfficMu_N_good",64, -1.6, 1.6);  histos.Add(hEfficMu_N_good);
  TH1D * hEfficMu_D_good =  new TH1D("hEfficMu_D_good","hEfficMu_D_good",64, -1.6, 1.6);  histos.Add(hEfficMu_D_good);


  // efficiency for trigger wrt L1Other
  TH1D * hEfficTk_N =  new TH1D("hEfficTk_N","hEfficTk_N",64, -1.6, 1.6);  histos.Add(hEfficTk_N);
  TH1D * hEfficTk_D  = new TH1D("hEfficTk_D","hEfficTk_D",64, -1.6, 1.6);  histos.Add(hEfficTk_D);

// efficiency for  L1rpc vs Pt 
//  TH1D * hEfficMuPt7_N = new TH1D("hEfficMuPt7_N","hEfficMuPt7_N",75,0.,150.);  histos.Add(hEfficMuPt7_N);
//  TH1D * hEfficMuPt_N = new TH1D("hEfficMuPt_N","hEfficMuPt_N",75,0.,150.);  histos.Add(hEfficMuPt_N);
//  TH1D * hEfficMuPt_D = new TH1D("hEfficMuPt_D","hEfficMuPt_D",75,0.,150.);  histos.Add(hEfficMuPt_D);

   TH1D * hEfficMuPt7All_N = new TH1D("hEfficMuPt7All_N","hEfficMuPt7All_N",nPtBins,PtBins);  histos.Add(hEfficMuPt7All_N);
   TH1D * hEfficMuPt7_N = new TH1D("hEfficMuPt7_N","hEfficMuPt7_N",nPtBins,PtBins);  histos.Add(hEfficMuPt7_N);
   TH1D * hEfficMuPt7_N_barrel = new TH1D("hEfficMuPt7_N_barrel","hEfficMuPt7_N_barrel",nPtBins,PtBins);  histos.Add(hEfficMuPt7_N_barrel);
   TH1D * hEfficMuPt7_N_overlap = new TH1D("hEfficMuPt7_N_overlap","hEfficMuPt7_N_overlap",nPtBins,PtBins);  histos.Add(hEfficMuPt7_N_overlap);
   TH1D * hEfficMuPt7_N_endcap = new TH1D("hEfficMuPt7_N_endcap","hEfficMuPt7_N_endcap",nPtBins,PtBins);  histos.Add(hEfficMuPt7_N_endcap);
   TH1D * hEfficMuPt7_N_endcapN = new TH1D("hEfficMuPt7_N_endcapN","hEfficMuPt7_N_endcapN",nPtBins,PtBins);  histos.Add(hEfficMuPt7_N_endcapN);
   TH1D * hEfficMuPt7_N_endcapP = new TH1D("hEfficMuPt7_N_endcapP","hEfficMuPt7_N_endcapP",nPtBins,PtBins);  histos.Add(hEfficMuPt7_N_endcapP);
   TH1D * hEfficMuPt_N = new TH1D("hEfficMuPt_N","hEfficMuPt_N",nPtBins,PtBins);  histos.Add(hEfficMuPt_N);
   TH1D * hEfficMuPt_N_barrel = new TH1D("hEfficMuPt_N_barrel","hEfficMuPt_N_barrel",nPtBins,PtBins);  histos.Add(hEfficMuPt_N_barrel);
   TH1D * hEfficMuPt_N_overlap = new TH1D("hEfficMuPt_N_overlap","hEfficMuPt_N_overlap",nPtBins,PtBins);  histos.Add(hEfficMuPt_N_overlap);
   TH1D * hEfficMuPt_N_endcap = new TH1D("hEfficMuPt_N_endcap","hEfficMuPt_N_endcap",nPtBins,PtBins);  histos.Add(hEfficMuPt_N_endcap);
   TH1D * hEfficMuPt_N_endcapN = new TH1D("hEfficMuPt_N_endcapN","hEfficMuPt_N_endcapN",nPtBins,PtBins);  histos.Add(hEfficMuPt_N_endcapN);
   TH1D * hEfficMuPt_N_endcapP = new TH1D("hEfficMuPt_N_endcapP","hEfficMuPt_N_endcapP",nPtBins,PtBins);  histos.Add(hEfficMuPt_N_endcapP);
   TH1D * hEfficMuPt_D = new TH1D("hEfficMuPt_D","hEfficMuPt_D",nPtBins,PtBins);  histos.Add(hEfficMuPt_D);
   TH1D * hEfficMuPt_D_barrel = new TH1D("hEfficMuPt_D_barrel","hEfficMuPt_D_barrel",nPtBins,PtBins);  histos.Add(hEfficMuPt_D_barrel);
   TH1D * hEfficMuPt_D_overlap = new TH1D("hEfficMuPt_D_overlap","hEfficMuPt_D_overlap",nPtBins,PtBins);  histos.Add(hEfficMuPt_D_overlap);
   TH1D * hEfficMuPt_D_endcap = new TH1D("hEfficMuPt_D_endcap","hEfficMuPt_D_endcap",nPtBins,PtBins);  histos.Add(hEfficMuPt_D_endcap);
   TH1D * hEfficMuPt_D_endcapN = new TH1D("hEfficMuPt_D_endcapN","hEfficMuPt_D_endcapN",nPtBins,PtBins);  histos.Add(hEfficMuPt_D_endcapN);
   TH1D * hEfficMuPt_D_endcapP = new TH1D("hEfficMuPt_D_endcapP","hEfficMuPt_D_endcapP",nPtBins,PtBins);  histos.Add(hEfficMuPt_D_endcapP);


   TH2D *hEfficMuPtVsEta_D = new TH2D("hEfficMuPtVsEta_D","",128,-1.6,1.6,nPtBins,PtBins);
   TH2D *hEfficMuPtVsEta_N = new TH2D("hEfficMuPtVsEta_N","",128,-1.6,1.6,nPtBins,PtBins);
   histos.Add(hEfficMuPtVsEta_D);
   histos.Add(hEfficMuPtVsEta_N);

   TH2D *hEfficMuPtVsPhi_D = new TH2D("hEfficMuPtVsPhi_D","",64,-3.2,3.2,nPtBins,PtBins);
   TH2D *hEfficMuPtVsPhi_N = new TH2D("hEfficMuPtVsPhi_N","",64,-3.2,3.2,nPtBins,PtBins);
   histos.Add(hEfficMuPtVsPhi_D);
   histos.Add(hEfficMuPtVsPhi_N);

   TH2D *hDeltaPtVsEta = new TH2D("hDeltaPtVsEta","",nEtaBins,EtaBins,200,-1,20);
   //histos.Add(hDeltaPtVsEta);
   std::map< unsigned int,TH2D* > histoRunMap;
  

//   // efficiency for  L1rpc vs Pt 
   TH1D * hEfficTkPt7_N = new TH1D("hEfficTkPt7_N","hEfficTkPt7_N",75,0.,150.);  histos.Add(hEfficTkPt7_N);
   TH1D * hEfficTkPt_N = new TH1D("hEfficTkPt_N","hEfficTkPt_N",75,0.,150.);  histos.Add(hEfficTkPt_N);
   TH1D * hEfficTkPt_D = new TH1D("hEfficTkPt_D","hEfficTkPt_D",75,0.,150.);  histos.Add(hEfficTkPt_D);


  // efficiency for  L1rpc vs Pt 
//  TH1D * hEfficTkPt7_N = new TH1D("hEfficTkPt7_N","hEfficTkPt7_N",50,5.,55.);  histos.Add(hEfficTkPt7_N);
//  TH1D * hEfficTkPt_N = new TH1D("hEfficTkPt_N","hEfficTkPt_N",50,5.,55.);  histos.Add(hEfficTkPt_N);
//  TH1D * hEfficTkPt_D = new TH1D("hEfficTkPt_D","hEfficTkPt_D",50,5.,55.);  histos.Add(hEfficTkPt_D);



  TH2D* hDistL1Rpc   = new TH2D("hDistL1Rpc","All L1 RPC candidates (#phi,#eta);L1 RPC #eta;L1 RPC #phi [rad];Muons / bin",
                                nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(hDistL1Rpc);  
  TH2D* hDistL1Other = new TH2D("hDistL1Other","All L1 DT/CSC candidates (#phi,#eta);L1 obj #eta;L1 obj #phi [rad];DT/CSC muons / bin",
                                nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(hDistL1Other);

  TH2D* hDistN = new TH2D("hDistN","All global muons with RPC L1T (#phi,#eta);Global Muon #eta;Global muon #phi [rad];Global muons / bin",
                                nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(hDistN);
  TH2D* hDistD = new TH2D("hDistD","All global muons (#phi,#eta);Global Muon #eta;Global muon #phi [rad];Global muons / bin",
                                nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(hDistD);

  TH2D* hRpcPtCode = new TH2D("hRpcPtCode","pt vs ptCode;ptCode;pt; Muons / bin",
                                140,0,140,140,0,140); histos.Add(hRpcPtCode);  

  //Without affected sectors

  TH2D* hDistL1Rpc_good   = new TH2D("hDistL1Rpc_good ","All L1 RPC candidates (#phi,#eta);L1 RPC #eta;L1 RPC #phi [rad];Muons / bin",
                                nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(hDistL1Rpc_good );
  TH2D* hDistL1Other_good  = new TH2D("hDistL1Other_good ","All L1 DT/CSC candidates (#phi,#eta);L1 obj #eta;L1 obj #phi [rad];DT/CSC muons / bin",
                                nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(hDistL1Other_good );

  TH2D* hDistN_good = new TH2D("hDistN_good","All global muons with RPC L1T (#phi,#eta);Global Muon #eta;Global muon #phi [rad];Global muons / bin",
                                nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(hDistN_good);
  TH2D* hDistD_good = new TH2D("hDistD_good","All global muons (#phi,#eta);Global Muon #eta;Global muon #phi [rad];Global muons / bin",
                                nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(hDistD_good);

  TH1D* hEffLumi = new TH1D("hEffLumi","Efficiency per LS;Efficiency per LS;No of LS / bin",100,0.,1.); histos.Add(hEffLumi);
  TH1D* hEffRun = new TH1D("hEffRun","Efficiency per run;Efficiency per run;No of runs / bin",100,0.,1.); histos.Add(hEffRun);

  // timing of L1 RPC trigger candidates
  TH1D* hL1RpcBX = new TH1D("hL1RpcBX","First L1 RPC candidate BX wrt L1;First L1 RPC wrt L1 [BX];Muons / BX",5,-2.5,2.5); histos.Add(hL1RpcBX);
  TH1D* hL1RpcBX_all = new TH1D("hL1RpcBX_all","All L1 RPC candidates BX wrt L1;L1 RPC wrt L1 [BX];Muons / BX",5,-2.5,2.5); histos.Add(hL1RpcBX_all);

  TH1D* hL1RpcGBX = new TH1D("hL1RpcGBX","First L1 RPC candidate BX;Event.bx + 1st L1RPC.bx;Muons / BX",4000,-0.5,4000.-0.5); histos.Add(hL1RpcGBX);
  TH1D* hL1RpcGBX_all = new TH1D("hL1RpcGBX_all","All L1 RPC candidates BX;Event.bx + L1RPC.bx;Muons / BX",4000,-0.5,4000.-0.5); histos.Add(hL1RpcGBX_all);
  TH1D* hMuGBX = new TH1D("hMuGBX","Glb.muon BX;Event BX;Muons / BX",4000,-0.5,4000.-0.5); histos.Add(hMuGBX);

  // L1 RPC trigger candidates eta, phi
  TH1D* hL1RpcEta = new TH1D("hL1RpcEta","All L1 RPC candidates #eta;L1 RPC #eta;Muons / bin",nEtaBins,EtaBins); histos.Add(hL1RpcEta);
  TH1D* hL1RpcPhi = new TH1D("hL1RpcPhi","All L1 RPC candidates #phi;L1 RPC #phi [rad];Muons / bin",144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(hL1RpcPhi);

  TH1D* hL1RpcEta_q0 = new TH1D("hL1RpcEta_q0","All L1 RPC candidates #eta, q0;L1 RPC #eta;Muons_{q0} / bin",nEtaBins,EtaBins); histos.Add(hL1RpcEta_q0);
  TH1D* hL1RpcPhi_q0 = new TH1D("hL1RpcPhi_q0","All L1 RPC candidates #phi, q0;L1 RPC #phi [rad];Muons_{q0} / bin",144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(hL1RpcPhi_q0);
  TH1D* hL1RpcEta_q1 = new TH1D("hL1RpcEta_q1","All L1 RPC candidates #eta, q1;L1 RPC #eta;Muons_{q1} / bin",nEtaBins,EtaBins); histos.Add(hL1RpcEta_q1);
  TH1D* hL1RpcPhi_q1 = new TH1D("hL1RpcPhi_q1","All L1 RPC candidates #phi, q1;L1 RPC #phi [rad];Muons_{q1} / bin",144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(hL1RpcPhi_q1);
  TH1D* hL1RpcEta_q2 = new TH1D("hL1RpcEta_q2","All L1 RPC candidates #eta, q2;L1 RPC #eta;Muons_{q2} / bin",nEtaBins,EtaBins); histos.Add(hL1RpcEta_q2);
  TH1D* hL1RpcPhi_q2 = new TH1D("hL1RpcPhi_q2","All L1 RPC candidates #phi, q2;L1 RPC #phi [rad];Muons_{q2} / bin",144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(hL1RpcPhi_q2);
  TH1D* hL1RpcEta_q3 = new TH1D("hL1RpcEta_q3","All L1 RPC candidates #eta, q3;L1 RPC #eta;Muons_{q3} / bin",nEtaBins,EtaBins); histos.Add(hL1RpcEta_q3);
  TH1D* hL1RpcPhi_q3 = new TH1D("hL1RpcPhi_q3","All L1 RPC candidates #phi, q3;L1 RPC #phi [rad];Muons_{q3} / bin",144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(hL1RpcPhi_q3);

  TH2D* h2L1RpcBX   = new TH2D("h2L1RpcBX","All L1 RPC candidates (#eta,#phi);L1 RPC #eta;L1 RPC #phi [rad];Muons / bin",nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(h2L1RpcBX);
  TH2D* h2L1RpcBX_q0   = new TH2D("h2L1RpcBX_q0","All L1 RPC candidates (#eta,#phi), q0;L1 RPC #eta;L1 RPC #phi [rad];Muons_{q0} / bin",nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(h2L1RpcBX_q0);
  TH2D* h2L1RpcBX_q1   = new TH2D("h2L1RpcBX_q1","All L1 RPC candidates (#eta,#phi), q1;L1 RPC #eta;L1 RPC #phi [rad];Muons_{q1} / bin",nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(h2L1RpcBX_q1);
  TH2D* h2L1RpcBX_q2   = new TH2D("h2L1RpcBX_q2","All L1 RPC candidates (#eta,#phi), q2;L1 RPC #eta;L1 RPC #phi [rad];Muons_{q2} / bin",nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(h2L1RpcBX_q2);
  TH2D* h2L1RpcBX_q3   = new TH2D("h2L1RpcBX_q3","All L1 RPC candidates (#eta,#phi), q3;L1 RPC #eta;L1 RPC #phi [rad];Muons_{q3} / bin",nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(h2L1RpcBX_q3);

  // L1 RPC trigger candidates at BX<>0 wrt L1
  TH2D* h2L1RpcBX1   = new TH2D("h2L1RpcBX1","All L1 RPC candidates (#eta,#phi) @ BX<>0;L1 RPC #eta;L1 RPC #phi [rad];Muons_{BX<>0} / bin",nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(h2L1RpcBX1);
  TH2D* h2L1RpcBX1_q0   = new TH2D("h2L1RpcBX1_q0","All L1 RPC candidates (#eta,#phi), q0 @ BX<>0;L1 RPC #eta;L1 RPC #phi [rad];Muons_{q0 BX<>0} / bin",nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(h2L1RpcBX1_q0);
  TH2D* h2L1RpcBX1_q1   = new TH2D("h2L1RpcBX1_q1","All L1 RPC candidates (#eta,#phi), q1 @ BX<>0;L1 RPC #eta;L1 RPC #phi [rad];Muons_{q1 BX<>0} / bin",nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(h2L1RpcBX1_q1);
  TH2D* h2L1RpcBX1_q2   = new TH2D("h2L1RpcBX1_q2","All L1 RPC candidates (#eta,#phi), q2 @ BX<>0;L1 RPC #eta;L1 RPC #phi [rad];Muons_{q2 BX<>0} / bin",nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(h2L1RpcBX1_q2);
  TH2D* h2L1RpcBX1_q3   = new TH2D("h2L1RpcBX1_q3","All L1 RPC candidates (#eta,#phi), q3 @ BX<>0;L1 RPC #eta;L1 RPC #phi [rad];Muons_{q3 BX<>0} / bin",nEtaBins,EtaBins,144,-0.2e-3,2*M_PI-0.2e-3); histos.Add(h2L1RpcBX1_q3);

  // average efficiency per LumiSection
  typedef std::map< std::pair<unsigned int, unsigned int>, std::pair<unsigned int, unsigned int> > EffLumiMap;
  EffLumiMap effLumiMap;

  // average efficiency per Run
  typedef std::map< unsigned int, std::pair<unsigned int, unsigned int> > EffRunMap;
  EffRunMap effRunMap;
  EffRunMap effRunMapBarrel;
  EffRunMap effRunMapEndcap;
  

  // define chain
  TChain chain("tL1Rpc");
  std::vector<std::string> treeFileNames = theConfig.getParameter<std::vector<std::string> >("treeFileNames");
  for (std::vector<std::string>::const_iterator it = treeFileNames.begin(); it != treeFileNames.end(); ++it)  chain.Add((*it).c_str() );

  std::vector<bool> *hitBarrel = 0;
  std::vector<bool> *hitEndcap = 0;
  std::vector<unsigned int> *detBarrel = 0;
  std::vector<unsigned int> *detEndcap = 0;

//  std::vector<unsigned int> *validPRCEndcap = 0;
//  std::vector<unsigned int> *validDTEndcap = 0;
//  std::vector<unsigned int> *validCSCEndcap = 0;


  EventObj * event = 0;
  TrackObj * track = 0;
  MuonObj * muon = 0;

  TBranch *bhitBarrel=0;
  TBranch *bhitEndcap=0;
  TBranch *bdetBarrel=0;
  TBranch *bdetEndcap=0;

  L1ObjColl* l1RpcColl = 0;
  L1ObjColl* l1OtherColl = 0;
  
  chain.SetBranchAddress("event",&event);

  chain.SetBranchAddress("muon",&muon);
  chain.SetBranchAddress("track",&track);

  chain.SetBranchAddress("hitBarrel",&hitBarrel, &bhitBarrel);
  chain.SetBranchAddress("hitEndcap",&hitEndcap, &bhitEndcap);
  chain.SetBranchAddress("detBarrel",&detBarrel, &bdetBarrel);
  chain.SetBranchAddress("detEndcap",&detEndcap, &bdetEndcap);

  chain.SetBranchAddress("l1RpcColl",&l1RpcColl);
  chain.SetBranchAddress("l1OtherColl",&l1OtherColl);

  Int_t nentries = (Int_t) chain.GetEntries();
  std::cout <<" ENTRIES: " << nentries << std::endl;
  std::cout <<"CUTS:  pt_reco: "<<theConfig.getParameter<double>("ptMin")
	    <<"  pt_l1Cut "<<theConfig.getParameter<double>("l1Cut")
	   // <<"  selected_RUN "<<theConfig.getParameter<double>("noRun")

	    << std::endl;


    std::cout << " WARNING ============= REJECTED RUNS: 147155 146417 146421 147749==============" << std::endl; 

int RPCTriggers = 0;
int RPCTriggersOnly = 0;
int OtherTriggers = 0;
int OtherTriggersOnly = 0;

  for (int ev=0; ev<nentries; ev++) {
    chain.GetEntry(ev);

      
  // reject wrong runs:
//  if(event->run == 147421 ||event->run == 147417) 
 if(event->run == 147115|| event->run == 147421 ||event->run == 147417 || event->run == 147749) continue;
 // if(event->run != theConfig.getParameter<double>("noRun") ) continue;
 //if(event->run < theConfig.getParameter<double>("noRun") ) continue;

std::vector<L1Obj> l1Others = l1OtherColl->getL1Objs();
std::vector<L1Obj> l1Rpcs = l1RpcColl->getL1ObjsMatched();
if(l1Others.size() ==0 && l1Rpcs.size() !=0 ) RPCTriggersOnly++;
if(l1Others.size() !=0 && l1Rpcs.size() ==0 ) OtherTriggersOnly++;
if(l1Rpcs.size() !=0 ) RPCTriggers++;
if(l1Others.size() !=0 ) OtherTriggers++;

if(l1Others.size() ==0 ) continue;


 if (effLumiMap.find( std::make_pair(event->run, event->lumi)) == effLumiMap.end())
        effLumiMap[ make_pair( event->run, event->lumi)] = make_pair(0,0);
    if (effRunMap.find(event->run) == effRunMap.end()){
        effRunMap[event->run] = make_pair(0,0);
	effRunMapBarrel[event->run] = make_pair(0,0);
	effRunMapEndcap[event->run] = make_pair(0,0);
    }

    if (   !muon->isGlobal() 
        || !muon->isTracker()
        || !muon->isOuter()
        || muon->pt()<theConfig.getParameter<double>("ptMin") 
        || fabs(muon->eta()) > 1.61 
        ) continue;

    hMuGBX->Fill(event->bx);
//    std::vector<L1Obj> l1Rpcs = l1RpcColl->getL1ObjsMatched();

    //
    // MAIN RPC EFFICIENCY HISTORGAMS
    //
    bool toto = false;
    bool isMuon = (muon->pt() > theConfig.getParameter<double>("ptMin") );
    if (isMuon) {
      //if (fabs(muon.eta) > 0.8) continue;
      //if (fabs(muon.eta) < 1.24) continue;

      hMuonPt->Fill(muon->pt());
      hMuonEta->Fill(muon->eta());
      hMuonPhi->Fill(muon->phi());
      hEfficMu_D->Fill(muon->eta());
      hDistD->Fill(muon->eta(),muon->phi());
      if (l1Rpcs.size()) {
    	  hEfficMu_N->Fill(muon->eta());
    	  hDistN->Fill(muon->eta(),muon->phi());
      }
      //Without Affected stations

      if (  !(muon->phi() < 2. && muon->phi() > -0.2) &&  !(muon->phi() < -1.4 && muon->phi() > -3.14)  && !(muon->phi() >2.9 && muon->phi() < 3.14)) {


    	  hMuonPt_good->Fill(muon->pt());
    	  hMuonEta_good->Fill(muon->eta());
    	  hMuonPhi_good->Fill(muon->phi());
    	  hEfficMu_D_good->Fill(muon->eta());
    	  hDistD_good->Fill(muon->eta(),muon->phi());
      	  if (l1Rpcs.size()) {
      		  hEfficMu_N_good->Fill(muon->eta());
      		  hDistN_good->Fill(muon->eta(),muon->phi());
      	  }

      }

      hEfficMuPt_D->Fill(muon->pt());
      if (l1Rpcs.size())  hEfficMuPt_N->Fill(muon->pt());

      ///AK
      hEfficMuPtVsEta_D->Fill(muon->eta(),muon->pt());
      if (l1Rpcs.size()) hEfficMuPtVsEta_N->Fill(muon->eta(),muon->pt());  

      if(fabs(muon->eta())<1.24 && fabs(muon->eta())>1.14){
      hEfficMuPtVsPhi_D->Fill(muon->phi(),muon->pt());
      if (l1Rpcs.size()){
	hEfficMuPtVsPhi_N->Fill(muon->phi(),muon->pt());  
      }
      }      
      if(l1Rpcs.size() && muon->pt()>8 && muon->pt()<10){
	float deltaPt = (l1Rpcs[0].pt - muon->pt())/muon->pt();
	hDeltaPtVsEta->Fill(muon->eta(),deltaPt);  
      }
      if(histoRunMap.find(event->run)==histoRunMap.end())
	histoRunMap[event->run] = (TH2D*)hDeltaPtVsEta->Clone(Form("hDeltaPtVsEta%d",event->run));
      else{
	histoRunMap[event->run]->Add(hDeltaPtVsEta);
      }
      hDeltaPtVsEta->Reset();
      /////

      if (l1RpcColl->getL1ObjsMatched(theConfig.getParameter<double>("l1Cut")).size()) hEfficMuPt7_N->Fill(muon->pt());
      if (l1RpcColl->getL1ObjsMatched(theConfig.getParameter<double>("ptMin")).size()) hEfficMuPt7All_N->Fill(muon->pt());



     if (fabs(muon->eta()) < 0.8){
    	  hEfficMuPt_D_barrel->Fill(muon->pt());
          if (l1Rpcs.size())  hEfficMuPt_N_barrel->Fill(muon->pt());
          if (l1RpcColl->getL1ObjsMatched(theConfig.getParameter<double>("l1Cut")).size()) hEfficMuPt7_N_barrel->Fill(muon->pt());
      }
      if (fabs(muon->eta()) < 1.24 && fabs(muon->eta()) > 0.8){
    	  hEfficMuPt_D_overlap->Fill(muon->pt());
          if (l1Rpcs.size())  hEfficMuPt_N_overlap->Fill(muon->pt());
          if (l1RpcColl->getL1ObjsMatched(theConfig.getParameter<double>("l1Cut")).size()) hEfficMuPt7_N_overlap->Fill(muon->pt());
      }
      if (fabs(muon->eta()) > 1.24 && fabs(muon->eta()) < 1.6){
    	  hEfficMuPt_D_endcap->Fill(muon->pt());
          if (l1Rpcs.size())  hEfficMuPt_N_endcap->Fill(muon->pt());
          if (l1RpcColl->getL1ObjsMatched(theConfig.getParameter<double>("l1Cut")).size()) hEfficMuPt7_N_endcap->Fill(muon->pt());
      }
      if (muon->eta() > 1.24 && muon->eta() < 1.6){
    	  hEfficMuPt_D_endcapP->Fill(muon->pt());
          if (l1Rpcs.size())  hEfficMuPt_N_endcapP->Fill(muon->pt());
          if (l1RpcColl->getL1ObjsMatched(theConfig.getParameter<double>("l1Cut")).size()) hEfficMuPt7_N_endcapP->Fill(muon->pt());
      }
      if (muon->eta() < -1.24 && muon->eta() > -1.6){
    	  hEfficMuPt_D_endcapN->Fill(muon->pt());
          if (l1Rpcs.size())  hEfficMuPt_N_endcapN->Fill(muon->pt());
          if (l1RpcColl->getL1ObjsMatched(theConfig.getParameter<double>("l1Cut")).size()) hEfficMuPt7_N_endcapN->Fill(muon->pt());
      }  
 }
    //
    // L1RPC EFFICIENCY AS FUNCTION OF RUN/LUMI
    //
    effLumiMap[make_pair(event->run,event->lumi)].second++;
    effRunMap[event->run].second++;
    if (l1Rpcs.size() > 0) {
      effLumiMap[make_pair(event->run,event->lumi)].first++;
      effRunMap[event->run].first++;
    }
    if (fabs(muon->eta()) <0.8){
      effRunMapBarrel[event->run].second++;
      if (l1Rpcs.size() > 0) effRunMapBarrel[event->run].first++;    
    }

    
    if (fabs(muon->eta()) < 1.6 && fabs(muon->eta()) > 0.8){
      effRunMapEndcap[event->run].second++;
      if (l1Rpcs.size() > 0) effRunMapEndcap[event->run].first++;    
    }    
  
    //
    // MUON HITS AND CROSSED DETS ANALYSIS
    //
    if (isMuon) {
      //number of layers with hits, counting: nHitsB - all 6 barrel layers, nHitsBL - first 4 barrel layers, nHitsE - all 3 endcap layers
      int nHitsB = 0; for (int i=0; i<6; i++) if( hitBarrel->at(i) ) nHitsB++;
      int nHitsBL= 0; for (int i=0; i<4; i++) if( hitBarrel->at(i) ) nHitsBL++;
      int nHitsE = 0; for (int i=0; i<3; i++) if( hitEndcap->at(i) ) nHitsE++;

      //number of barrel stations with hits
      int nStationHitsB = 0;
      if( hitBarrel->at(0)|| hitBarrel->at(1)) nStationHitsB++;
      if( hitBarrel->at(2)|| hitBarrel->at(3)) nStationHitsB++;
      if( hitBarrel->at(4)) nStationHitsB++;
      if( hitBarrel->at(5)) nStationHitsB++;

      //number of layers crossed by the muon, counting: nDetsB - all 6 barrel layers, nDetsBL - first 4 barrel layers, nDetsE - all 3 endcap layers
      int nDetsB = 0;  for (int i=0; i<6; i++) if( detBarrel->at(i) ) nDetsB++;
      int nDetsBL= 0;  for (int i=0; i<4; i++) if( detBarrel->at(i) ) nDetsBL++;
      int nDetsE = 0;  for (int i=0; i<3; i++) if( detEndcap->at(i) ) nDetsE++;

      //number of barrel stations crossed by the muon
      int nStationsB = 0;
      if( detBarrel->at(0)|| detBarrel->at(1)) nStationsB++;
      if( detBarrel->at(2)|| detBarrel->at(3)) nStationsB++;
      if( detBarrel->at(4)) nStationsB++;
      if( detBarrel->at(5)) nStationsB++;


      hEfficHitDet_D->Fill(muon->eta());
      for (int i=0; i<6;++i) {
        if (detBarrel->at(i)) hEfficChambBar_D->Fill(i+1.);
	  if (hitBarrel->at(i)) hEfficChambBar_N->Fill(i+1.);
        if (detBarrel->at(i)) hEfficDetB_N[i]->Fill(muon->eta());
        if (hitBarrel->at(i)) hEfficHitB_N[i]->Fill(muon->eta());
      }
      for (int i=0; i<3;++i) {
        if (detEndcap->at(i)) hEfficChambEnd_D->Fill(i+1.);
	  if (hitEndcap->at(i)) hEfficChambEnd_N->Fill(i+1.);
        if (detEndcap->at(i)) hEfficDetE_N[i]->Fill(muon->eta());
        if (hitEndcap->at(i)) hEfficHitE_N[i]->Fill(muon->eta());
      }

      //Fill histograms: hEfficGeom_M - 	all global muons
      //				 hEfficGeom_D - 	global muons crossing >= no. of layers required to fire the trigger, 4/6 algo
      //				 hEfficGeom_D_3z6 - global muons crossing >= no. of layers (low pt muons) or stations (high pt muons) required to fire the trigger, 3/6 algo
      //				 hEfficGeom_H - 	global muons leaving hits in >= no. of layers required to fire the trigger, 4/6 algo
      //				 hEfficGeom_H_3z6 - global muons leaving hits in >= no. of layers low pt muons) or stations (high pt muons) required to fire the trigger, 3/6 algo
      //				 hEfficGeom_T - 	global muons leaving hits in >= no. of layers required to fire the trigger and firing the trigger, 4/6 algo,
      //				 hEfficGeom_T_3z6 - global muons leaving hits in >= no. of layers low pt muons) or stations (high pt muons) required to fire the trigger and firing the trigger, 3/6 algo
      // pure Barrel
      if (fabs(muon->eta()) < 0.8) {
        
        hHitsB->Fill(nHitsB);
        hDetsB_100->Fill(nDetsB);
        hEfficGeom_M->Fill(muon->eta());      
        if (nDetsBL>=3 || nDetsB>=4) hEfficGeom_D->Fill(muon->eta());
        if (nDetsBL>=3 || nStationsB>=3) hEfficGeom_D_3z6->Fill(muon->eta());
        if (nHitsBL>=3 || nHitsB>=4) {
          hEfficGeom_H->Fill(muon->eta());
          if (l1Rpcs.size()>0) hEfficGeom_T->Fill(muon->eta());
        } else if (false && l1Rpcs.size()>0) { toto=true; std::cout <<"nHitsBL: " << nHitsBL<<" nHitsB: "<< nHitsB<< std::endl; }
        if (nHitsBL>=3 || nStationHitsB>=3) {
          hEfficGeom_H_3z6->Fill(muon->eta());
          if (l1Rpcs.size()>0) hEfficGeom_T_3z6->Fill(muon->eta());
        } else if (false && l1Rpcs.size()>0) { toto=true; std::cout <<"nHitsBL: " << nHitsBL<<" nHitsB: "<< nHitsB<< std::endl; }
      }

      // pure Endcap
      if (fabs(muon->eta()) > 1.25  && fabs(muon->eta()) < 1.6) {
        if (l1Rpcs.size())  hRpcPtCode->Fill(l1Rpcs[0].pt, muon->pt());
        hHitsE->Fill(nHitsE);
        hDetsE_100->Fill(nDetsE);
        hEfficGeom_M->Fill(muon->eta());
        if (nDetsE>=3){
        	hEfficGeom_D->Fill(muon->eta());
        	hEfficGeom_D_3z6->Fill(muon->eta());
        }
        if (nHitsE>=3) {
          hEfficGeom_H->Fill(muon->eta());
          hEfficGeom_H_3z6->Fill(muon->eta());
          if (l1Rpcs.size()){
        	  hEfficGeom_T->Fill(muon->eta());
        	  hEfficGeom_T_3z6->Fill(muon->eta());
          }
        }
        if (false && l1Rpcs.size()>0 && nHitsE!=3) { toto=true; std::cout <<"NHITS E: " << nHitsE<< std::endl; }
      }

      //Without Affected stations

      if (  fabs(muon->eta()) < 0.8 &&  !(muon->phi() < 2. && muon->phi() > -0.2) &&  !(muon->phi() < -1.4 && muon->phi() > -3.14)  && !(muon->phi() >2.9 && muon->phi() < 3.14)) {
        //hHitsB->Fill(nHitsB);
        //hDetsB_100->Fill(nDetsB);
        hEfficGeom_M_good->Fill(muon->eta());
        if (nDetsBL>=3 || nDetsB>=4) hEfficGeom_D_good->Fill(muon->eta());
        if (nDetsBL>=3 || nStationsB>=3) hEfficGeom_D_3z6_good->Fill(muon->eta());
        if (nHitsBL>=3 || nHitsB>=4) {
          hEfficGeom_H_good->Fill(muon->eta());
          if (l1Rpcs.size()>0) hEfficGeom_T_good->Fill(muon->eta());
        } else if (false && l1Rpcs.size()>0) { toto=true; std::cout <<"nHitsBL: " << nHitsBL<<" nHitsB: "<< nHitsB<< std::endl; }
        if (nHitsBL>=3 || nStationHitsB>=3) {
          hEfficGeom_H_3z6_good->Fill(muon->eta());
          if (l1Rpcs.size()>0) hEfficGeom_T_3z6_good->Fill(muon->eta());
        } else if (false && l1Rpcs.size()>0) { toto=true; std::cout <<"nHitsBL: " << nHitsBL<<" nHitsB: "<< nHitsB<< std::endl; }
      }

      // pure Endcap
      if (fabs(muon->eta()) > 1.25  && fabs(muon->eta() ) < 1.6 && !(muon->phi() < 2. && muon->phi() > -0.2) &&  !(muon->phi() < -1.4 && muon->phi() > -3.14)  && !(muon->phi() >2.9 && muon->phi() < 3.14)) {
        hHitsE->Fill(nHitsE);
        hDetsE_100->Fill(nDetsE);
        hEfficGeom_M_good->Fill(muon->eta());
        if (nDetsE>=3){
        	hEfficGeom_D_good->Fill(muon->eta());
        	hEfficGeom_D_3z6_good->Fill(muon->eta());
        }
        if (nHitsE>=3) {
          hEfficGeom_H_good->Fill(muon->eta());
          hEfficGeom_H_3z6_good->Fill(muon->eta());
          if (l1Rpcs.size()){
        	  hEfficGeom_T_good->Fill(muon->eta());
        	  hEfficGeom_T_3z6_good->Fill(muon->eta());
          }
        }
        if (false && l1Rpcs.size()>0 && nHitsE!=3) { toto=true; std::cout <<"NHITS E: " << nHitsE<< std::endl; }
      }


      //full Detector
      // pure Barrel
      if (fabs(muon->eta()) < 0.8 ) {
        hEfficGeomTot_M->Fill(muon->eta());      
// warunek Marcina dodatkowe wymagania dla layerow 5 lub 6 jesli 4/6
//          if (nDetsBL>=3 || (nDetsB>=4 && (detBarrel->at(4)||detBarrel->at(5)) ) ) hEfficGeomTot_D->Fill(muon->eta());
// 	 //         if (nHitsBL>=3 || (nHitsB>=4 && (hitBarrel->at(4)||hitBarrel->at(5)) && !hitBarrel->at(0) ) ) {
//          if (nHitsBL>=3 || (nHitsB>=4 && (hitBarrel->at(4)||hitBarrel->at(5))  ) ) {
// ogolny waunek na 3/6 lub 4/6
       if (nDetsBL>=3 || nStationsB>=3)  hEfficGeomTot_D->Fill(muon->eta());
       // if ( (nHitsBL>=3 || nStationHitsB>=3)  && !hitBarrel->at(0)  ) { //dlaczego && !hitBarrel->at(0) ?????
       if ( nHitsBL>=3 || nStationHitsB>=3) {
          hEfficGeomTot_H->Fill(muon->eta());
          if (l1Rpcs.size()>0) hEfficGeomTot_T->Fill(muon->eta());
        } else if (false && l1Rpcs.size()>0) { toto=true; std::cout <<"nHitsBL: " << nHitsBL<<" nHitsB: "<< nHitsB<< std::endl; }
      }
      // pure Endcap
      if (fabs(muon->eta()) > 1.25  && fabs(muon->eta()) < 1.6) { 
        hEfficGeomTot_M->Fill(muon->eta());      
        if (nDetsE>=3) hEfficGeomTot_D->Fill(muon->eta());
        if (nHitsE>=3) {
          hEfficGeomTot_H->Fill(muon->eta());
          if (l1Rpcs.size()) hEfficGeomTot_T->Fill(muon->eta());
        }
        if (false && l1Rpcs.size()>0 && nHitsE!=3) { toto=true; std::cout <<"NHITS E: " << nHitsE<< std::endl; }
      }
      //overlap 
      //tower 6 like barrel
      if (fabs(muon->eta()) >=0.8  && fabs(muon->eta()) < 0.93) { 
        hEfficGeomTot_M->Fill(muon->eta()); 
// 	if (nDetsBL>=3 || (nDetsB>=4 && (detBarrel->at(4)||detBarrel->at(5)) ) ) hEfficGeomTot_D->Fill(muon->eta());
// 	if (nHitsBL>=3 || (nHitsB>=4 && (hitBarrel->at(4)||hitBarrel->at(5))  ) ) {
// 	if (nDetsBL>=3 || (nDetsB>=4 && (detBarrel->at(4) && !detBarrel->at(5)) ) ) hEfficGeomTot_D->Fill(muon->eta());
// 	if (nHitsBL>=3 || (nHitsB>=4 && (hitBarrel->at(4) && !hitBarrel->at(5))  ) ) {
        if (nDetsBL>=3 || nStationsB>=3)  hEfficGeomTot_D->Fill(muon->eta());
        if ( nHitsBL>=3 || nStationHitsB>=3)  {

          hEfficGeomTot_H->Fill(muon->eta());
          if (l1Rpcs.size()>0) hEfficGeomTot_T->Fill(muon->eta());
        }
      }
      //tower 7 : 3/4 & 4/5 & EndcapL2 off
      if (fabs(muon->eta()) >=0.93  && fabs(muon->eta()) < 1.04) { 
        hEfficGeomTot_M->Fill(muon->eta()); 
	if (nDetsBL>=3 || ( (nDetsBL+nDetsE)>=4 && !detEndcap->at(1)) ) hEfficGeomTot_D->Fill(muon->eta());
	if (nHitsBL>=3 || ( (nHitsBL+nHitsE)>=4 && !hitEndcap->at(1)) ) {
	          hEfficGeomTot_H->Fill(muon->eta());
          if (l1Rpcs.size()>0) hEfficGeomTot_T->Fill(muon->eta());
        }
      }
      //tower 8:  3/4  Bar L1,L2,Endcap L1,L2 
      if (fabs(muon->eta()) >=1.04  && fabs(muon->eta()) < 1.14) { 
        hEfficGeomTot_M->Fill(muon->eta()); 
	if (( (nDetsBL+nDetsE)>=3 && !detBarrel->at(2)) ) hEfficGeomTot_D->Fill(muon->eta());
	if (( (nHitsBL+nHitsE)>=3 && !hitBarrel->at(2)) ) {
	          hEfficGeomTot_H->Fill(muon->eta());
          if (l1Rpcs.size()>0) hEfficGeomTot_T->Fill(muon->eta());
        }
      }
      //tower 9:  3/4  Bar L1,L2,Endcap L1,L2 
      if (fabs(muon->eta()) >=1.14  && fabs(muon->eta()) <= 1.25) { 
        hEfficGeomTot_M->Fill(muon->eta()); 
	if (( (nDetsBL+nDetsE)>=3 && !detBarrel->at(1) && !detEndcap->at(0)) ) hEfficGeomTot_D->Fill(muon->eta());
	if (( (nHitsBL+nHitsE)>=3 && !hitBarrel->at(1) && !hitEndcap->at(0)) ) {
	          hEfficGeomTot_H->Fill(muon->eta());
          if (l1Rpcs.size()>0) hEfficGeomTot_T->Fill(muon->eta());
        }
      }


      /*
      //overlap 
      if (fabs(muon->eta()) >=0.8  && fabs(muon->eta()) <= 1.25) { 
        hEfficGeomTot_M->Fill(muon->eta()); 
// 	if (nDetsBL>=3 || (nDetsB>=4 && (detBarrel->at(4)||detBarrel->at(5)) ) || nDetsE>=3) hEfficGeomTot_D->Fill(muon->eta());
// 	if (nHitsBL>=3 || (nHitsB>=4 && (hitBarrel->at(4)||hitBarrel->at(5)) ) || nHitsE>=3) {
	

	bool war0 = (nHitsB+nHitsE) >= 3;
	bool war1=  (nHitsB>=1 && nHitsE >= 2 && hitEndcap->at(1) && hitEndcap->at(2) )|| nHitsBL >=3 ;


	if ( fabs(muon->eta()) >1.04 && fabs(muon->eta()) <1.24 && ( ( nHitsB+nHitsE) >= 3)  != 
	     (( nHitsB>=1 && nHitsE >= 2 && hitEndcap->at(1) && hitEndcap->at(2) )|| nHitsBL >=3 ))
	  std::cout<<"coto "<< muon->eta()
		   <<" war.ogol " << war0
		   <<" war.szcz " << war1
		   << " det "<<nDetsB <<" "<< nDetsBL<<" "<< nDetsE
		   << " hits "<<nHitsB <<" "<< nHitsBL<<" "<< nHitsE
		   <<" "<<nHitsB+nHitsE<< " trig:" << l1Rpcs.size();
	std::cout<< "    BarrelHits: ";
	for(int i=0;i<6;i++)std::cout<<hitBarrel->at(i);
	std::cout<< "  EndcapHits: ";
	for(int  i=0;i<3;i++)std::cout<<hitEndcap->at(i);
	std::cout<<endl;


	
 	if ((nDetsB+nDetsE) >= 3) hEfficGeomTot_D->Fill(muon->eta());
 	if ((nHitsB+nHitsE) >= 3) {

// 	if (nDetsB+nDetsE >= 3) hEfficGeomTot_D->Fill(muon->eta());
// 	if (( nHitsB>=1 && nHitsE >= 2 && hitEndcap->at(1) && hitEndcap->at(2) ) 
// 	    || ( fabs(muon->eta()) <= .93 && nHitsBL>=3 )) {

          hEfficGeomTot_H->Fill(muon->eta());
          if (l1Rpcs.size()>0) hEfficGeomTot_T->Fill(muon->eta());
        }
      }
      */
    }

    //
    // TIMING
    // 
    if (l1Rpcs.size() > 0) {
      int firstBX = 100;
      for (std::vector<L1Obj>::const_iterator it=l1Rpcs.begin(); it!= l1Rpcs.end(); ++it) {

        if ( (it->bx) < firstBX) { firstBX = it->bx; }

        hL1RpcBX_all->Fill(it->bx);
        hL1RpcEta->Fill(it->eta);
        hL1RpcPhi->Fill(it->phi);
        h2L1RpcBX->Fill(it->eta,it->phi);
        hL1RpcGBX_all->Fill(event->bx+it->bx); // global bunch crossing number of L1RPC candidate

        if (it->q == 0){ hL1RpcEta_q0->Fill(it->eta); hL1RpcPhi_q0->Fill(it->phi);h2L1RpcBX_q0->Fill(it->eta,it->phi); }
        if (it->q == 1){ hL1RpcEta_q1->Fill(it->eta); hL1RpcPhi_q1->Fill(it->phi);h2L1RpcBX_q1->Fill(it->eta,it->phi); }
        if (it->q == 2){ hL1RpcEta_q2->Fill(it->eta); hL1RpcPhi_q2->Fill(it->phi);h2L1RpcBX_q2->Fill(it->eta,it->phi); }
        if (it->q == 3){ hL1RpcEta_q3->Fill(it->eta); hL1RpcPhi_q3->Fill(it->phi);h2L1RpcBX_q3->Fill(it->eta,it->phi); }

        // off peak BX-es ?
        if (it->bx){
          h2L1RpcBX1->Fill(it->eta,it->phi);
          if (it->q == 0) h2L1RpcBX1_q0->Fill( it->eta, it->phi);
          if (it->q == 1) h2L1RpcBX1_q1->Fill( it->eta, it->phi);
          if (it->q == 2) h2L1RpcBX1_q2->Fill( it->eta, it->phi);
          if (it->q == 3) h2L1RpcBX1_q3->Fill( it->eta, it->phi);
        }
      }
      // the earliest candidate
      hL1RpcBX->Fill(firstBX);
      hL1RpcGBX->Fill(event->bx+firstBX); // global bunch crossing number of L1RPC candidate

      //############################################
      // TRAP for PRE-FIRED/POST-FIRED RPC TRIGGERS
      //############################################
     
    //  if (firstBX !=0) {
      if (false && toto) {
        std::cout <<" event: " << ev << std::endl;
        if (firstBX < 0) std::cout<< "RPC Pre-firing! ";
        if (firstBX > 0) std::cout<< "RPC Post-firing! ";
        std::cout<< " Run:"<< event->run
                 << " Evt:"<<event->id
                 << " LumiSec: "<< event->lumi
                 << " Time: "<< event->time
                 << " Orbit: "<< event->orbit
                 << " BX:"<< event->bx
                 << " / "
                 << " size_L1RPC: "<< l1Rpcs.size() 
                 << " first_L1RPC_BX: " << firstBX
                 << std::endl;
      } 
    }


 
    //
    // ADDITIONAL HISTOGRAMS WITH TRACK AS DENOM.
    //
    bool isTrack = (track->pt() > theConfig.getParameter<double>("ptMin") );
    if (isTrack) {
      //if (fabs(track.eta) > 0.8) continue;
      //if (fabs(track.eta) < 1.24) continue;
      hTrackPt->Fill(track->pt());
      hTrackEta->Fill(track->eta());
      hTrackPhi->Fill(track->phi());

      hEfficTk_D->Fill(track->eta());
      if (l1Rpcs.size()) hEfficTk_N->Fill(track->eta()); 

      hEfficTkPt_D->Fill(track->pt());
      if (l1Rpcs.size())     hEfficTkPt_N->Fill(track->pt());
      if (l1RpcColl->getL1ObjsMatched(theConfig.getParameter<double>("l1Cut")).size()) hEfficTkPt7_N->Fill(track->pt()); 
    }

    // L1 RPC candidates
    if (l1Rpcs.size()) hDistL1Rpc->Fill(l1Rpcs[0].eta, l1Rpcs[0].phi);

   
    // L1 DT/CSC candidate
    if (l1Others.size()) hDistL1Other->Fill(l1Others[0].eta, l1Others[0].phi);

    //Without affected stations

    if (  !(muon->phi() < 2. && muon->phi() > -0.2) &&  !(muon->phi() < -1.4 && muon->phi() > -3.14)  && !(muon->phi() >2.9 && muon->phi() < 3.14)) {
    if (l1Rpcs.size()) hDistL1Rpc_good->Fill(l1Rpcs[0].eta, l1Rpcs[0].phi); // TODO: why [0] ??
    // L1 DT/CSC candidate
    if (l1Others.size()) hDistL1Other_good->Fill(l1Others[0].eta, l1Others[0].phi); // TODO: why [0] ??
    }
  } // end of event loop
 std::cout <<"RPCTriggers: "<< RPCTriggers << " RPCTriggersOnly: " << RPCTriggersOnly <<  "OtherTriggers: "<<OtherTriggers<< " OtherTriggersOnly: "<< OtherTriggersOnly <<std::endl;


  /*
  //
  // SUMMARIES RUN/LUMI EFFIC INTO GRAPH
  // average efficiency per LumiSection
  hGraphLumi->Set(effLumiMap.size());
  unsigned int iPoint=0;
  for( EffLumiMap::const_iterator im = effLumiMap.begin(); im != effLumiMap.end(); ++im) {
    float eff = 0.;
    if (im->second.second != 0) eff = float(im->second.first)/float(im->second.second);
    float effErr = sqrt( (1-eff)*im->second.first)/im->second.second; 
//     std::cout <<" RUN: "<<im->first.first
//               <<" LS: "<<im->first.second
//               <<" Effic: "<< eff <<" ("<<im->second.first<<"/"<<im->second.second<<")"<<std::endl; 
    hEffLumi->Fill(eff);
    hGraphLumi->SetPoint(iPoint, im->first.first + im->first.second*0.001, eff);
    hGraphLumi->SetPointError(iPoint, 0., effErr);
    iPoint++;
  } 

  */


  // average efficiency per Lumi
  int nPoints = 0; 
  for( EffLumiMap::const_iterator im = effLumiMap.begin(); im != effLumiMap.end(); ++im) {
    //reject wrong runs
//    if(im->first.first == 146417. || im->first.first == 146421. ) continue;
    if(im->first.first == 147115. || im->first.first == 146417. || im->first.first == 146421. || im->first.first ==147749. ) continue;
// if( im->first.first < theConfig.getParameter<double>("noRun") ) continue;
  std::vector<L1Obj> l1Others = l1OtherColl->getL1Objs();
 if(l1Others.size() ==0 ) continue;

  if (im->second.first != 0) ++nPoints; 
  }
  hGraphLumi->Set(nPoints);

  int  iPoint=0;
  for( EffLumiMap::const_iterator im = effLumiMap.begin(); im != effLumiMap.end(); ++im) {
    //reject wrong runs
//     if(im->first.first == 146417. || im->first.first == 146421. ) continue;
     if(im->first.first == 147115. || im->first.first == 146417. || im->first.first == 146421. || im->first.first ==147749. ) continue;
 //if( im->first.first < theConfig.getParameter<double>("noRun") ) continue;
 
    std::vector<L1Obj> l1Others = l1OtherColl->getL1Objs();
if(l1Others.size() ==0 ) continue;
    float eff = 0.;
    if (im->second.first==0 ) continue;
    if (im->second.second != 0) eff = float(im->second.first)/float(im->second.second); 
    float effM1 = float(im->second.first-1)/float(im->second.second);
    float effErr = sqrt( (1-effM1)*std::max((int) im->second.first,1))/im->second.second; 
    hEffLumi->Fill(eff, 1./sqr(effErr));
    hGraphLumi->SetPoint(iPoint, im->first.first + im->first.second*0.001, eff);
    hGraphLumi->SetPointError(iPoint, 0., effErr);
    iPoint++;
  } 

 
  // average efficiency per Run
  nPoints = 0; 
  for( EffRunMap::const_iterator im = effRunMap.begin(); im != effRunMap.end(); ++im){
  //reject wrong runs
//    if(im->first == 146417 || im->first == 146421 ) continue;
if(im->first == 147115. || im->first == 146417 || im->first == 146421 ) continue;
// if( im->first < theConfig.getParameter<double>("noRun") ) continue;

std::vector<L1Obj> l1Others = l1OtherColl->getL1Objs();
if(l1Others.size() ==0 ) continue;
    if (im->second.first != 0) ++nPoints; 
  }
  hGraphRun->Set(nPoints);
  hGraphRunBarrel->Set(nPoints);
  hGraphRunEndcap->Set(nPoints);

  iPoint=0;
  for( EffRunMap::const_iterator im = effRunMap.begin(); im != effRunMap.end(); ++im) {

    //reject wrong runs
//    if(im->first == 146417 || im->first == 146421 ) continue;
  if(im->first == 147115 || im->first == 146417 || im->first == 146421 ) continue;
//  if( im->first < theConfig.getParameter<double>("noRun") ) continue;

  std::vector<L1Obj> l1Others = l1OtherColl->getL1Objs();
if(l1Others.size() ==0 ) continue;

    float eff = 0.;
    if (im->second.first==0 ) continue;
    if (im->second.second != 0) eff = float(im->second.first)/float(im->second.second); 
    float effM1 = float(im->second.first-1)/float(im->second.second);
    float effErr = sqrt( (1-effM1)*std::max((int) im->second.first,1))/im->second.second; 
    std::cout <<" RUN: "<<im->first
              <<" Effic: "<< eff <<" ("<<im->second.first<<"/"<<im->second.second<<")"<<std::endl; 
    hEffRun->Fill(eff, 1./sqr(effErr));
    hGraphRun->SetPoint(iPoint, im->first, eff);
    hGraphRun->SetPointError(iPoint, 0., effErr);
    iPoint++;
  }
  ////////////////////
  iPoint=0;
  for( EffRunMap::const_iterator im = effRunMapBarrel.begin(); im != effRunMapBarrel.end(); ++im) {
    if(im->first == 147115 || im->first == 146417 || im->first == 146421 ) continue;

    std::vector<L1Obj> l1Others = l1OtherColl->getL1Objs();
    if(l1Others.size() ==0 ) continue;

    float eff = 0.;
    if (im->second.first==0 ) continue;
    if (im->second.second != 0) eff = float(im->second.first)/float(im->second.second); 
    float effM1 = float(im->second.first-1)/float(im->second.second);
    float effErr = sqrt( (1-effM1)*std::max((int) im->second.first,1))/im->second.second; 
    std::cout <<" RUN: "<<im->first
              <<" Effic barrel: "<< eff <<" ("<<im->second.first<<"/"<<im->second.second<<")"<<std::endl; 
    hGraphRunBarrel->SetPoint(iPoint, im->first, eff);
    hGraphRunBarrel->SetPointError(iPoint, 0., effErr);
    iPoint++;
  }
 ////////////////////
  iPoint=0;
  for( EffRunMap::const_iterator im = effRunMapEndcap.begin(); im != effRunMapEndcap.end(); ++im) {
    if(im->first == 147115 || im->first == 146417 || im->first == 146421 ) continue;

    std::vector<L1Obj> l1Others = l1OtherColl->getL1Objs();
    if(l1Others.size() ==0 ) continue;

    float eff = 0.;
    if (im->second.first==0 ) continue;
    if (im->second.second != 0) eff = float(im->second.first)/float(im->second.second); 
    float effM1 = float(im->second.first-1)/float(im->second.second);
    float effErr = sqrt( (1-effM1)*std::max((int) im->second.first,1))/im->second.second; 
    std::cout <<" RUN: "<<im->first
              <<" Effic endcap: "<< eff <<" ("<<im->second.first<<"/"<<im->second.second<<")"<<std::endl; 
    hGraphRunEndcap->SetPoint(iPoint, im->first, eff);
    hGraphRunEndcap->SetPointError(iPoint, 0., effErr);
    iPoint++;
  }
  unsigned int iRun = 1;
  TH2D *hDeltaPtVsEtaVsRun = new TH2D("hDeltaPtVsEtaVsRun","",nEtaBins,EtaBins,histoRunMap.size()+1,-0.5,histoRunMap.size()+0.5);
  histos.Add(hDeltaPtVsEtaVsRun);
  for(std::map< unsigned int,TH2D* >::const_iterator im = histoRunMap.begin(); im != histoRunMap.end(); ++im) {	    
    TH2D *hTmp =   im->second;
    TH1D *hShift = getDeltaPtFit(hTmp);
    histos.Add(hTmp);
    for(int iBin=0;iBin<hShift->GetNbinsX()+1;++iBin){
      hDeltaPtVsEtaVsRun->SetBinContent(iBin,iRun,hShift->GetBinContent(iBin));
    }
    hDeltaPtVsEtaVsRun->GetYaxis()->SetBinLabel(iRun,Form("%d",im->first));
    iRun++;
  }


  //  chain.ResetBranchAddresses(); 
  std::string histoFile = theConfig.getParameter<std::string>("histoFileName");
  TFile f(histoFile.c_str(),"RECREATE");
  histos.Write();
  hGraphLumi->Write("hGraphLumi");
  hGraphRun->Write("hGraphRun");
  hGraphRunBarrel->Write("hGraphRunBarrel");
  hGraphRunEndcap->Write("hGraphRunEndcap");
  f.Close();
 
  cout <<"KUKU"<<endl;
}
///////////////////////////////////////////////
///////////////////////////////////////////////
TH1D* EfficiencyAnalysis::getDeltaPtFit(TH2D *hDeltaPtVsEta){

  TF1 *func = new TF1("func","gaus(0)",-1,5);
  func->SetParameter(0,1);
  func->SetParameter(1,1);
  func->SetParameter(2,1);

  TH1D *hShifts =  hDeltaPtVsEta->ProjectionX("hShifts");
  hShifts->Reset();

  //for(int iBinX=17;iBinX<18;++iBinX){
  for(int iBinX=0;iBinX<18;++iBinX){
  //for(int iBinX=5;iBinX<33;++iBinX){ Tower 12
    TH1D *hProjTowerM = hDeltaPtVsEta->ProjectionY("hProjTowerM",iBinX,iBinX);
    TH1D *hProjTowerP = hDeltaPtVsEta->ProjectionY("hProjTowerP",34-iBinX,34-iBinX);    
    hProjTowerM->SetLineColor(2);
    hProjTowerM->Add(hProjTowerP);
    hProjTowerM->Fit(func,"");
    float meanShift = func->GetParameter(1);
    float meanShiftError = func->GetParError(1);
    if(fabs(meanShift-1.0)<1e-5) meanShift = 0.0;
    hShifts->SetBinContent(iBinX,meanShift);
    hShifts->SetBinError(iBinX,meanShiftError);
    func->SetParameter(0,1);
    func->SetParameter(1,1);
    func->SetParameter(2,1);
  }

  delete func;
  return hShifts;
}
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
