#include "TSBSSimAuxi.h"
#include "TMath.h"
#define DEBUG 0

//ClassImp(TSpectroInfo) // Implements TSpectroInfo
ClassImp(TRndmManager) // Implements TRndmManager
ClassImp(TGeoInfo) // Implements TGeoInfo
ClassImp(TDigInfo) // Implements TDigInfo
ClassImp(TDigSlot) // Implements TDigSlot
ClassImp(TDigGEMSlot) // Implements TDigSlot
ClassImp(TDetInfo) // Implements TDetInfo
ClassImp(TSPEModel) // Implements TSPEModel
ClassImp(TPMTSignal) // Implements TPMTSignal
ClassImp(TSignalInfo) // Implements TSignalInfo

using namespace std;
//
// Class TRndmManager
//
TRndmManager * TRndmManager::fRndmMan = NULL;

TRndmManager::TRndmManager() 
{
}
//______________________________________________________________
TRndmManager::~TRndmManager()
{
}


//
// Class TNPEModel
//
TSPEModel::TSPEModel(const char* detname, 
		     double tau, double sigma, 
		     double t0, double tmin, double tmax)
{
#if DEBUG>=1
  cout << " making new TSPEmodel for detector " << detname << endl;
#endif  
  TF1 fFunc1(Form("fFunc1%s",detname),
  	     TString::Format("TMath::Max(0.,(x/%g)*TMath::Exp(-x/(%g)))", 
  			     tau*tau, tau),
  	     tmin ,tmax);
  TF1 fFunc2(Form("fFunc2%s",detname), "gaus", 
  // 	     TString::Format("%g*TMath::Exp(-((x-%g)**2)/(%g))", 
  // 			     1./TMath::Sqrt(2*TMath::Pi()*sigma), t0, sigma*sigma),
   	     tmin, tmax);
  fFunc2.SetParameters(1.0e0/10.0/(sqrt(2*TMath::Pi())*sigma), 0, sigma);//do it this way for thaat purpose
  
  //TF1Convolution is deemed too premature to be used - and indeed pretty slow
  //TF1Convolution fConvolution(&fFunc1, &fFunc2);
  //fPulseModel = new TF1(Form("fSignal%s",detname), fConvolution, tmin, tmax, fConvolution.GetNpar());
  
  const int NbinsTotal = int(tmax-tmin)*10;// 10 bins/ns should do... since we will extrapolate after...
  fPulseHisto = new TH1D(Form("fPulseHisto%s",detname), "", NbinsTotal, tmin, tmax);
  double t_i, t_j;
  double ps_i, g_j;
  for(int i = 1; i<=NbinsTotal; i++){
    t_i = fPulseHisto->GetBinCenter(i+1);
    ps_i = fFunc1.Eval(t_i);
    if(sigma>0){
      fFunc2.SetParameter(1, t_i);
      for(int j = 1; j<=NbinsTotal; j++){
	t_j = fPulseHisto->GetBinCenter(j+1);
	g_j = fFunc2.Eval(t_j);
	fPulseHisto->Fill(t_j, ps_i*g_j);
      }
    }else{
      fPulseHisto->Fill(t_i, ps_i);
    }
  }
  
  cout << endl<< detname << " pulse histo built with address << " << fPulseHisto << endl;
#if DEBUG>2
  for(int i = 0; i<NbinsTotal; i++){
    if(fPulseHisto->GetBinContent(i)>0)cout << fPulseHisto->GetBinContent(i) << " ";
  }
  cout << endl;
#endif
  
}

bool TSPEModel::PulseOverThr(double charge, double thr)
{
#if DEBUG>0
  cout << "fPulseHisto address " << fPulseHisto << endl;
  cout //<< "unnormalized pulse max (ns -1) " << fPulseHisto->GetMaximum() 
       << ", threshold (C/ns) " << thr << ", charge (C) " << charge << endl;
#endif
  //if(fPulseModel->GetMaximum()<thr/charge){
  if(fPulseHisto->GetMaximum()<thr/charge){
    return false;
  }else{
    return true;
  }
};
 
