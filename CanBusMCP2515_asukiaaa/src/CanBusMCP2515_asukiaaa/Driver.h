#pragma once

#include <Arduino.h>
#include <CanBusData_asukiaaa.h>
#include <SPI.h>

#include "ACANBuffer16.h"
#include "Frame.h"
#include "MCP2515ReceiveFilters.h"
#include "Settings.h"

// #define SERIAL_DEBUG Serial

namespace CanBusMCP2515_asukiaaa {

typedef void (*ACANCallBackRoutine)(
    const CanBusData_asukiaaa::Frame &inMessage);

class QuartzFrequency {
 public:
  static const uint32_t MHz8 = 8UL * 1000UL * 1000UL;
  static const uint32_t MHz16 = 16UL * 1000UL * 1000UL;
};

class BitRate {
 public:
  static const uint32_t Mbps1 = 10UL ^ 6;
  static const uint32_t Kbps500 = 500UL * 1000UL;
  static const uint32_t Kbps200 = 200UL * 1000UL;
  static const uint32_t Kbps125 = 125UL * 1000UL;
  static const uint32_t Kbps100 = 100UL * 1000UL;
  static const uint32_t Kbps50 = 50UL * 1000UL;
  static const uint32_t Kbps40 = 40UL * 1000UL;
  static const uint32_t Kbps20 = 20UL * 1000UL;
  static const uint32_t Kbps10 = 10UL * 1000UL;
  static const uint32_t Kbps5 = 5UL * 1000UL;
};

class Error {
 public:
  static const uint16_t NoMCP2515 = 1 << 0;
  static const uint16_t TooFarFromDesiredBitRate = 1 << 1;
  static const uint16_t InconsistentBitRateSettings = 1 << 2;
  static const uint16_t INTPinIsNotAnInterrupt = 1 << 3;
  static const uint16_t ISRIsNull = 1 << 4;
  static const uint16_t RequestedModeTimeOut = 1 << 5;
  static const uint16_t AcceptanceFilterArrayIsNULL = 1 << 6;
  static const uint16_t OneFilterMaskRequiresOneOrTwoAcceptanceFilters = 1 << 7;
  static const uint16_t TwoFilterMasksRequireThreeToSixAcceptanceFilters = 1
                                                                           << 8;
  static const uint16_t CannotAllocateReceiveBuffer = 1 << 9;
  static const uint16_t CannotAllocateTransmitBuffer0 = 1 << 10;
  static const uint16_t CannotAllocateTransmitBuffer1 = 1 << 11;
  static const uint16_t CannotAllocateTransmitBuffer2 = 1 << 12;

  static String toString(uint16_t errorCode);
};

class Driver {
 public:
  Driver(const uint8_t inCS, const int inINT = -1);
  uint16_t begin(const Settings &inSettings,
                 void (*inInterruptServiceRoutine)(void) = NULL);
  uint16_t begin(const Settings &inSettings,
                 void (*inInterruptServiceRoutine)(void),
                 const ACAN2515Mask inRXM0,
                 const ACAN2515AcceptanceFilter inAcceptanceFilters[],
                 const uint8_t inAcceptanceFilterCount);
  uint16_t begin(const Settings &inSettings,
                 void (*inInterruptServiceRoutine)(void),
                 const ACAN2515Mask inRXM0, const ACAN2515Mask inRXM1,
                 const ACAN2515AcceptanceFilter inAcceptanceFilters[],
                 const uint8_t inAcceptanceFilterCount);
  uint16_t changeModeOnTheFly(const OperationMode operationMode);
  uint16_t setFiltersOnTheFly(void);
  uint16_t setFiltersOnTheFly(
      const ACAN2515Mask inRXM0,
      const ACAN2515AcceptanceFilter inAcceptanceFilters[],
      const uint8_t inAcceptanceFilterCount);
  uint16_t setFiltersOnTheFly(
      const ACAN2515Mask inRXM0, const ACAN2515Mask inRXM1,
      const ACAN2515AcceptanceFilter inAcceptanceFilters[],
      const uint8_t inAcceptanceFilterCount);
  void end(void);
  bool available(void);
  bool receive(Frame *outFrame);
  bool receive(CanBusData_asukiaaa::Frame *outFrame);
  typedef void (*tFilterMatchCallBack)(const uint8_t inFilterIndex);
  bool dispatchReceivedMessage(
      const tFilterMatchCallBack inFilterMatchCallBack = NULL);
  void isr(void);
  bool isr_core(void);

