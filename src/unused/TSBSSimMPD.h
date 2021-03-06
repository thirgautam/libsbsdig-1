#ifndef TSBSSimMPD_
#define TSBSSimMPD_

/////////////////////////////////////////////////////////////////////
//
//   TSBSSimMPD
//   G4SBS Simplified Simulated MPD Module
//
/////////////////////////////////////////////////////////////////////

#include "PipeliningModule.h"
#include "stdint.h"
#include <vector>
#include "TSBSSimDataEncoder.h"

namespace Decoder {

  class TSBSSimMPD : public PipeliningModule {   // Inheritance

  public:

    TSBSSimMPD();                         // Default constructor
    TSBSSimMPD(Int_t crate, Int_t slot);  // Constructor
    virtual ~TSBSSimMPD();                // Virtual constructor

    // Use parent class functions
    using Module::GetData;
    using Module::LoadSlot;

    virtual void Clear(const Option_t *opt="");
    virtual void Init();
    virtual void CheckDecoderStatus() const;
    virtual void ClearDataVectors();
    virtual Int_t LoadSlot(THaSlotData *sldat, const UInt_t* evbuffer, const UInt_t *pstop);
    virtual Int_t LoadSlot(THaSlotData *sldat, const UInt_t* evbuffer, Int_t pos, Int_t len);

    // We don't need these functions for simulated data, but must be defined
    // so that this won't be an abstract class
    Int_t LoadNextEvBuffer(THaSlotData*) {return 0;};// needs return something for compilation
    Int_t LoadThisBlock(THaSlotData*, std::vector<UInt_t >) {return 0;};// needs return something for compilation
    Int_t Decode(const UInt_t *) { return 0; }; // use DecodeOneWord instead

  private:
    //static const size_t NADCCHAN = 64; // Max ADC channels
    static TypeIter_t fgThisType;
    //std::vector<SimEncoder::fadc_data> fadc_data;

    ClassDef(TSBSSimMPD,1)  //  Generic SimMPD module

  };  // TSBSSimMPD class

}  // Decoder namespace

#endif