bool TSPEModel::FindLeadTrailTime(double charge, double thr, double &t_lead, double &t_trail)
{
#if DEBUG>0
  cout << "Find Lead and Trail time for pulse charge " << charge << " over threshold " << thr << endl;
  if(fPulseHisto->IsZombie())cout << "fPulseHisto " << fPulseHisto << "is Zombie " << endl;
#endif
  if(!PulseOverThr(charge, thr)){
    t_lead = 1.0e38;
    t_trail = 1.0e38;
    return false;
  }else{
    double xmax = fPulseHisto->GetBinCenter(fPulseHisto->GetMaximumBin());
    if(fPulseHisto->GetBinContent(1)<thr/charge){
      t_lead = GetHistoX(thr/charge, fPulseHisto->GetBinLowEdge(1), xmax);
    }else{
      t_lead = 1.0e38;
      return false;
    }
    if(fPulseHisto->GetBinContent(fPulseHisto->GetNbinsX())<thr/charge){
      t_trail = GetHistoX(thr/charge, xmax, fPulseHisto->GetBinLowEdge(fPulseHisto->GetNbinsX()+1));
    }else{
      t_trail = 1.0e38;
      return false;
    }
    //why does this function seem to oblitarate fMCHit containers when t_trail is calculated to be 1.e38... GetHistoX?
    if(t_trail>1.e30) cout << "thr/charge" << thr/charge << " >? " <<  fPulseHisto->GetBinContent(fPulseHisto->GetNbinsX()) << " or " << fPulseHisto->GetBinContent(fPulseHisto->GetNbinsX()+1) << endl;
    return true;
  }
}

double TSPEModel::GetHistoX(double y, double x1, double x2)
{
  double splineslope;
  for(int k = fPulseHisto->FindBin(x1); k<=fPulseHisto->FindBin(x2); k++){
    if(  ( (fPulseHisto->GetBinContent(k+1)-y)*(fPulseHisto->GetBinContent(k)-y) )<0 ){
      // threshold crossed if diff(TH1::GetBinContent-y) changes sign
      splineslope = (fPulseHisto->GetBinContent(k+1)-fPulseHisto->GetBinContent(k))/(fPulseHisto->GetBinCenter(k+1)-fPulseHisto->GetBinCenter(k));
      
      return fPulseHisto->GetBinCenter(k)+(y-fPulseHisto->GetBinContent(k))/splineslope;
    }
  }
  return 1.0e38;
}



//
// Class TPMTSignal
//
TPMTSignal::TPMTSignal()
  : fSumEdep(0), fNpe(0), fNpeChargeConv(1.0), fADC(0), fEventTime(0), fMCHitSize(0)
{ 
  fLeadTimes.clear();
  fTrailTimes.clear();
  fTDCs.clear();
  
  fRN = TRndmManager::GetInstance();
  
  fMCHitSource.clear();
  fMCHitEdep.clear();
  fMCHitNpe.clear();
  fMCHitTime.clear();
  fMCHitLeadTimes.clear();
  fMCHitTrailTimes.clear();
}

TPMTSignal::TPMTSignal(double npechargeconv)
  : fSumEdep(0), fNpe(0), fNpeChargeConv(npechargeconv), fADC(0), fEventTime(0), fMCHitSize(0)
{ 
  fLeadTimes.clear();
  fTrailTimes.clear();
  fTDCs.clear();
  
  fMCHitSource.clear();
  fMCHitEdep.clear();
  fMCHitNpe.clear();
  fMCHitTime.clear();
  fMCHitLeadTimes.clear();
  fMCHitTrailTimes.clear();
}

