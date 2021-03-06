#include "TSBSSimData.h"
#include <stdlib.h>

///////////////////////////////////////////////////////////////
// hitdata classes 

g4sbshitdata::g4sbshitdata(int unique_detid, unsigned int size ){
  fDetID = unique_detid%10;
  fDetType = (det_type)((unique_detid-fDetID)/10);//Should assign the right det flag to the right 
  // again: kHCal: 0, kECal: 1, kCher: 3, kScint: 3, kGEM: 4;
  fData  = new double[size];
  fSize  = size;
  fFillbits = 0;
  
  if( size > sizeof( long long int )*8 ){
    fprintf(stderr, "%s %s line %d:  Error:  Event size too long for bit pattern storage (requested %d, have %ld)\n",
	    __FILE__, __PRETTY_FUNCTION__, __LINE__, size, 
	    sizeof(long long int)*8);
    exit(1);
  }
  
  // There is no value indexed at 0, so we'll just set it to 0 for
  // sanity's sake and not deal with crazy offsets all over
  
  fFillbits |= 1;
  fData[0] = 3.1415927;
}

void g4sbshitdata::SetData(unsigned int idx, double data ){
  if( idx >= fSize ){
    fprintf(stderr, "%s %s line %d:  Error:  index out of range (%d oor of size %d)\n",__FILE__, __PRETTY_FUNCTION__, __LINE__, idx, fSize);
    return;

  }

  fFillbits |= (1<<idx);

  fData[idx] = data;
  return;
}

double g4sbshitdata::GetData(unsigned int idx) const {
  if( idx >= fSize ){
    fprintf(stderr, "%s %s line %d:  Error:  index out of range (%d oor of size %d)\n",__FILE__, __PRETTY_FUNCTION__, __LINE__, idx, fSize);
    return 1e9;
  }

  if( !(fFillbits & (1<<idx)) ){
    fprintf(stderr, "%s %s line %d:  Error:  Accessing unset data (idx %d) val: %f\n",__FILE__, __PRETTY_FUNCTION__, __LINE__, idx, fData[idx] );
    return 1e9;
  }

  return fData[idx];
}

bool g4sbshitdata::IsFilled() const {
  if( fFillbits == ((1<<fSize) - 1) ){
	return true;
    }

    return false;
}

g4sbshitdata::~g4sbshitdata(){
    delete fData;
}

///////////////////////////////////////////////////////////////
// gendata classes

// Size is 1 bigger because we are also including the weight
// Set that default to 1
g4sbsgendata::g4sbsgendata(int unique_detid, unsigned int size ):g4sbshitdata(unique_detid, size){
  //SetData(8,1.0);
}


// -------------------------------------- //
//          Output data classes           //
// -------------------------------------- //


//////////////////////
// MC Track hit data
mctrack_outdata::mctrack_outdata(){
  Clear();
}

mctrack_outdata::~mctrack_outdata(){
  Clear();
}

void mctrack_outdata::Clear(){
  fNMCTracks = 0;
  fMCTrackSource.clear();
  fMCTrackTRID.clear();
  fMCTrackPID.clear();
  fMCTrackX.clear();
  fMCTrackY.clear();
  fMCTrackT.clear();  
  fMCTrackP.clear();
  fMCTrackdX.clear();
  fMCTrackdY.clear();
  fMCTrackXv.clear();
  fMCTrackYv.clear();
  fMCTrackZv.clear();
  fMCTrackPXv.clear();
  fMCTrackPYv.clear();
  fMCTrackPZv.clear();
  fMCTrackWeight.clear();
}