  inline uint16_t receiveBufferSize(void) const {
    return mReceiveBuffer.size();
  }

  inline uint16_t receiveBufferCount(void) const {
    return mReceiveBuffer.count();
  }

  inline uint16_t receiveBufferPeakCount(void) const {
    return mReceiveBuffer.peakCount();
  }

  bool tryToSend(const Frame &inMessage);
  bool tryToSend(const CanBusData_asukiaaa::Frame &inMessage);

  inline uint16_t transmitBufferSize(const uint8_t inIndex) const {
    return mTransmitBuffer[inIndex].size();
  }
  inline uint16_t transmitBufferCount(const uint8_t inIndex) const {
    return mTransmitBuffer[inIndex].count();
  }
  inline uint16_t transmitBufferPeakCount(const uint8_t inIndex) const {
    return mTransmitBuffer[inIndex].peakCount();
  }

  void poll(void);

  uint8_t receiveErrorCounter(void);
  uint8_t transmitErrorCounter(void);
  uint8_t errorFlagRegister(void);

 private:
  const SPISettings mSPISettings;
  const uint8_t mCS;
  const int mINT;
  ACANBuffer16 mReceiveBuffer;
  ACANCallBackRoutine mCallBackFunctionArray[6];
  ACANBuffer16 mTransmitBuffer[3];
  bool mTXBIsFree[3];
  void handleTXBInterrupt(const uint8_t inTXB);
  void handleRXBInterrupt(void);
  SPIClass *mSpi = NULL;
  bool isAttachedInterrupt = false;

  void internalSendMessage(const CanBusData_asukiaaa::Frame &inFrame,
                           const uint8_t inTXB);
  inline void select(void) { digitalWrite(mCS, LOW); }
  inline void unselect(void) { digitalWrite(mCS, HIGH); }
  uint16_t beginWithoutFilterCheck(
      const Settings &inSettings, void (*inInterruptServiceRoutine)(void),
      const ACAN2515Mask inRXM0, const ACAN2515Mask inRXM1,
      const ACAN2515AcceptanceFilter inAcceptanceFilters[],
      const uint8_t inAcceptanceFilterCount);
  uint16_t internalBeginOperation(
      const Settings &inSettings, const ACAN2515Mask inRXM0,
      const ACAN2515Mask inRXM1,
      const ACAN2515AcceptanceFilter inAcceptanceFilters[],
      const uint8_t inAcceptanceFilterCount);
  void write2515Register(const uint8_t inRegister, const uint8_t inValue);
  uint8_t read2515Register(const uint8_t inRegister);
  uint8_t read2515Status(void);
  uint8_t read2515RxStatus(void);
  void bitModify2515Register(const uint8_t inRegister, const uint8_t inMask,
                             const uint8_t inData);
  void setupMaskRegister(const ACAN2515Mask inMask, const uint8_t inRegister);
  uint16_t setRequestedMode(const uint8_t inCANControlRegister);
  uint16_t internalSetFiltersOnTheFly(
      const ACAN2515Mask inRXM0, const ACAN2515Mask inRXM1,
      const ACAN2515AcceptanceFilter inAcceptanceFilters[],
      const uint8_t inAcceptanceFilterCount);
  Driver(const Driver &) = delete;
  Driver &operator=(const Driver &) = delete;
  void sendReset();
  bool neededToHandleInterruptTask();
};

};  // namespace CanBusMCP2515_asukiaaa