void TPMTSignal::Fill(TSPEModel *model, int npe, double thr, double evttime, int signal)
{
  if(signal==0)fEventTime = evttime;
  fNpe+= npe;
  //if(model->PulseOverThr(fCharge, thr))fNpe//fADC = model->GetCharge()*model->GetADCconversion();
  
#if DEBUG>0
  cout << "TPMTSignal::Fill : fNpeChargeConv = " << fNpeChargeConv << endl;
#endif
  
  //determine lead and trail times
  double t_lead, t_trail;
  // find the lead and trail time for *this* pulse, not the total pulse
  bool goodtime = model->FindLeadTrailTime(npe*fNpeChargeConv, thr, t_lead, t_trail);
  
  t_lead+=evttime;
  t_trail+=evttime;
  
#if DEBUG>0
  cout << "past find lead/trail time: t_lead = " << t_lead << ", t_trail = " << t_trail 
       << " (sig = " << npe*fNpeChargeConv << ", thr = " << thr << " mV)." << endl;
    cout << "after " << fMCHitSize << " " << &fMCHitSource << " " << &fMCHitNpe << " " << &fMCHitTime << " " << &fMCHitLeadTimes << " " << &fMCHitTrailTimes << endl;
#endif

  fMCHitSize++;
  fMCHitSource.push_back(signal);
  fMCHitNpe.push_back(npe);
  fMCHitTime.push_back(evttime);
  fMCHitLeadTimes.push_back(t_lead);
  fMCHitTrailTimes.push_back(t_trail);
  
  if(goodtime){//t_lead<1e30 && t_trail<1e30){
    //cout << "past find lead/trail time: t_lead = " << t_lead << ", t_trail = " << t_trail << " (sig = " << npe*fNpeChargeConv << ", thr = " << thr << " mV)." << endl;   
    //Filter here the lead and trail times
    if(fLeadTimes.size()>0){
      // Check if the lead and trail times are inside an existing lead time- trail time pair
      // we assume here that fLeadTimes and fTrailTimes are same size 
      // *if we do things correctly, that should be the case*
      // we shall keep lead-trail times pair in timing order
      // we neglect pulse overlaps ftm.
      if(fLeadTimes.size()!=fTrailTimes.size()){
	cout << " B - Warning: size of lead times container: " << fLeadTimes.size() 
	     << " != size of trail times container: " << fTrailTimes.size() << endl;
      }
      
      for(size_t i = 0; i<fLeadTimes.size(); i++){
	// possibility of the current pair straddling with others.... :/
	// treat those separately to simplify...
	//cout << " i = " << i << " / " << fLeadTimes.size() << endl;
	// tL < tT_i-1 < tL_i < tT
	if(i>0){
	  if(t_lead < fTrailTimes.at(i-1) && fLeadTimes.at(i) < t_trail){
	    //do necessary substitutions first, then erase
	    // tL < tL_i-1 => tL *replaces* tL_i-1
	    if(t_lead < fLeadTimes.at(i-1)){
#if DEBUG>1
	      cout << "a1 - erase el i-1 = " << i+1 << " off " << fLeadTimes.size() << endl;
#endif
	      fLeadTimes.erase(fLeadTimes.begin()+i-1);
	      //cout << "a1 - new size: " << fLeadTimes.size() << " ( i = " << i << ")" << endl;
	      fLeadTimes.insert(fLeadTimes.begin()+i-1, t_lead);
	      //cout << "a1 - new size: " << fLeadTimes.size() << " ( i = " << i << ")" << endl;
	    }
	    // tT_i < tT => tT *replaces* tT_i
	    if(fTrailTimes.at(i) < t_trail){
#if DEBUG>1
	      cout << "a2 - erase el i = " << i << " off " << fLeadTimes.size() << endl;
#endif
	      fTrailTimes.erase(fTrailTimes.begin()+i);
	      //cout << "a1 - new size: " << fTrailTimes.size() << " ( i = " << i << ")" << endl;
	      fTrailTimes.insert(fTrailTimes.begin()+i, t_trail);
	      //cout << "a1 - new size: " << fTrailTimes.size() << " ( i = " << i << ")" << endl;
	    }
#if DEBUG>1
	    cout << "a - erase el i = " << i << " and i-1 off " << fLeadTimes.size() << " " << endl;
#endif
	    fLeadTimes.erase(fLeadTimes.begin()+i);
	    fTrailTimes.erase(fTrailTimes.begin()+i-1);
	    //cout << "a - new size: " << fLeadTimes.size() << " " << fLeadTimes.size() << " ( i = " << i << ")" << endl;
	    break;
	  }
	}//end if(i>0)
	// tL < tT_i < tL_i+1 < tT
	if(i<fLeadTimes.size()-1){
	  if(t_lead < fTrailTimes.at(i) && fLeadTimes.at(i+1) < t_trail){
	    //do necessary substitutions first, then erase
	    // tL < tL_i => tL *replaces* tL_i
	    if(t_lead < fLeadTimes.at(i)){
#if DEBUG>1
	      cout << "b1 - erase el i = " << i << " off " << fLeadTimes.size() << endl;
#endif
	      fLeadTimes.erase(fLeadTimes.begin()+i);
	      //cout << "b1 - new size: " << fLeadTimes.size() << " ( i = " << i << ")" << endl;
	      fLeadTimes.insert(fLeadTimes.begin()+i, t_lead);
	      //cout << "b1 - new size: " << fLeadTimes.size() << " ( i = " << i << ")" << endl;
	    }
	    // tT_i+1 < tT => tT *replaces* tT_i+1
	    if(fTrailTimes.at(i+1) < t_trail){
#if DEBUG>1
	      cout << "b2 - erase el i+1 = " << i+1 << " off " << fLeadTimes.size() << endl;
#endif
	      fTrailTimes.erase(fTrailTimes.begin()+i+1);
	      //cout << "b2 - new size: " << fTrailTimes.size() << " ( i = " << i << ")" << endl;
	      fTrailTimes.insert(fTrailTimes.begin()+i+1, t_trail);
	      //cout << "b2 - new size: " << fTrailTimes.size() << " ( i = " << i << ")" << endl;
	    }
#if DEBUG>1
	    cout << "b - erase el i = " << i << " and i+1 off " << fLeadTimes.size() << " " << endl;
#endif
	    fLeadTimes.erase(fLeadTimes.begin()+i+1);
	    fTrailTimes.erase(fTrailTimes.begin()+i);
	    //cout << "b - new size: " << fLeadTimes.size() << " " << fLeadTimes.size() << " ( i = " << i << ")" << endl;
	    break;
	  }
	}//end if(i<fLeadTimes.size()-1)
	
	// if not, 6 cases to consider:
	// tL < tT < tL_i < tT_i => both inserted *before* existing pair 
	if(t_trail < fLeadTimes.at(i)){
	  //cout << "c1 insert el " << i << " / " << fLeadTimes.size() << endl;
	  fLeadTimes.insert(fLeadTimes.begin()+i, t_lead);
	  fTrailTimes.insert(fTrailTimes.begin()+i, t_trail); 
	  //cout << "c1 - new size: " << fLeadTimes.size() << " " << fLeadTimes.size() << " ( i = " << i << ")" << endl;
	  break;
	}
	// tL < tL_i < tT < tT_i => tL *replaces* tL_i
	if(t_lead < fLeadTimes.at(i) && fLeadTimes.at(i) < t_trail && t_trail < fTrailTimes.at(i)){
#if DEBUG>1
	  cout << "c - erase el i = " << i << " off " << fLeadTimes.size() << endl;
#endif
	  fLeadTimes.erase(fLeadTimes.begin()+i);
	  //cout << "c - new size: " << fLeadTimes.size() << " ( i = " << i << ")" << endl;
	  fLeadTimes.insert(fLeadTimes.begin()+i, t_lead);
	  //cout << "c - new size: " << fLeadTimes.size() << " ( i = " << i << ")" << endl;
	  break;
	}
	// tL_i < tL < tT < tT_i => tL *replaces* tL_i AND tT *replaces* tT_i
	if(t_lead < fLeadTimes.at(i) && fTrailTimes.at(i) < t_trail){
#if DEBUG>1
	  cout << "d - erase el i = " << i << " off " << fLeadTimes.size() << endl;
#endif
	  fLeadTimes.erase(fLeadTimes.begin()+i);
	  //cout << "d - new size: " << fLeadTimes.size() << " ( i = " << i << ")" << endl;
	  fLeadTimes.insert(fLeadTimes.begin()+i, t_lead);
	  //cout << "d - new size: " << fLeadTimes.size() << " ( i = " << i << ")" << endl;
	  fTrailTimes.erase(fTrailTimes.begin()+i);
	  //cout << "d - new size: " << fTrailTimes.size() << " ( i = " << i << ")" << endl;
	  fTrailTimes.insert(fTrailTimes.begin()+i, t_trail);
	  //cout << "d - new size: " << fTrailTimes.size() << " ( i = " << i << ")" << endl;
	  break;
	}
	// tL < tL_i < tT_i < tT => nothing happens
	if(fLeadTimes.at(i) < t_lead && t_trail < fTrailTimes.at(i)){
	  //cout << "e1 - new size: " << fTrailTimes.size() << " ( i = " << i << ")" << endl;
	  break;
	}
	// tL_i < tL   < tT_i < tT   => tT *replaces* tT_i
	if(fLeadTimes.at(i) < t_lead && t_lead < fTrailTimes.at(i) && fTrailTimes.at(i) < t_trail){
#if DEBUG>1
	  cout << "e - erase el i = " << i << " off " << fLeadTimes.size() << endl;
#endif
	  fTrailTimes.erase(fTrailTimes.begin()+i);
	  //cout << "e - new size: " << fTrailTimes.size() << " ( i = " << i << ")" << endl;
	  fTrailTimes.insert(fTrailTimes.begin()+i, t_trail);
	  //cout << "e - new size: " << fTrailTimes.size() << " ( i = " << i << ")" << endl;
	}
	// tL_i < tL   < tT   < tT_i => both inserted *after* existing pair 
	if(fTrailTimes.at(i) < t_lead){
	  //cout << "f1 insert el " << i+1 << " / " << fLeadTimes.size() << endl;
	  fLeadTimes.insert(fLeadTimes.begin()+i+1, t_lead);
	  fTrailTimes.insert(fTrailTimes.begin()+i+1, t_trail);
	  //cout << "f - new size: " << fLeadTimes.size() << " " << fTrailTimes.size() << " ( i = " << i << ")" << endl;
	  break;
	}
	if(fLeadTimes.size()!=fTrailTimes.size()){
	  cout << " A - Warning: size of lead times container: " << fLeadTimes.size() 
	       << " != size of trail times container: " << fTrailTimes.size() << endl;
	}
	if(i>=fLeadTimes.size())cout << "Warning: i = " << i << " >= size of containers = " << fLeadTimes.size() << endl;
      }
    }else{
      //of course, if initial size was 0, just psuh it back
      //hopefully it will be the case most of the time
      //cout << "g - insert (push back): " << fLeadTimes.size() << " " << fTrailTimes.size() << endl;
      fLeadTimes.push_back(t_lead);
      fTrailTimes.push_back(t_trail);
      //cout << "g - new size: " << fLeadTimes.size() << " " << fTrailTimes.size() << endl;
    }
    if(fLeadTimes.size()!=fTrailTimes.size()){
      cout << " A - Warning: size of lead times container: " << fLeadTimes.size() 
	   << " != size of trail times container: " << fTrailTimes.size() << endl;
    }
  }//end if(t_lead && t_trail<30)
  //cout << "end TPMTSignal" << endl;
  //if(!check_vec_size())cout << "Warning: Size of MC info containers don't check out!!!" << endl;

}