bool mctrack_outdata::CheckSize(bool print)
{
  bool checkout = true;
  if(fMCTrackSource.size()!=fNMCTracks){
    checkout = false;
    if(print)printf("fMCTrackSource.size() = %zu != %u\n", fMCTrackSource.size(), fNMCTracks);
  }
  if(fMCTrackTRID.size()!=fNMCTracks){
    checkout = false;
    if(print)printf("fMCTrackTRID.size() = %zu != %u\n", fMCTrackTRID.size(), fNMCTracks);
  }
  if(fMCTrackPID.size()!=fNMCTracks){
    checkout = false;
    if(print)printf("fMCTrackPID.size() = %zu != %u\n", fMCTrackPID.size(), fNMCTracks);
  }
  if(fMCTrackX.size()!=fNMCTracks){
    checkout = false;
    if(print)printf("fMCTrackX.size() = %zu != %u\n", fMCTrackX.size(), fNMCTracks);
  }
  if(fMCTrackY.size()!=fNMCTracks){
    checkout = false;
    if(print)printf("fMCTrackY.size() = %zu != %u\n", fMCTrackY.size(), fNMCTracks);
  }
  if(fMCTrackT.size()!=fNMCTracks){
    checkout = false;
    if(print)printf("fMCTrackT.size() = %zu != %u\n", fMCTrackT.size(), fNMCTracks);
  }
  if(fMCTrackP.size()!=fNMCTracks){
    checkout = false;
    if(print)printf("fMCTrackP.size() = %zu != %u\n", fMCTrackP.size(), fNMCTracks);
  }
  if(fMCTrackdX.size()!=fNMCTracks){
    checkout = false;
    if(print)printf("fMCTrackX.size() = %zu != %u\n", fMCTrackdX.size(), fNMCTracks);
  }
  if(fMCTrackdY.size()!=fNMCTracks){
    checkout = false;
    if(print)printf("fMCTrackY.size() = %zu != %u\n", fMCTrackdY.size(), fNMCTracks);
  }
  if(fMCTrackXv.size()!=fNMCTracks){
    checkout = false;
    if(print)printf("fMCTrackXv.size() = %zu != %u\n", fMCTrackXv.size(), fNMCTracks);
  }
  if(fMCTrackYv.size()!=fNMCTracks){
    checkout = false;
    if(print)printf("fMCTrackYv.size() = %zu != %u\n", fMCTrackYv.size(), fNMCTracks);
  }
  if(fMCTrackZv.size()!=fNMCTracks){
    checkout = false;
    if(print)printf("fMCTrackZv.size() = %zu != %u\n", fMCTrackZv.size(), fNMCTracks);
  }
  if(fMCTrackPXv.size()!=fNMCTracks){
    checkout = false;
    if(print)printf("fMCTrackXv.size() = %zu != %u\n", fMCTrackPXv.size(), fNMCTracks);
  }
  if(fMCTrackPYv.size()!=fNMCTracks){
    checkout = false;
    if(print)printf("fMCTrackYv.size() = %zu != %u\n", fMCTrackPYv.size(), fNMCTracks);
  }
  if(fMCTrackPZv.size()!=fNMCTracks){
    checkout = false;
    if(print)printf("fMCTrackZv.size() = %zu != %u\n", fMCTrackPZv.size(), fNMCTracks);
  }
  if(fMCTrackWeight.size()!=fNMCTracks){
    checkout = false;
    if(print)printf("fMCTrackWeight.size() = %zu != %u\n", fMCTrackWeight.size(), fNMCTracks);
  }
  
  return checkout;
}

//////////////////////
// Track MC hit data
trackmchit_outdata::trackmchit_outdata(){
  Clear();
}

trackmchit_outdata::~trackmchit_outdata(){
  Clear();
}

void trackmchit_outdata::Clear(){
  fNTrackMCHits = 0;
  fTrackMCSource.clear();
  fTrackMCTRID.clear();
  fTrackMCPID.clear();
  fTrackMCXhit.clear();
  fTrackMCYhit.clear();
  fTrackMCThit.clear();
  fTrackMCE.clear();
  fTrackMCWeight.clear();
}

bool trackmchit_outdata::CheckSize(bool print)
{
  bool checkout = true;
  if(fTrackMCSource.size()!=fNTrackMCHits){
    checkout = false;
    if(print)printf("fTrackMCSource.size() = %zu != %u\n", fTrackMCSource.size(), fNTrackMCHits);
  }
  if(fTrackMCTRID.size()!=fNTrackMCHits){
    checkout = false;
    if(print)printf("fTrackMCTRID.size() = %zu != %u\n", fTrackMCTRID.size(), fNTrackMCHits);
  }
  if(fTrackMCPID.size()!=fNTrackMCHits){
    checkout = false;
    if(print)printf("fTrackMCPID.size() = %zu != %u\n", fTrackMCPID.size(), fNTrackMCHits);
  }
  if(fTrackMCXhit.size()!=fNTrackMCHits){
    checkout = false;
    if(print)printf("fTrackMCXhit.size() = %zu != %u\n", fTrackMCXhit.size(), fNTrackMCHits);
  }
  if(fTrackMCYhit.size()!=fNTrackMCHits){
    checkout = false;
    if(print)printf("fTrackMCYhit.size() = %zu != %u\n", fTrackMCYhit.size(), fNTrackMCHits);
  }
  if(fTrackMCThit.size()!=fNTrackMCHits){
    checkout = false;
    if(print)printf("fTrackMCThit.size() = %zu != %u\n", fTrackMCThit.size(), fNTrackMCHits);
  }
  if(fTrackMCE.size()!=fNTrackMCHits){
    checkout = false;
    if(print)printf("fTrackMCThit.size() = %zu != %u\n", fTrackMCThit.size(), fNTrackMCHits);
  }
  if(fTrackMCWeight.size()!=fNTrackMCHits){
    checkout = false;
    if(print)printf("fTrackMCWeight.size() = %zu != %u\n", fTrackMCWeight.size(), fNTrackMCHits);
  }
  return checkout;
}

