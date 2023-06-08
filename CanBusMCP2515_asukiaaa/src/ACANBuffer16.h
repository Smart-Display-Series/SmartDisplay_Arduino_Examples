//----------------------------------------------------------------------------------------------------------------------
// A CAN driver for MCP2515
// by Pierre Molinaro
// https://github.com/pierremolinaro/acan2515
//
// This file is common with the acan2517 library
// https://github.com/pierremolinaro/acan2517
//----------------------------------------------------------------------------------------------------------------------

#ifndef ACAN_BUFFER_16_CLASS_DEFINED
#define ACAN_BUFFER_16_CLASS_DEFINED

//----------------------------------------------------------------------------------------------------------------------

#include <CanBusData_asukiaaa.h>

//----------------------------------------------------------------------------------------------------------------------

class ACANBuffer16 {

//······················································································································
// Default constructor
//······················································································································

  public: ACANBuffer16 (void) :
  mBuffer (nullptr),
  mSize (0),
  mReadIndex (0),
  mCount (0),
  mPeakCount (0) {
  }

//······················································································································
// Destructor
//······················································································································

  public: ~ ACANBuffer16(void) { delete [] mBuffer ; }

//······················································································································
// Private properties
//······················································································································

  private: CanBusData_asukiaaa::Frame* mBuffer ;
  private: uint16_t mSize ;
  private: uint16_t mReadIndex ;
  private: volatile uint16_t mCount ;
  private: uint16_t mPeakCount ; // > mSize if overflow did occur

//······················································································································
// Accessors
//······················································································································

  public: inline uint16_t size (void) const { return mSize ; }
  public: inline uint16_t count (void) const { return mCount ; }
  public: inline uint16_t peakCount (void) const { return mPeakCount ; }

//······················································································································
// initWithSize
//······················································································································

  public: bool initWithSize (const uint16_t inSize) {
    delete [] mBuffer ;
    mBuffer = new CanBusData_asukiaaa::Frame [inSize] ;
    const bool ok = mBuffer != nullptr ;
    mSize = ok ? inSize : 0 ;
    mReadIndex = 0 ;
    mCount = 0 ;
    mPeakCount = 0 ;
    return ok ;
  }

//······················································································································
// append
//······················································································································

  public: bool append (const CanBusData_asukiaaa::Frame & inMessage) {
    const bool ok = mCount < mSize ;
    if (ok) {
      uint16_t writeIndex = mReadIndex + mCount ;
      if (writeIndex >= mSize) {
        writeIndex -= mSize ;
      }
      mBuffer[writeIndex] = inMessage ;
      mCount += 1 ;
      if (mPeakCount < mCount) {
        mPeakCount = mCount ;
      }
    }else{
      mPeakCount = mSize + 1 ;
    }
    return ok ;
  }

//······················································································································
// Remove
//······················································································································

  public: bool remove (CanBusData_asukiaaa::Frame & outMessage) {
    const bool ok = mCount > 0 ;
    if (ok) {
      outMessage = mBuffer [mReadIndex] ;
      mCount -= 1 ;
      mReadIndex += 1 ;
      if (mReadIndex == mSize) {
        mReadIndex = 0 ;
      }
    }
    return ok ;
  }

//······················································································································
// Free
//······················································································································

  public: void free (void) {
    delete [] mBuffer ;
    mBuffer = nullptr ;
    mSize = 0 ;
    mReadIndex = 0 ;
    mCount = 0 ;
    mPeakCount = 0 ;
  }

//······················································································································
// No copy
//······················································································································

  private: ACANBuffer16 (const ACANBuffer16 &) = delete ;
  private: ACANBuffer16 & operator = (const ACANBuffer16 &) = delete ;

//······················································································································

} ;

//----------------------------------------------------------------------------------------------------------------------

#endif