void TPMTSignal::Digitize(TDigInfo diginfo, int chan)
{
  if(fNpe<=0){
    fADC = fRN->Gaus(diginfo.Pedestal(chan), diginfo.PedestalNoise(chan));
    return;
  }
    
#if DEBUG>0
  cout << "Charge (C) " << Charge() << " (fC) " << Charge()*1.0e15 << ", ADC conversion (fC/ch) " << diginfo.ADCConversion();
#endif
  
  fADC = TMath::Nint(Charge()*1.0e15/diginfo.ADCConversion()+fRN->Gaus(diginfo.Pedestal(chan), diginfo.PedestalNoise(chan)));
  //if ADC value bigger than number of ADC bits, ADC saturates
  if( fADC>UInt_t(TMath::Nint( TMath::Power(2, diginfo.ADCBits()) )) ){
    fADC = TMath::Nint( TMath::Power(2, diginfo.ADCBits()) );
  }
  //cout << "TPMTSignal::Digitize():  " << fLeadTimes.size() << " - " << fTrailTimes.size() << endl;
  
#if DEBUG>0
  cout << " => ADC = " << fADC << endl;
#endif
  
  UInt_t tdc_value;
  
  // For the sake of going forward, we assume that the signal is the first entry of each vector
  fTDCData.time.clear();
  if(fLeadTimes.size() && fTrailTimes.size()){
    for(size_t i = 0; i<fLeadTimes.size(); i++){
#if DEBUG>0
      cout << " fLeadTimes.at(" << i << ") " << fLeadTimes.at(i) 
	   << " fTrailTimes.at(" << i << ") " << fTrailTimes.at(i) << endl;
#endif
      // trim "all" bits that are above the number of TDC bits - a couple to speed it up
      // (since TDC have a revolving clock, as far as I understand)
      // let's use an arbitrary reference time offset of 1us before the trigger
      tdc_value = TMath::Nint((fLeadTimes.at(i)+1.e3)/diginfo.TDCConversion());
      for(int j = 30; j>=diginfo.TDCBits(); j--){
	tdc_value ^= ( -0 ^ tdc_value) & ( 1 << (j) );
      }
      tdc_value ^= ( -0 ^ tdc_value) & ( 1 << (31) );
      fTDCData.time.push_back(tdc_value);
      //fTDCs.insert(fTDCs.begin()+0, TMath::Nint(fLeadTimes.at(0)*diginfo.TDCConversion()));//bug!!!!
      fTDCs.push_back(tdc_value);//they're already sorted in order, presumably
      // also mark the traling time with setting bin 31 to 1 // need to reconvert then
      tdc_value = TMath::Nint((fTrailTimes.at(i)+1.e3)/diginfo.TDCConversion());
      for(int j = 30; j>=diginfo.TDCBits(); j--){
	tdc_value ^= ( -0 ^ tdc_value) & ( 1 << (j) );
      }
      tdc_value ^= ( -1 ^ tdc_value) & ( 1 << (31) );
      fTDCs.push_back(tdc_value);
      fTDCData.time.push_back(tdc_value);
      
#if DEBUG>0
      cout << " fTDCs.at(0) " << fTDCs.at(0) << " fTDCs.at(1) " << fTDCs.at(1) << endl;
#endif
    }
  }
  
  fSumEdep*=1.0e9;// store in eV.
}