////////////////
// MC hit data
simhitmc_outdata::simhitmc_outdata(){
  Clear();
}

simhitmc_outdata::~simhitmc_outdata(){
  Clear();
}

void simhitmc_outdata::Clear(){
  fNSimHits = 0;
  fSimSource.clear();
  fSimTRID.clear();
  fSimPID.clear();
  fSimChannel.clear();
  fSimEdep.clear();
  fSimNpe.clear();
  fSimTime.clear();
  fSimLeadTime.clear();
  fSimTrailTime.clear();
}

bool simhitmc_outdata::CheckSize(bool check_edep,
				 bool check_npe,
				 bool check_times,
				 bool print)
{
  bool checkout = true;
  if(fSimSource.size()!=fNSimHits){
    checkout = false;
    if(print)printf("fSimSource.size() = %zu != %u\n", fSimSource.size(), fNSimHits);
  }
  if(fSimTRID.size()!=fNSimHits){
    checkout = false;
    if(print)printf("fSimTRID.size() = %zu != %u\n", fSimTRID.size(), fNSimHits);
  }
  if(fSimPID.size()!=fNSimHits){
    checkout = false;
    if(print)printf("fSimPID.size() = %zu != %u\n", fSimPID.size(), fNSimHits);
  }
  if(fSimChannel.size()!=fNSimHits){
    checkout = false;
    if(print)printf("fSimChannel.size() = %zu != %u\n", fSimChannel.size(), fNSimHits);
  }
  if(fSimEdep.size()!=fNSimHits && check_edep){
    checkout = false;
    if(print)printf("fSimEdep.size() = %zu != %u\n", fSimEdep.size(), fNSimHits);
  }
  if(fSimNpe.size()!=fNSimHits && check_npe){
    checkout = false;
    if(print)printf("fSimNpe.size() = %zu != %u\n", fSimNpe.size(), fNSimHits);
  }
  if(fSimTime.size()!=fNSimHits){
    checkout = false;
    if(print)printf("fSimTime.size() = %zu != %u\n", fSimTime.size(), fNSimHits);
  }
  if(fSimLeadTime.size()!=fNSimHits && check_times){
    checkout = false;
    if(print)printf("fSimLeadTime.size() = %zu != %u\n", fSimLeadTime.size(), fNSimHits);
  }
  if(fSimTrailTime.size()!=fNSimHits && check_times){
    checkout = false;
    if(print)printf("fSimTrailTime.size() = %zu != %u\n", fSimTrailTime.size(), fNSimHits);
  }
  return checkout;
}

simgemhitmc_outdata::simgemhitmc_outdata(){
  Clear();
}

simgemhitmc_outdata::~simgemhitmc_outdata(){
  Clear();
}

void simgemhitmc_outdata::Clear(){
  simhitmc_outdata::Clear();
  fPlane.clear();
  fModule.clear();
  fSizeX.clear();
  fSizeY.clear();
  fStartX.clear();
  fStartY.clear();
  fXpos.clear();
  fYpos.clear();
  fPX.clear();
  fPY.clear();
  fPZ.clear();
}

