
unsigned int nPtBins = 32;

double ptBins[33]={0., 0.1, 
		 1.5, 2., 2.5, 3., 3.5, 4., 4.5, 5., 6., 7., 8., 
		 10., 12., 14., 16., 18., 20., 25., 30., 35., 40., 45., 
		 50., 60., 70., 80., 90., 100., 120., 140., 
		 160. };
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
void plotRecoPt(string sysType="Otf",
		int ipt=22){

  string path = "/home/akalinow/scratch/CMS/OverlapTrackFinder/Dev1/job_4_ana/"; 
  path+= string(TString::Format("SingleMu_%d_p/",ipt));

  
  TFile *file = new TFile((path+"EfficiencyTree.root").c_str());
  if (file->IsZombie()) return;
  
  TTree *tree = (TTree*)file->Get("efficiencyTree");
  
  TH1F *h1D = new TH1F("h1D",";p_{T}^{L1} [GeV/c];",nPtBins,ptBins);
  //h1D->SetStats(kFALSE);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(11);
  gStyle->SetStatW(0.3);
  gStyle->SetStatY(0.9);
  gStyle->SetStatX(0.9);
  
  TH1F *h = (TH1F*)h1D->Clone(TString::Format("h%s",sysType.c_str()));
  
  tree->Draw(TString::Format("l1Objects%s[0].pt>>h%s",sysType.c_str(),sysType.c_str())
	     ,"","goff");

  TCanvas* c = new TCanvas(TString::Format("RecoPt_%s",sysType.c_str()),"RecoPt",460,500);			  
  h->Draw();
  h->Fit("gaus","S","",ptBins[ipt]-30,ptBins[ipt]+30);

  TLatex aLatex(100,4000,
		TString::Format("%d<p_{T}^{gen}<%d",(int)ptBins[ipt],(int)ptBins[ipt+1]).Data());
  aLatex.Draw();
  
  float x = (ptBins[ipt] + ptBins[ipt+1])/2.0;
  TLine aLine(x,0,x,4000);
  aLine.SetLineWidth(3);
  aLine.Draw();

  c->Print(TString::Format("fig_eps/RecoPt%s_%d.eps",sysType.c_str(),ipt).Data());
  c->Print(TString::Format("fig_png/RecoPt%s_%d.png",sysType.c_str(),ipt).Data());

}
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
void plotRecoPhi(int ipt=0){


  string path = "/home/akalinow/scratch/CMS/OverlapTrackFinder/Dev1/job_4_ana/"; 
  if(ipt>0) path+= string(TString::Format("SingleMu_%d_m/",ipt));

  
  TFile *file = new TFile((path+"EfficiencyTree.root").c_str());
  if (file->IsZombie()) return;
  
  TTree *tree = (TTree*)file->Get("efficiencyTree");
  
  TH1F *h1D = new TH1F("h1D",";#varphi^{L1}-#varphi^{SimHit@ref st.};",100,-0.2,0.2);
  h1D->SetLineWidth(3);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(11);
  gStyle->SetStatW(0.3);
  gStyle->SetStatY(0.9);
  gStyle->SetStatX(0.9);
  
  TH1F *hOtf = (TH1F*)h1D->Clone("hOtf");
  TH1F *hGmt = (TH1F*)h1D->Clone("hGmt");
 
  hGmt->SetLineColor(2);
  hGmt->SetLineStyle(2);
  hOtf->SetLineColor(4);
 
  std::string sysType = "Otf";
  tree->Draw(TString::Format("l1Objects%s[0].phi - phiHit>>h%s",sysType.c_str(),sysType.c_str())
	     ,"l1ObjectsOtf[0].q%100>4","goff");

  std::string sysType = "Gmt";
  tree->Draw(TString::Format("l1Objects%s[0].phi - phiHit>>h%s",sysType.c_str(),sysType.c_str())
	     ,"","goff");

  TCanvas* c = new TCanvas("RecoPhiRes","RecoPhiRes",460,500);
  TLegend *leg = new TLegend(0.6074561,0.6800847,0.8464912,0.8728814,NULL,"brNDC");
  leg->SetTextSize(0.05);
  leg->SetFillStyle(4000);
  leg->SetBorderSize(0);
  leg->SetFillColor(10);			  

  hOtf->Draw();
  hGmt->Draw("same");
  leg->AddEntry(hOtf,"OTF");
  leg->AddEntry(hGmt,"GMT");
  leg->Draw();

  if(ipt>0){
    TLatex aLatex(0.02,1500,
		  TString::Format("%d<p_{T}^{gen}<%d",(int)ptBins[ipt],(int)ptBins[ipt+1]).Data());
    aLatex.Draw();
  }
  

  c->Print(TString::Format("eps/RecoPhiRes_%d.eps",ipt).Data());
  c->Print(TString::Format("png/RecoPhiRes_%d.png",ipt).Data());

}
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
void plotLLH(int ipt=22){

  string path = "/home/akalinow/scratch0/OverlapTrackFinder/job_4_ana/"; 
  path+= string(TString::Format("SingleMu_%d_m/",ipt));

  TFile *file = new TFile((path+"EfficiencyTree.root").c_str());
  if (file->IsZombie()) return;
  
  TTree *tree = (TTree*)file->Get("efficiencyTree");

  TH1F *hLLH = new TH1F("hLLH","",50,-100,0);
  hLLH->SetStats(kFALSE);

  TH1F *hLLHgood = hLLH->Clone("hLLHgood");
  TH1F *hLLHbad = hLLH->Clone("hLLHbad");

  tree->Draw("l1ObjectsOtf.eta>>hLLHgood",
	     TString::Format("l1ObjectsOtf.pt>%3.2f && l1ObjectsOtf.pt<%3.2f",ptBins[ipt-1],ptBins[ipt+1]).Data(),
	     "goff");

  tree->Draw("l1ObjectsOtf.eta>>hLLHbad",
	     TString::Format("l1ObjectsOtf.pt<%3.2f",ptBins[ipt]).Data(),
	     "goff");


  hLLHgood->Scale(1.0/hLLHgood->Integral());
  hLLHbad->Scale(1.0/hLLHbad->Integral());

  hLLHbad->SetLineColor(2);

  hLLHgood->Draw();
  hLLHbad->Draw("same");

}
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
void plotGoldenPattern(int iPt=9, int iTower=9, int iPhi=2260, int iCharge=-1){

  string path = "/home/akalinow/scratch/CMS/OverlapTrackFinder/job_3_pat/SingleMu_31_p/";
  TFile *file = new TFile((path+"GoldenPatterns.root").c_str());


  TObjArray grRPC, grCSC, grDT;

  iCharge*=-1;
  iCharge+=1;
  TString dirName = TString::Format("fig_png/iPt_%d_iTower_%d_iPhi_%d_iCh_%d",iPt,iTower,iPhi,iCharge);
  gSystem->mkdir(dirName);

  TString locName = TString::Format("tower%d_phi%d_pt%d_sign%d",iTower,iPhi,iPt,iCharge);
  ////
  TString cName = "RPC"+locName;
  TCanvas *cRPC = (TCanvas*)file->Get(cName.Data());
  cName = "DT"+locName;
  TCanvas *cDT = (TCanvas*)file->Get(cName.Data());
  cName = "CSC"+locName;
  TCanvas *cCSC = (TCanvas*)file->Get(cName.Data());
  ////
  TList *aPrimList = cRPC->GetListOfPrimitives();
  TIter padItr(aPrimList);
  TObject *aPad = padItr();
  while (aPad){
    TIter objItr(((TPad*)aPad)->GetListOfPrimitives());
    TObject *obj = objItr();
    while (obj){
      string aName(obj->GetName());
      if(aName.find("Station")!=std::string::npos){
	TGraph *aGr = (TGraph*)obj; 
	Double_t x,y, sum=0;
	for(int i=0;i<aGr->GetN();++i){aGr->GetPoint(i,x,y);sum+=y;}
	std::cout<<aGr->GetName()<<" sum: "<<sum<<std::endl;
	if(sum>1E-2) grRPC.Add(aGr);			
      }
      obj = objItr();
    }
    aPad = padItr();
  }
  /////////
  aPrimList = cCSC->GetListOfPrimitives();
  TIter padItr(aPrimList);
  aPad = padItr();
  while (aPad){
    TIter objItr(((TPad*)aPad)->GetListOfPrimitives());
    TObject *obj = objItr();
    while (obj){
      string aName(obj->GetName());
      if(aName.find("Station")!=std::string::npos){
	TGraph *aGr = (TGraph*)obj;
	Double_t x,y, sum=0;
	for(int i=0;i<aGr->GetN();++i){aGr->GetPoint(i,x,y);sum+=y;}
	std::cout<<aGr->GetName()<<" sum: "<<sum<<std::endl;
	if(sum>1E-2) grCSC.Add(aGr);
      }
      obj = objItr();
    }
    aPad = padItr();
  }
  ////
  aPrimList = cDT->GetListOfPrimitives();
  TIter padItr(aPrimList);
  aPad = padItr();
  while (aPad){
    TIter objItr(((TPad*)aPad)->GetListOfPrimitives());
    TObject *obj = objItr();
    while (obj){
      string aName(obj->GetName());
      if(aName.find("Station")!=std::string::npos){
	TGraph *aGr = (TGraph*)obj; 
	Double_t x,y, sum=0;
	for(int i=0;i<aGr->GetN();++i){aGr->GetPoint(i,x,y);sum+=y;}
	std::cout<<aGr->GetName()<<" sum: "<<sum<<std::endl;
	if(sum>1E-2) grDT.Add(aGr);
      }
      obj = objItr();
    }
    aPad = padItr();
  }
  ////
  std::cout<<grRPC.GetEntries()<<std::endl;
  std::cout<<grCSC.GetEntries()<<std::endl;
  std::cout<<grDT.GetEntries()<<std::endl;

  TCanvas* c = new TCanvas("GoldenPattern","GoldenPattern",460,500);	
  c->SetRightMargin(0.02);
  c->SetLeftMargin(0.15);

  for(int i=0;i<grRPC.GetEntries();++i){
    TGraph *gr = (TGraph*)grRPC.At(i);
    gr->GetXaxis()->SetLabelSize(0.06);
    gr->GetXaxis()->SetLabelOffset(0.03);
    gr->GetYaxis()->SetLabelSize(0.06);
    gr->SetFillColor(kBlue-10);
    gr->Draw("A b");
    c->Print(TString::Format("%s/pdf_%s.png",dirName.Data(),gr->GetName()));
  }
  /////
  for(int i=0;i<grDT.GetEntries();++i){
    TGraph *gr = (TGraph*)grDT.At(i);
    gr->GetXaxis()->SetLabelSize(0.06);
    gr->GetXaxis()->SetLabelOffset(0.03);
    gr->GetYaxis()->SetLabelSize(0.06);
    gr->SetFillColor(kBlue-10);
    gr->Draw("A b");
    c->Print(TString::Format("%s/pdf_%s.png",dirName.Data(),gr->GetName()));
  }
  //////
  for(int i=0;i<grCSC.GetEntries();++i){
    TGraph *gr = (TGraph*)grCSC.At(i);
    gr->GetXaxis()->SetLabelSize(0.06);
    gr->GetXaxis()->SetLabelOffset(0.03);
    gr->GetYaxis()->SetLabelSize(0.06);
    gr->SetFillColor(kBlue-10);
    gr->Draw("A b");
    c->Print(TString::Format("%s/pdf_%s.png",dirName.Data(),gr->GetName()));
  }
  //////
}
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
void plots(){


  plotRecoPt("Otf",22);
  plotRecoPt("Gmt",22);
  plotRecoPhi(22);
  return;

  //plotGoldenPattern(31,6,10000,1);  
  //plotGoldenPattern(31,7,10000,1);  
  //return;
  ///
  plotRecoPhi(0);
  plotRecoPhi(22);
  ///
  plotRecoPt("Otf",16);
  plotRecoPt("Gmt",16);
  ///
  plotRecoPt("Otf",22);
  plotRecoPt("Gmt",22);
  ///
  // plotGoldenPattern(int iPt=9, int iTower=9, int iPhi=2260, int iCharge=-1)
  plotGoldenPattern(9,6,2260,-1);  
  plotGoldenPattern(9,9,2260,-1);  
  //plotLLH(22);

  
}