void TPMTSignal::Clear(Option_t*)
{
  //cout << " TPMTSignal::Clear() " << endl;
  
  fSumEdep = 0;
  fNpe = 0;
  fADC = 0;
  
  fEventTime = 0;
  fLeadTimes.clear();
  fTrailTimes.clear();
  fTDCs.clear();
  
  fMCHitSize = 0;
  fMCHitSource.clear();
  fMCHitEdep.clear();
  fMCHitNpe.clear();
  fMCHitTime.clear();
  fMCHitLeadTimes.clear();
  fMCHitTrailTimes.clear();
}

bool TPMTSignal::check_vec_size(bool ignore_edep)
{
  bool b = true;
  if(fMCHitSize!=fMCHitSource.size()){
    printf("fMCHitSource.size() = %zu != %u\n", fMCHitEdep.size(), fMCHitSize);
    b = false;
  }
  if(ignore_edep && fMCHitSize!=fMCHitEdep.size()){
    printf("fMCHitEdep.size() = %zu != %u\n", fMCHitEdep.size(), fMCHitSize);
    b = false;
  }
  if(fMCHitSize!=fMCHitNpe.size()){
    printf("fMCHitNpe.size() = %zu != %u\n", fMCHitNpe.size(), fMCHitSize);
    b = false;
  }
  if(fMCHitSize!=fMCHitTime.size()){
    printf("fMCHitTime.size() = %zu != %u\n", fMCHitTime.size(), fMCHitSize);
    b = false;
  }
  if(fMCHitSize!=fMCHitLeadTimes.size()){
    printf("fMCHitLeadTime.size() = %zu != %u\n", fMCHitLeadTimes.size(), fMCHitSize);
    b = false;
  }
  if(fMCHitSize!=fMCHitTrailTimes.size()){
    printf("fMCHitTrailTime.size() = %zu != %u\n", fMCHitTrailTimes.size(), fMCHitSize);
    b = false;
  }
  return b;
}