bool simgemhitmc_outdata::CheckSize(bool print)
{
  bool checkout = simhitmc_outdata::CheckSize(true, false, false, print);

  if(fPlane.size()!=fNSimHits){
    checkout = false;
    if(print)printf("fPlane.size() = %zu != %u\n", fPlane.size(), fNSimHits);
  }
  if(fModule.size()!=fNSimHits){
    checkout = false;
    if(print)printf("fModule.size() = %zu != %u\n", fModule.size(), fNSimHits);
  }
  if(fSizeX.size()!=fNSimHits){
    checkout = false;
    if(print)printf("fSizeX.size() = %zu != %u\n", fSizeX.size(), fNSimHits);
  }
  if(fSizeY.size()!=fNSimHits){
    checkout = false;
    if(print)printf("fSizeY.size() = %zu != %u\n", fSizeY.size(), fNSimHits);
  }
  if(fStartX.size()!=fNSimHits){
    checkout = false;
    if(print)printf("fStartX.size() = %zu != %u\n", fStartX.size(), fNSimHits);
  }
  if(fStartY.size()!=fNSimHits){
    checkout = false;
    if(print)printf("fStartY.size() = %zu != %u\n", fStartY.size(), fNSimHits);
  }
  if(fXpos.size()!=fNSimHits){
    checkout = false;
    if(print)printf("fXpos.size() = %zu != %u\n", fXpos.size(), fNSimHits);
  }
  if(fYpos.size()!=fNSimHits){
    checkout = false;
    if(print)printf("fYpos.size() = %zu != %u\n", fYpos.size(), fNSimHits);
  }
  if(fPX.size()!=fNSimHits){
    checkout = false;
    if(print)printf("fPX.size() = %zu != %u\n", fPX.size(), fNSimHits);
  }
  if(fPY.size()!=fNSimHits){
    checkout = false;
    if(print)printf("fPY.size() = %zu != %u\n", fPY.size(), fNSimHits);
  }
  if(fPZ.size()!=fNSimHits){
    checkout = false;
    if(print)printf("fPZ.size() = %zu != %u\n", fPZ.size(), fNSimHits);
  }
  
  
  return checkout;
}




///////////////////
// digitized data

// Base class: simdig_outdata
simdig_outdata::simdig_outdata(){
  Clear();
}

simdig_outdata::~simdig_outdata(){
  Clear();
}

void simdig_outdata::Clear(){
  fNHits = 0;
  fChannel.clear();
  fDataWord.clear();
  fADC.clear();
  fTDC_L.clear();
  fTDC_T.clear();
}

bool simdig_outdata::CheckSize(bool check_adc,
			       bool check_tdc,
			       bool print){
  bool checkout = true;
  if(fChannel.size()!=fNHits){
    checkout = false;
    if(print)printf("fChannel.size() = %zu != %u\n", fChannel.size(), fNHits);
  }
  if(fDataWord.size()!=fNHits){
    checkout = false;
    if(print)printf("fDataWord.size() = %zu != %u\n", fDataWord.size(), fNHits);
  }
  if(check_adc && fADC.size()!=fNHits){
    checkout = false;
    if(print)printf("fADC.size() = %zu != %u\n", fADC.size(), fNHits);
  }
  if(check_tdc && fTDC_L.size()!=fNHits){
    checkout = false;
    if(print)printf("fTDC_L.size() = %zu != %u\n", fTDC_L.size(), fNHits);
  }
  if(check_tdc && fTDC_T.size()!=fNHits){
    checkout = false;
    if(print)printf("fTDC_T.size() = %zu != %u\n", fTDC_T.size(), fNHits);
  }
  
  return checkout;
}

// class: simdigsamp_outdata -> for DAQ with sampled ADC
simdigsamp_outdata::simdigsamp_outdata(){
  Clear();
}

simdigsamp_outdata::~simdigsamp_outdata(){
  Clear();
}

void simdigsamp_outdata::Clear(){
  simdig_outdata::Clear();
  fNsamps.clear();
  fDataWord_samps.clear();
  fADC_samps.clear();
}

bool simdigsamp_outdata::CheckSize(bool check_tdc,
				   bool print){
  bool checkout = simdig_outdata::CheckSize(true, check_tdc, print);//total ADC will not be filled
  if(fNsamps.size()!=fNHits){
    checkout = false;
    if(print)printf("fNsamps.size() = %zu != %u\n", fNsamps.size(), fNHits);
  }
  if(fDataWord_samps.size()!=fNHits){
    checkout = false;
    if(print)printf("fDataWord_samps.size() = %zu != %u\n", fDataWord_samps.size(), fNHits);
  }
  if(fADC_samps.size()!=fNHits){
    checkout = false;
    if(print)printf("fADC_samps.size() = %zu != %u\n", fADC_samps.size(), fNHits);
  }
  
  return checkout;
}
