//----------------------------------------------------------------------------------------------------------------------
// A CAN driver for MCP2515
// by Pierre Molinaro
// https://github.com/pierremolinaro/acan2515
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include <Arduino.h>

namespace CanBusMCP2515_asukiaaa {

enum class OperationMode : uint8_t {
  Normal = 0 << 5,
  Sleep = 1 << 5,
  LoopBack = 2 << 5,
  ListenOnly = 3 << 5,
};

class Settings {
 public:
  typedef enum : uint8_t { CLOCK, CLOCK2, CLOCK4, CLOCK8, SOF, HiZ } CLKOUT_SOF;
  explicit Settings(const uint32_t inQuartzFrequency,  // In Hertz
                    const uint32_t inDesiredBitRate,
                    const uint32_t inTolerancePPM = 1000);
  explicit Settings(const uint32_t inQuartzFrequency,    // In Hertz
                    const uint8_t inBitRatePrescaler,    // 1...64
                    const uint8_t inPropagationSegment,  // 1...8
                    const uint8_t inPhaseSegment1,       // 1...8
                    const uint8_t inPhaseSegment2,       // 2...8
                    const uint8_t inSJW);                // 1...4
  String toString();
  const uint32_t mQuartzFrequency;

  uint32_t mDesiredBitRate = mQuartzFrequency / 64;  // In kb/s
  uint8_t mPropagationSegment = 5;                   // 1...8
  uint8_t mPhaseSegment1 = 5;                        // 1...8
  uint8_t mPhaseSegment2 = 5;                        // 2...8
  uint8_t mSJW = 4;                                  // 1...4
  uint8_t mBitRatePrescaler = 32 / (1 + mPropagationSegment + mPhaseSegment1 +
                                    mPhaseSegment2);  // 1...64
  bool mTripleSampling =
      false;  // true --> triple sampling, false --> single sampling
  bool mBitRateClosedToDesiredRate =
      true;  // The above configuration is correct

  //······················································································································
  //   One shot mode
  //      true --> Enabled; messages will only attempt to transmit one time
  //      false --> Disabled; messages will reattempt transmission if required
  //······················································································································

  bool mOneShotModeEnabled = false;

  //······················································································································
  //    MCP2515 TXBi priorities
  //       bits 7-6: unused
  //       bits 5-4: TXB2 priority
  //       bits 3-2: TXB1 priority
  //       bits 1-0: TXB0 priority
  //······················································································································

  uint8_t mTXBPriority = 0;

  OperationMode mOperationMode = OperationMode::Normal;
  CLKOUT_SOF mCLKOUT_SOF_pin = CLOCK;

  //······················································································································
  //    Rollover Enable Bit (is set to the BUKT bit of the RXB0CTRL register)
  //       true  --> RXB0 message will roll over and be written to RXB1 if RXB0
  //       is full false --> Rollover is disabled
  //······················································································································

  bool mRolloverEnable = true;

  uint16_t mReceiveBufferSize = 32;
  uint16_t mTransmitBuffer0Size = 16;
  uint16_t mTransmitBuffer1Size = 0;
  uint16_t mTransmitBuffer2Size = 0;
  uint32_t actualBitRate(void) const;
  bool exactBitRate(void) const;
  uint32_t ppmFromDesiredBitRate(void) const;
  uint32_t samplePointFromBitStart(void) const;
  uint16_t CANBitSettingConsistency(void) const;
  static const uint16_t kBitRatePrescalerIsZero = 1 << 0;
  static const uint16_t kBitRatePrescalerIsGreaterThan64 = 1 << 1;
  static const uint16_t kPropagationSegmentIsZero = 1 << 2;
  static const uint16_t kPropagationSegmentIsGreaterThan8 = 1 << 3;
  static const uint16_t kPhaseSegment1IsZero = 1 << 4;
  static const uint16_t kPhaseSegment1IsGreaterThan8 = 1 << 5;
  static const uint16_t kPhaseSegment2IsLowerThan2 = 1 << 6;
  static const uint16_t kPhaseSegment2IsGreaterThan8 = 1 << 7;
  static const uint16_t kPhaseSegment1Is1AndTripleSampling = 1 << 8;
  static const uint16_t kSJWIsZero = 1 << 9;
  static const uint16_t kSJWIsGreaterThan4 = 1 << 10;
  static const uint16_t kSJWIsGreaterThanOrEqualToPhaseSegment2 = 1 << 11;
  static const uint16_t kPhaseSegment2IsGreaterThanPSPlusPS1 = 1 << 12;
};

}  // namespace CanBusMCP2515_asukiaaa