//
// Class TDigSlot
//
TDigSlot::TDigSlot() : fCrate(-1), fSlot(-1), fNchan(-1), fChanLo(-1),
  fChanHi(-1)
{
};

TDigSlot::TDigSlot(Int_t crate, Int_t slot, Int_t lo,
    Int_t hi) : fCrate(crate), fSlot(slot), fChanLo(lo), fChanHi(hi)
{
  fNchan = 1 + (fChanHi - fChanLo);
}

TDigSlot::~TDigSlot()
{
}

TDigGEMSlot::TDigGEMSlot() : TDigSlot()
{
};

TDigGEMSlot::TDigGEMSlot(Int_t crate, Int_t slot, Int_t mpd_id,
    Int_t gem_id, Int_t adc_id, Int_t i2c, Int_t pos, Int_t inv) :
  TDigSlot(crate,slot,0,1), fMPDId(mpd_id), fGEMId(gem_id),
  fADCId(adc_id), fI2C(i2c), fPos(pos), fInvert(inv)
{
}

TDigGEMSlot::~TDigGEMSlot()
{
}


Int_t TDigSlot::GetChanNumber(Int_t ch)
{
  Int_t lch = fChanLo + ch;
  if (lch > fChanHi)
    return -1;

  return lch;
}

//
// Class TDigDetMap
//
/*
Int_t TDigDetMap::Fill(std::vector<Int_t> vals)
{
  // TODO: Check if some of the data does not repeat.
  // As a first try, just pre-fill all data the user provides
  for(size_t k = 0; k < vals.size(); k+=4) {
    TDigSlot slot(vals[k],vals[k+1],vals[k+2],vals[k+3]);
    fSlots.push_back(slot);
  }

  return 0;
}
*/


//
// Class TDetInfo
//
TDetInfo::TDetInfo()
{
  fModSlots.clear();
  fGEMSlots.clear();
  fNmodules.clear();
  fGeoInfo.clear();
  fDetUniqueId = -1;
}

TDetInfo::TDetInfo(const std::string detname, const std::string specname)
{
  SetDetName(detname,specname);
  fNmodules.clear();
  fGeoInfo.clear();
}

TDetInfo::~TDetInfo()
{
  fNmodules.clear();
  fGeoInfo.clear();
}


void TDetInfo::SetDetName(std::string detname, std::string specname)
{
  fDetName = detname;
  if(!specname.empty()){
    fDetFullName = specname +"."+detname;
  } else {
    fDetFullName = detname;
  }
}


Int_t TDetInfo::AddSlot(Int_t crate, Int_t slot, Int_t lo, Int_t hi)
{
  TDigSlot modslot(crate,slot,lo,hi);
  fModSlots.push_back(modslot);
  return modslot.GetNchan();
}

Int_t TDetInfo::AddGEMSlot(Int_t crate, Int_t slot, Int_t mpd_id, Int_t gem_id,
    Int_t adc_id, Int_t i2c, Int_t pos, Int_t invert)
{
  TDigGEMSlot modslot(crate,slot,mpd_id,gem_id,
      adc_id,i2c,pos,invert);
  fGEMSlots.push_back(modslot);
  return modslot.GetNchan();
}

TDigChannelInfo TDetInfo::FindLogicalChannelSlot(Int_t lch)
{
  TDigChannelInfo info;
  info.ch = -1;
  info.slot = -1;
  info.crate = -1;
  // If we have a detector map, then use that
  if(!fDetMap.empty()) {
    std::map<int,std::pair<int,int> >::iterator it = fDetMap.find(lch);
    if(it != fDetMap.end() ) {
      TDigSlot &sl = fModSlots[it->second.first];
      info.ch = sl.GetChanNumber(it->second.second);
      info.slot = sl.GetSlot();
      info.crate = sl.GetCrate();
      return info;
    }
  } else if(! fModSlots.empty()) {
    // Otherwise, loop through all the modules and find the one we want
    for(std::vector<TDigSlot>::iterator it = fModSlots.begin();
        it != fModSlots.end(); it++) {
      if ( (*it).GetNchan() > lch ) {
        info.ch = (*it).GetChanNumber(lch);
        info.slot = (*it).GetSlot();
        info.crate = (*it).GetCrate();
        return info;
      }
      lch -= (*it).GetNchan();
    }
  } else {
    // No map of any kind, so then come up with the channel
    // based on fFirstSlot and fFirstCrate
    info.ch = lch%fChanPerSlot;
    info.slot = ((lch-info.ch)/fChanPerSlot)%fSlotPerCrate+fFirstSlot;
    info.crate = (lch-info.slot*fChanPerSlot-info.ch)/fSlotPerCrate+fFirstCrate;
  }

  // Well, if we got to here, then clearly we didn't find it, so instead
  // make it up based on the firstSlot and lastSlot
  return info;

}


void TDetInfo::LoadChannelMap(std::vector<int> chanmap, int start)
{
  // Assume DBManager has checked it for proper size and proceed blindly
  // accepting the format.
  int lch = 0;
  int nmods = fModSlots.size();
  int nch = 0;
  int lch_off = -fNchan;
  for(int i = 0; i < nmods; i++) {
    nch = fModSlots[i].GetNchan();
    for(int k = 0; k < nch; k++) {
      if(lch%int(fNchan)==0) {
        lch_off += fNchan;
      }
      //fDetMap[(lch++)] = std::pair<int,int>(i,k);
      /*
      std::cerr << chanmap[lch] - start + lch_off << " = "
        << i << ", " << k  << " :: chanmap[" << lch
        << "] : " << chanmap[lch] << ", start: " << start
        << ", lch_off: " << lch_off
        << std::endl;
        */
      //fDetMap[ lch++ - start ] = std::pair<int,int>(i,k);
      fDetMap[ chanmap[lch++] - start + lch_off] = std::pair<int,int>(i,k);
    }
  }
}

//
// Class TDigInfo
//
TDigInfo::TDigInfo()
{
  //fRN = new TRandom3(0);
  fGain.clear();
  fPedestal.clear();
  fPedNoise.clear();
  fThreshold.clear();
  fEncoderADC = 0;
  fEncoderTDC = 0;
}

TDigInfo::~TDigInfo()
{
  fGain.clear();
  fPedestal.clear();
  fPedNoise.clear();
  fThreshold.clear();
}

double TDigInfo::Gain(uint chan)
{
  if(fGain.size()>1){
    if(fGain.size()<=chan){
      printf("warning: requested channel number %ud larger than channel size %ld. Check code where this method (DigInfo.Gain(int)) is employed and /or database!\n", 
	     chan, fGain.size());
      exit(-1);
    }
    return fGain.at(chan);
  }else{
    return fGain.at(0);
  }
};

double TDigInfo::Pedestal(uint chan)
{
  if(fPedestal.size()>1){
    if(fPedestal.size()<=chan){
      printf("warning: requested channel number %ud larger than channel size %ld. Check code where this method (DigInfo.Pedestal(int)) is employed and /or database!\n", 
	     chan, fPedestal.size());
      exit(-1);
    }
    return fPedestal.at(chan);
  }else{
    return fPedestal.at(0);
  }
};

double TDigInfo::PedestalNoise(uint chan)
{
  if(fPedNoise.size()>1){
    if(fPedNoise.size()<=chan){
      printf("warning: requested channel number %ud larger than channel size %ld. Check code where this method (DigInfo.PedestalNoise(int)) is employed and /or database!\n", 
	     chan, fPedNoise.size());
      exit(-1);
    }
    return fPedNoise.at(chan);
  }else{
    return fPedNoise.at(0);
  }
};

double TDigInfo::Threshold(uint chan)//returns threshold in C/ns !!!
{
  if(fThreshold.size()>1){
    if(fThreshold.size()<=chan){
      printf("warning: requested channel number %ud larger than channel size %ld. Check code where this method (DigInfo.ThresholdNoise(int)) is employed and /or database!\n", 
	     chan, fThreshold.size());
      exit(-1);
    }
    //Thr(V)/Omega = Thr(A); Thr(A)*unit = Thr(C/ns)
    return fThreshold.at(chan)*spe_unit/fROimpedance;
  }else{
    return fThreshold.at(0)*spe_unit/fROimpedance;
  }
};






/*
//
// Class TNPEModel
//
TNPEModel::TNPEModel(DigInfo diginfo, const char* detname, int npe)
  : fDigInfo(diginfo), fNpe(npe)
{
  fScale = fDigInfo.fROImpedance*qe/spe_unit;
  
  if(fDigInfo.fGain.size()==1){
    fScale*= fDigInfo.fGain[0];
  }
  
  double mint = -fDigInfo.fGateWidth/2.0;
  double maxt = +fDigInfo.fGateWidth/2.0;
  // test values
  double tau = fDigInfo.fSPEtau;
  double sig = fDigInfo.fSPEsig;
  double t0 = fDigInfo.fSPEtransittime-fDigInfo.fTriggerOffset;
  fStartTime = t0;
  
  TF1 fFunc1(Form("fFunc1%s",detname),
	     TString::Format("TMath::Max(0.,(x/%g)*TMath::Exp(-x/(%g)))",
			     tau*tau,tau),
	     mint,maxt);
  TF1 fFunc2(Form("fFunc2%s",detname),
	     TString::Format("%g*TMath::Exp(-((x-%g)**2)/(%g))",
			     1./TMath::Sqrt(2*TMath::Pi()*sig),t0,sig*sig),
	     mint,maxt);
  TF1Convolution fConvolution(&fFunc1,&fFunc2);
  
  fModel = new TF1(Form("fSignal%s",detname),fConvolution,mint,maxt, fConvolution.GetNpar());
}

// return the total charge: comes in handy to evaluate ADC value quickly.
double TNPEModel::GetCharge(int chan)
{
  double totalcharge = fNpe*fScale;
  if(fDigInfo.fGain.size()>1){//if not, fScale already includes the gain
    if(fDigInfo.fGain.size()<=chan){
      cout << "warning: requested channel number " << chan << "larger than number of channel size " << fDigInfo.fGain.size() << " check your code ! " << endl;
      exit(-1);
    }
    totalcharge*= fDigInfo.fGain[chan];
  }
  
  return totalcharge;
}

//
double TNPEModel::Eval(int chan, double t)
{
  double totalscale = fNpe*fScale;
  if(fDigInfo.fGain.size()>1){//if not, fScale already includes the gain
    if(fDigInfo.fGain.size()<=chan){
      cout << "warning: requested channel number " << chan << "larger than number of channel size " << fDigInfo.fGain.size() << " check your code ! " << endl;
      exit(-1);
    }
    totalscale*= fDigInfo.fGain[chan];
  }
  
  return totalscale*fModel->Eval(t);
}

// 
void TNPEModel::FindLeadTrailTime(int chan, double &t_lead, double &t_trail)
{
  double thresh = fDigInfo.fThreshold[0];
  if(fDigInfo.fThreshold.size()>1){
    if(fDigInfo.fThreshold.size()<=chan){
      cout << "warning: requested channel number " << chan << "larger than number of channel size " << fDigInfo.fThreshold.size() << " check your code ! " << endl;
      exit(-1);
    }
    thresh = fDigInfo.fThreshold[chan];
  }
  double totalscale = fNpe*fScale;
  if(fDigInfo.fGain.size()>1){
    totalscale*= fDigInfo.fGain[chan];
  }
  //Since we cannot seem to scale the pulse, we'll scale the threshold
  thresh = thresh/totalscale;
  
  if(PulseOverThr()){
    t_lead = 1.0e38;
    t_trail = 1.0e38;
  }else{
    double xmax = fModel->GetMaximumX();
    t_lead = fModel->GetX(thresh, -fDigInfo.fGateWidth, xmax);
    t_trail = fModel->GetX(thresh, xmax, +fDigInfo.fGateWidth);
    
  }
}

// 
bool TNPEModel::PulseOverThr(int chan)
{
  double thresh = fDigInfo.fThreshold[0];
  if(fDigInfo.fThreshold.size()>1){
    if(fDigInfo.fThreshold.size()<=chan){
      cout << "warning: requested channel number " << chan << "larger than number of channel size " << fDigInfo.fThreshold.size() << " check your code ! " << endl;
      exit(-1);
    }
    thresh = fDigInfo.fThreshold[chan];
  }
  double totalscale = fNpe*fScale;
  if(fDigInfo.fGain.size()>1){
    totalscale*= fDigInfo.fGain[chan];
  }
  //Since we cannot seem to scale the pulse, we'll scale the threshold
  thresh = thresh/totalscale;
  
  if(fModel->GetMaximum(-fDigInfo.fGateWidth, +fDigInfo.fGateWidth)<thresh){
    return false;
  }else{
    return true;
  }
};
*/
