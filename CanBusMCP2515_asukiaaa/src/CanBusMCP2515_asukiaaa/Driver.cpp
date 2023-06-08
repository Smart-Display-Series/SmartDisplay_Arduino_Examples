//··································································································
// A CAN driver for MCP2515
// Original is made by Pierre Molinaro
// https://github.com/pierremolinaro/acan2515
//··································································································

#include "CanBusMCP2515_asukiaaa.h"

namespace CanBusMCP2515_asukiaaa {

static const uint8_t RESET_COMMAND = 0xC0;
static const uint8_t WRITE_COMMAND = 0x02;
static const uint8_t READ_COMMAND = 0x03;
static const uint8_t BIT_MODIFY_COMMAND = 0x05;
static const uint8_t LOAD_TX_BUFFER_COMMAND = 0x40;
static const uint8_t REQUEST_TO_SEND_COMMAND = 0x80;
static const uint8_t READ_FROM_RXB0SIDH_COMMAND = 0x90;
static const uint8_t READ_FROM_RXB1SIDH_COMMAND = 0x94;
static const uint8_t READ_STATUS_COMMAND = 0xA0;
static const uint8_t RX_STATUS_COMMAND = 0xB0;

//··································································································
//   MCP2515 REGISTERS
//··································································································

static const uint8_t BFPCTRL_REGISTER = 0x0C;
static const uint8_t TXRTSCTRL_REGISTER = 0x0D;
static const uint8_t CANSTAT_REGISTER = 0x0E;
static const uint8_t CANCTRL_REGISTER = 0x0F;
static const uint8_t TEC_REGISTER = 0x1C;
static const uint8_t REC_REGISTER = 0x1D;
static const uint8_t RXM0SIDH_REGISTER = 0x20;
static const uint8_t RXM1SIDH_REGISTER = 0x24;
static const uint8_t CNF3_REGISTER = 0x28;
static const uint8_t CNF2_REGISTER = 0x29;
static const uint8_t CNF1_REGISTER = 0x2A;
static const uint8_t CANINTF_REGISTER = 0x2C;
static const uint8_t EFLG_REGISTER = 0x2D;
static const uint8_t TXB0CTRL_REGISTER = 0x30;
static const uint8_t TXB1CTRL_REGISTER = 0x40;
static const uint8_t TXB2CTRL_REGISTER = 0x50;
static const uint8_t RXB0CTRL_REGISTER = 0x60;
static const uint8_t RXB1CTRL_REGISTER = 0x70;

static const uint8_t RXFSIDH_REGISTER[6] = {0x00, 0x04, 0x08, 0x10, 0x14, 0x18};

//··································································································
// Note about ESP32
//··································································································
//
// It appears that Arduino ESP32 interrupts are managed in a completely
// different way from "usual" Arduino:
//   - SPI.usingInterrupt is not implemented;
//   - noInterrupts() and interrupts() are NOPs;
//   - interrupt service routines should be fast, otherwise you get an "Guru
//   Meditation Error: Core 1 panic'ed
//     (Interrupt wdt timeout on CPU1)".

// So we handle the ESP32 interrupt in the following way:
//   - interrupt service routine performs a xSemaphoreGive on mISRSemaphore of
//   can driver
//   - this activates the myESP32Task task that performs "isr_core" that is done
//   by interrupt service routine
//     in "usual" Arduino;
//   - as this task runs in parallel with setup / loop routines, SPI access is
//   natively protected by the
//     beginTransaction / endTransaction pair, that manages a mutex.

//··································································································

String Error::toString(uint16_t errorCode) {
  return "0x" + String(errorCode, HEX);
  // String str = "";
  // String strJoin = ", ";
  // if ((errorCode & Error::NoMCP2515) != 0) {
  //   str += "NoMCP2515" + strJoin;
  // }
  // if ((errorCode & Error::TooFarFromDesiredBitRate) != 0) {
  //   str += "TooFarFromDesiredBitRate" + strJoin;
  // }
  // if ((errorCode & Error::InconsistentBitRateSettings) != 0) {
  //   str += "InconsistentBitRateSettings" + strJoin;
  // }
  // if ((errorCode & Error::INTPinIsNotAnInterrupt) != 0) {
  //   str += "INTPinIsNotAnInterrupt" + strJoin;
  // }
  // if ((errorCode & Error::ISRIsNull) != 0) {
  //   str += "ISRIsNull" + strJoin;
  // }
  // if ((errorCode & Error::RequestedModeTimeOut) != 0) {
  //   str += "RequestedModeTimeOut" + strJoin;
  // }
  // if ((errorCode & Error::AcceptanceFilterArrayIsNULL) != 0) {
  //   str += "AcceptanceFilterArrayIsNULL" + strJoin;
  // }
  // if ((errorCode & Error::OneFilterMaskRequiresOneOrTwoAcceptanceFilters) !=
  // 0) {
  //   str += "OneFilterMaskRequiresOneOrTwoAcceptanceFilters" + strJoin;
  // }
  // if ((errorCode & Error::TwoFilterMasksRequireThreeToSixAcceptanceFilters)
  // != 0) {
  //   str += "TwoFilterMasksRequireThreeToSixAcceptanceFilters" + strJoin;
  // }
  // if ((errorCode & Error::CannotAllocateReceiveBuffer) != 0) {
  //   str += "CannotAllocateReceiveBuffer" + strJoin;
  // }
  // if ((errorCode & Error::CannotAllocateTransmitBuffer0) != 0) {
  //   str += "CannotAllocateTransmitBuffer0" + strJoin;
  // }
  // if ((errorCode & Error::CannotAllocateTransmitBuffer1) != 0) {
  //   str += "CannotAllocateTransmitBuffer1" + strJoin;
  // }
  // if ((errorCode & Error::CannotAllocateTransmitBuffer2) != 0) {
  //   str += "CannotAllocateTransmitBuffer2" + strJoin;
  // }
  // int lastJoinIndex = str.lastIndexOf(strJoin);
  // if (lastJoinIndex > 0) {
  //   str = str.substring(0, lastJoinIndex);
  // }
  // return str;
}

Driver::Driver(const uint8_t inCS, const int inINT)
    : mSPISettings(10UL * 1000UL * 1000UL, MSBFIRST,
                   SPI_MODE0),  // 10 MHz, UL suffix is required for Arduino Uno
      mCS(inCS),
      mINT(inINT),
      mReceiveBuffer(),
      mCallBackFunctionArray(),
      mTXBIsFree() {
  for (uint8_t i = 0; i < 6; i++) {
    mCallBackFunctionArray[i] = NULL;
  }
}

uint16_t Driver::begin(const Settings& inSettings,
                       void (*inInterruptServiceRoutine)(void)) {
  return beginWithoutFilterCheck(inSettings, inInterruptServiceRoutine,
                                 ACAN2515Mask(), ACAN2515Mask(), NULL, 0);
}

uint16_t Driver::begin(const Settings& inSettings,
                       void (*inInterruptServiceRoutine)(void),
                       const ACAN2515Mask inRXM0,
                       const ACAN2515AcceptanceFilter inAcceptanceFilters[],
                       const uint8_t inAcceptanceFilterCount) {
  uint16_t errorCode = 0;
  if (inAcceptanceFilterCount == 0) {
    errorCode = Error::OneFilterMaskRequiresOneOrTwoAcceptanceFilters;
  } else if (inAcceptanceFilterCount > 2) {
    errorCode = Error::OneFilterMaskRequiresOneOrTwoAcceptanceFilters;
  } else if (inAcceptanceFilters == NULL) {
    errorCode = Error::AcceptanceFilterArrayIsNULL;
  } else {
    errorCode = beginWithoutFilterCheck(inSettings, inInterruptServiceRoutine,
                                        inRXM0, inRXM0, inAcceptanceFilters,
                                        inAcceptanceFilterCount);
  }
  return errorCode;
}

uint16_t Driver::begin(const Settings& inSettings,
                       void (*inInterruptServiceRoutine)(void),
                       const ACAN2515Mask inRXM0, const ACAN2515Mask inRXM1,
                       const ACAN2515AcceptanceFilter inAcceptanceFilters[],
                       const uint8_t inAcceptanceFilterCount) {
  uint16_t errorCode = 0;
  if (inAcceptanceFilterCount < 3) {
    errorCode = Error::TwoFilterMasksRequireThreeToSixAcceptanceFilters;
  } else if (inAcceptanceFilterCount > 6) {
    errorCode = Error::TwoFilterMasksRequireThreeToSixAcceptanceFilters;
  } else if (inAcceptanceFilters == NULL) {
    errorCode = Error::AcceptanceFilterArrayIsNULL;
  } else {
    errorCode = beginWithoutFilterCheck(inSettings, inInterruptServiceRoutine,
                                        inRXM0, inRXM1, inAcceptanceFilters,
                                        inAcceptanceFilterCount);
  }
  return errorCode;
}

uint16_t Driver::beginWithoutFilterCheck(
    const Settings& inSettings, void (*inInterruptServiceRoutine)(void),
    const ACAN2515Mask inRXM0, const ACAN2515Mask inRXM1,
    const ACAN2515AcceptanceFilter inAcceptanceFilters[],
    const uint8_t inAcceptanceFilterCount) {
  if (mSpi == NULL) {
    mSpi = &SPI;
    mSpi->begin();
  }
  uint16_t errorCode = 0;  // Means no error
  // Check mINT has interrupt capability
  const int itPin = digitalPinToInterrupt(mINT);
  if (inInterruptServiceRoutine != NULL && itPin == NOT_AN_INTERRUPT) {
    errorCode = Error::INTPinIsNotAnInterrupt;
  }
  // if no error, configure port and MCP2515
  if (errorCode == 0) {
    //--- Configure ports
    if (mINT >= 0) {
      pinMode(mINT, INPUT_PULLUP);
    }
    pinMode(mCS, OUTPUT);
    digitalWrite(mCS, HIGH);
    sendReset();
    errorCode =
        internalBeginOperation(inSettings, inRXM0, inRXM1, inAcceptanceFilters,
                               inAcceptanceFilterCount);
  }

  if (errorCode == 0 && itPin >= 0 && inInterruptServiceRoutine != NULL) {
#ifndef ARDUINO_ARCH_ESP32
    // usingInterrupt is not implemented in Arduino ESP32
    mSpi->usingInterrupt(itPin);
#endif
    attachInterrupt(itPin, inInterruptServiceRoutine, FALLING);
    isAttachedInterrupt = true;
  }
  return errorCode;
}

bool Driver::available(void) {
  if (neededToHandleInterruptTask()) {
    isr_core();
  }
  const bool hasReceivedMessage = mReceiveBuffer.count() > 0;
  return hasReceivedMessage;
}

bool Driver::receive(Frame* outMessage) {
  if (neededToHandleInterruptTask()) {
    isr_core();
  }
  if (isAttachedInterrupt) noInterrupts();
  const bool hasReceivedMessage = mReceiveBuffer.remove(*outMessage);
  if (isAttachedInterrupt) interrupts();
  return hasReceivedMessage;
}

bool Driver::receive(CanBusData_asukiaaa::Frame* outMessage) {
  Frame frame;
  bool result = receive(&frame);
  *outMessage = frame;
  return result;
}

bool Driver::dispatchReceivedMessage(
    const tFilterMatchCallBack inFilterMatchCallBack) {
  Frame receivedMessage;
  const bool hasReceived = receive(&receivedMessage);
  if (hasReceived) {
    const uint8_t filterIndex = receivedMessage.idx;
    if (NULL != inFilterMatchCallBack) {
      inFilterMatchCallBack(filterIndex);
    }
    ACANCallBackRoutine callBackFunction = mCallBackFunctionArray[filterIndex];
    if (NULL != callBackFunction) {
      callBackFunction(receivedMessage);
    }
  }
  return hasReceived;
}

uint16_t Driver::internalBeginOperation(
    const Settings& inSettings, const ACAN2515Mask inRXM0,
    const ACAN2515Mask inRXM1,
    const ACAN2515AcceptanceFilter inAcceptanceFilters[],
    const uint8_t inAcceptanceFilterCount) {
  uint16_t errorCode = 0;
  //----------------------------------- Check if MCP2515 is accessible
  mSpi->beginTransaction(mSPISettings);
  write2515Register(CNF1_REGISTER, 0x55);
  bool ok = read2515Register(CNF1_REGISTER) == 0x55;
  if (ok) {
    write2515Register(CNF1_REGISTER, 0xAA);
    ok = read2515Register(CNF1_REGISTER) == 0xAA;
  }
  if (!ok) {
    errorCode = Error::NoMCP2515;
  }
  mSpi->endTransaction();
  //----------------------------------- Check if settings are correct
  if (!inSettings.mBitRateClosedToDesiredRate) {
    errorCode |= Error::TooFarFromDesiredBitRate;
  }
  if (inSettings.CANBitSettingConsistency() != 0) {
    errorCode |= Error::InconsistentBitRateSettings;
  }
  //----------------------------------- Allocate buffer
  if (!mReceiveBuffer.initWithSize(inSettings.mReceiveBufferSize)) {
    errorCode |= Error::CannotAllocateReceiveBuffer;
  }
  if (!mTransmitBuffer[0].initWithSize(inSettings.mTransmitBuffer0Size)) {
    errorCode |= Error::CannotAllocateTransmitBuffer0;
  }
  if (!mTransmitBuffer[1].initWithSize(inSettings.mTransmitBuffer1Size)) {
    errorCode |= Error::CannotAllocateTransmitBuffer1;
  }
  if (!mTransmitBuffer[2].initWithSize(inSettings.mTransmitBuffer2Size)) {
    errorCode |= Error::CannotAllocateTransmitBuffer2;
  }
  mTXBIsFree[0] = true;
  mTXBIsFree[1] = true;
  mTXBIsFree[2] = true;
  //----------------------------------- If ok, perform configuration
  if (errorCode == 0) {
    mSpi->beginTransaction(mSPISettings);
    //----------------------------------- Set CNF3, CNF2, CNF1 and CANINTE
    // registers
    select();
    mSpi->transfer(WRITE_COMMAND);
    mSpi->transfer(CNF3_REGISTER);
    //--- Register CNF3:
    //  Bit 7: SOF
    //  bit 6 --> 0: No Wake-up Filter bit
    //  Bit 5-3: -
    //  Bit 2-0: PHSEG2 - 1
    const uint8_t cnf3 =
        ((inSettings.mCLKOUT_SOF_pin == Settings::SOF) << 6) /* SOF */ |
        ((inSettings.mPhaseSegment2 - 1) << 0) /* PHSEG2 */
        ;
    mSpi->transfer(cnf3);
    //--- Register CNF2:
    //  Bit 7 --> 1: BLTMODE
    //  bit 6: SAM
    //  Bit 5-3: PHSEG1 - 1
    //  Bit 2-0: PRSEG - 1
    const uint8_t cnf2 = 0x80 /* BLTMODE */ |
                         (inSettings.mTripleSampling << 6) /* SAM */ |
                         ((inSettings.mPhaseSegment1 - 1) << 3) /* PHSEG1 */ |
                         ((inSettings.mPropagationSegment - 1) << 0) /* PRSEG */
        ;
    mSpi->transfer(cnf2);
    //--- Register CNF1:
    //  Bit 7-6: SJW - 1
    //  Bit 5-0: BRP - 1
    const uint8_t cnf1 =
        ((inSettings.mSJW - 1) << 6) /* SJW */ |  // Incorrect SJW setting fixed
                                                  // in 2.0.1
        ((inSettings.mBitRatePrescaler - 1) << 0) /* BRP */
        ;
    mSpi->transfer(cnf1);
    //--- Register CANINTE: activate interrupts
    //  Bit 7 --> 0: MERRE
    //  Bit 6 --> 0: WAKIE
    //  Bit 5 --> 0: ERRIE
    //  Bit 4 --> 1: TX2IE
    //  Bit 3 --> 1: TX1IE
    //  Bit 2 --> 1: TX0IE
    //  Bit 1 --> 1: RX1IE
    //  Bit 0 --> 1: RX0IE
    mSpi->transfer(0x1F);
    unselect();
    write2515Register(BFPCTRL_REGISTER, 0);
    write2515Register(TXRTSCTRL_REGISTER, 0);
    write2515Register(RXB0CTRL_REGISTER, ((uint8_t)inSettings.mRolloverEnable)
                                             << 2);
    write2515Register(RXB1CTRL_REGISTER, 0x00);
    setupMaskRegister(inRXM0, RXM0SIDH_REGISTER);
    setupMaskRegister(inRXM1, RXM1SIDH_REGISTER);
    if (inAcceptanceFilterCount > 0) {
      uint8_t idx = 0;
      while (idx < inAcceptanceFilterCount) {
        setupMaskRegister(inAcceptanceFilters[idx].mMask,
                          RXFSIDH_REGISTER[idx]);
        mCallBackFunctionArray[idx] = inAcceptanceFilters[idx].mCallBack;
        idx += 1;
      }
      while (idx < 6) {
        setupMaskRegister(
            inAcceptanceFilters[inAcceptanceFilterCount - 1].mMask,
            RXFSIDH_REGISTER[idx]);
        mCallBackFunctionArray[idx] =
            inAcceptanceFilters[inAcceptanceFilterCount - 1].mCallBack;
        idx += 1;
      }
    }
    write2515Register(TXB0CTRL_REGISTER, inSettings.mTXBPriority & 3);
    write2515Register(TXB1CTRL_REGISTER, (inSettings.mTXBPriority >> 2) & 3);
    write2515Register(TXB2CTRL_REGISTER, (inSettings.mTXBPriority >> 4) & 3);
    mSpi->endTransaction();
    uint8_t canctrl = inSettings.mOneShotModeEnabled ? (1 << 3) : 0;
    switch (inSettings.mCLKOUT_SOF_pin) {
      case Settings::CLOCK:
        canctrl |= 0x04 | 0x00;  // Same as default setting
        break;
      case Settings::CLOCK2:
        canctrl |= 0x04 | 0x01;
        break;
      case Settings::CLOCK4:
        canctrl |= 0x04 | 0x02;
        break;
      case Settings::CLOCK8:
        canctrl |= 0x04 | 0x03;
        break;
      case Settings::SOF:
        canctrl |= 0x04;
        break;
      case Settings::HiZ:
        break;
    }
    const uint8_t requestedMode = (uint8_t)inSettings.mOperationMode;
    errorCode |= setRequestedMode(canctrl | requestedMode);
  }
  return errorCode;
}

uint16_t Driver::setRequestedMode(const uint8_t inCANControlRegister) {
  uint16_t errorCode = 0;
  //--- Request mode
  mSpi->beginTransaction(mSPISettings);
  write2515Register(CANCTRL_REGISTER, inCANControlRegister);
  mSpi->endTransaction();
  //--- Wait until requested mode is reached (during 1 or 2 ms)
  bool wait = true;
  const uint32_t deadline = millis() + 2;
  while (wait) {
    mSpi->beginTransaction(mSPISettings);
    const uint8_t actualMode = read2515Register(CANSTAT_REGISTER) & 0xE0;
    mSpi->endTransaction();
    wait = actualMode != (inCANControlRegister & 0xE0);
    if (wait && (millis() >= deadline)) {
      errorCode |= Error::RequestedModeTimeOut;
      wait = false;
    }
  }
  //---
  return errorCode;
}

uint16_t Driver::changeModeOnTheFly(const OperationMode operationMode) {
  //--- Read current mode register (for saving settings of bits 0 ... 4)
  mSpi->beginTransaction(mSPISettings);
  const uint8_t currentMode = read2515Register(CANCTRL_REGISTER);
  mSpi->endTransaction();
  //--- New mode
  const uint8_t newMode = (currentMode & 0x1F) | (uint8_t)operationMode;
  //--- Set new mode
  const uint16_t errorCode = setRequestedMode(newMode);
  //---
  return errorCode;
}

uint16_t Driver::setFiltersOnTheFly(void) {
  return internalSetFiltersOnTheFly(ACAN2515Mask(), ACAN2515Mask(), NULL, 0);
}

uint16_t Driver::setFiltersOnTheFly(
    const ACAN2515Mask inRXM0,
    const ACAN2515AcceptanceFilter inAcceptanceFilters[],
    const uint8_t inAcceptanceFilterCount) {
  uint16_t errorCode = 0;
  if (inAcceptanceFilterCount == 0) {
    errorCode = Error::OneFilterMaskRequiresOneOrTwoAcceptanceFilters;
  } else if (inAcceptanceFilterCount > 2) {
    errorCode = Error::OneFilterMaskRequiresOneOrTwoAcceptanceFilters;
  } else if (inAcceptanceFilters == NULL) {
    errorCode = Error::AcceptanceFilterArrayIsNULL;
  } else {
    errorCode = internalSetFiltersOnTheFly(
        inRXM0, ACAN2515Mask(), inAcceptanceFilters, inAcceptanceFilterCount);
  }
  return errorCode;
}

uint16_t Driver::setFiltersOnTheFly(
    const ACAN2515Mask inRXM0, const ACAN2515Mask inRXM1,
    const ACAN2515AcceptanceFilter inAcceptanceFilters[],
    const uint8_t inAcceptanceFilterCount) {
  uint16_t errorCode = 0;
  if (inAcceptanceFilterCount < 3) {
    errorCode = Error::TwoFilterMasksRequireThreeToSixAcceptanceFilters;
  } else if (inAcceptanceFilterCount > 6) {
    errorCode = Error::TwoFilterMasksRequireThreeToSixAcceptanceFilters;
  } else if (inAcceptanceFilters == NULL) {
    errorCode = Error::AcceptanceFilterArrayIsNULL;
  } else {
    errorCode = internalSetFiltersOnTheFly(inRXM0, inRXM1, inAcceptanceFilters,
                                           inAcceptanceFilterCount);
  }
  return errorCode;
}

uint16_t Driver::internalSetFiltersOnTheFly(
    const ACAN2515Mask inRXM0, const ACAN2515Mask inRXM1,
    const ACAN2515AcceptanceFilter inAcceptanceFilters[],
    const uint8_t inAcceptanceFilterCount) {
  //--- Read current mode register
  mSpi->beginTransaction(mSPISettings);
  const uint8_t currentMode = read2515Register(CANCTRL_REGISTER);
  mSpi->endTransaction();
  //--- Request configuration mode
  const uint8_t configurationMode =
      (currentMode & 0x1F) | (0b100 << 5);  // Preserve bits 0 ... 4
  uint16_t errorCode = setRequestedMode(configurationMode);
  //--- Setup mask registers
  if (errorCode == 0) {
    setupMaskRegister(inRXM0, RXM0SIDH_REGISTER);
    setupMaskRegister(inRXM1, RXM1SIDH_REGISTER);
    if (inAcceptanceFilterCount > 0) {
      uint8_t idx = 0;
      while (idx < inAcceptanceFilterCount) {
        setupMaskRegister(inAcceptanceFilters[idx].mMask,
                          RXFSIDH_REGISTER[idx]);
        mCallBackFunctionArray[idx] = inAcceptanceFilters[idx].mCallBack;
        idx += 1;
      }
      while (idx < 6) {
        setupMaskRegister(
            inAcceptanceFilters[inAcceptanceFilterCount - 1].mMask,
            RXFSIDH_REGISTER[idx]);
        mCallBackFunctionArray[idx] =
            inAcceptanceFilters[inAcceptanceFilterCount - 1].mCallBack;
        idx += 1;
      }
    }
  }
  //--- Restore saved mode
  if (errorCode == 0) {
    errorCode = setRequestedMode(currentMode);
  }
  //---
  return errorCode;
}

void Driver::end(void) {
  if (isAttachedInterrupt) {
    detachInterrupt(digitalPinToInterrupt(mINT));
    isAttachedInterrupt = false;
  }
  //--- Request configuration mode
  const uint8_t configurationMode = (0b100 << 5);
  const uint16_t errorCode __attribute__((unused)) =
      setRequestedMode(configurationMode);
  //--- Deallocate driver buffers
  mTransmitBuffer[0].free();
  mTransmitBuffer[1].free();
  mTransmitBuffer[2].free();
  mReceiveBuffer.free();
}

void Driver::poll(void) {
  noInterrupts();
  while (isr_core()) {
  }
  interrupts();
}

void Driver::isr(void) { isr_core(); }

bool Driver::isr_core(void) {
  bool handled = false;
  mSpi->beginTransaction(mSPISettings);
  uint8_t itStatus = read2515Register(CANSTAT_REGISTER) & 0x0E;
  while (itStatus != 0) {
    handled = true;
    switch (itStatus) {
      case 0:  // No interrupt
        break;
      case 1 << 1:                                         // Error interrupt
        bitModify2515Register(CANINTF_REGISTER, 0x20, 0);  // Ack interrupt
        break;
      case 2 << 1:                                         // Wake-up interrupt
        bitModify2515Register(CANINTF_REGISTER, 0x40, 0);  // Ack interrupt
        break;
      case 3 << 1:  // TXB0 interrupt
        handleTXBInterrupt(0);
        break;
      case 4 << 1:  // TXB1 interrupt
        handleTXBInterrupt(1);
        break;
      case 5 << 1:  // TXB2 interrupt
        handleTXBInterrupt(2);
        break;
      case 6 << 1:  // RXB0 interrupt
      case 7 << 1:  // RXB1 interrupt
        handleRXBInterrupt();
        break;
    }
    itStatus = read2515Register(CANSTAT_REGISTER) & 0x0E;
  }
  mSpi->endTransaction();
  return handled;
}

void Driver::handleRXBInterrupt(void) {
  const uint8_t rxStatus =
      read2515RxStatus();  // Bit 6: message in RXB0, bit 7: message in RXB1
  const bool received = (rxStatus & 0xC0) != 0;
  if (received) {  // Message in RXB0 and / or RXB1
    const bool accessRXB0 = (rxStatus & 0x40) != 0;
    Frame message;
    message.rtr = (rxStatus & 0x08) !=
                  0;  // Thanks to Arjan-Woltjer for having fixed this bug
    message.ext = (rxStatus & 0x10) !=
                  0;  // Thanks to Arjan-Woltjer for having fixed this bug
                      //--- Set idx field to matching receive filter
    message.idx = rxStatus & 0x07;
    if (message.idx > 5) {
      message.idx -= 6;
    }
    //---
    select();
    mSpi->transfer(accessRXB0 ? READ_FROM_RXB0SIDH_COMMAND
                              : READ_FROM_RXB1SIDH_COMMAND);
    //--- SIDH
    message.id = mSpi->transfer(0);
    //--- SIDL
    const uint32_t sidl = mSpi->transfer(0);
    message.id <<= 3;
    message.id |= sidl >> 5;
    //--- EID8
    const uint32_t eid8 = mSpi->transfer(0);
    if (message.ext) {
      message.id <<= 2;
      message.id |= (sidl & 0x03);
      message.id <<= 8;
      message.id |= eid8;
    }
    //--- EID0
    const uint32_t eid0 = mSpi->transfer(0);
    if (message.ext) {
      message.id <<= 8;
      message.id |= eid0;
    }
    //--- DLC
    const uint8_t dlc = mSpi->transfer(0);
    message.len = dlc & 0x0F;
    //--- Read data
    for (int i = 0; i < message.len; i++) {
      message.data[i] = mSpi->transfer(0);
    }
    //---
    unselect();
    //--- Free receive buffer command
    bitModify2515Register(CANINTF_REGISTER, accessRXB0 ? 0x01 : 0x02, 0);
    //--- Enter received message in receive buffer (if not full)
    mReceiveBuffer.append(message);
  }
}

void Driver::handleTXBInterrupt(
    const uint8_t inTXB) {  // inTXB value is 0, 1 or 2
                            //--- Acknowledge interrupt
  bitModify2515Register(CANINTF_REGISTER, 0x04 << inTXB, 0);
  //--- Send an other message ?
  CanBusData_asukiaaa::Frame message;
  const bool ok = mTransmitBuffer[inTXB].remove(message);
  if (ok) {
    internalSendMessage(message, inTXB);
  } else {
    mTXBIsFree[inTXB] = true;
  }
}

void Driver::internalSendMessage(const CanBusData_asukiaaa::Frame& inFrame,
                                 const uint8_t inTXB) {  // inTXB is 0, 1 or 2
  //--- Send command
  //      send via TXB0: 0x81
  //      send via TXB1: 0x82
  //      send via TXB2: 0x84
  const uint8_t sendCommand = REQUEST_TO_SEND_COMMAND | (1 << inTXB);
  //--- Load TX buffer command
  //      Load TXB0, start at TXB0SIDH: 0x40
  //      Load TXB1, start at TXB1SIDH: 0x42
  //      Load TXB2, start at TXB2SIDH: 0x44
  const uint8_t loadTxBufferCommand = LOAD_TX_BUFFER_COMMAND | (inTXB << 1);
  //--- Send message
  select();
  mSpi->transfer(loadTxBufferCommand);
  if (inFrame.ext) {  // Extended frame
    uint32_t v = inFrame.id >> 21;
    mSpi->transfer((uint8_t)v);      // ID28 ... ID21 --> SIDH
    v = (inFrame.id >> 13) & 0xE0;   // ID20, ID19, ID18 in bits 7, 6, 5
    v |= (inFrame.id >> 16) & 0x03;  // ID17, ID16 in bits 1, 0
    v |= 0x08;                       // Extended bit
    mSpi->transfer(
        (uint8_t)v);  // ID20, ID19, ID18, -, 1, -, ID17, ID16 --> SIDL
    v = (inFrame.id >> 8) & 0xFF;  // ID15, ..., ID8
    mSpi->transfer(
        (uint8_t)v);  // ID15, ID14, ID13, ID12, ID11, ID10, ID9, ID8 --> EID8
    v = inFrame.id & 0xFF;  // ID7, ..., ID0
    mSpi->transfer(
        (uint8_t)v);  // ID7, ID6, ID5, ID4, ID3, ID2, ID1, ID0 --> EID0
  } else {            // Standard frame
    uint32_t v = inFrame.id >> 3;
    mSpi->transfer((uint8_t)v);    // ID10 ... ID3 --> SIDH
    v = (inFrame.id << 5) & 0xE0;  // ID2, ID1, ID0 in bits 7, 6, 5
    mSpi->transfer((uint8_t)v);    // ID2, ID1, ID0, -, 0, -, 0, 0 --> SIDL
    mSpi->transfer(0x00);          // any value --> EID8
    mSpi->transfer(0x00);          // any value --> EID0
  }
  //--- DLC
  uint8_t v = inFrame.len;
  if (v > 8) {
    v = 8;
  }
  if (inFrame.rtr) {
    v |= 0x40;
  }
  mSpi->transfer(v);
  //--- Send data
  if (!inFrame.rtr) {
    for (uint8_t i = 0; i < inFrame.len; i++) {
      mSpi->transfer(inFrame.data[i]);
    }
  }
  unselect();
  //--- Write send command
  select();
  mSpi->transfer(sendCommand);
  unselect();
}

void Driver::write2515Register(const uint8_t inRegister,
                               const uint8_t inValue) {
  select();
  mSpi->transfer(WRITE_COMMAND);
  mSpi->transfer(inRegister);
  mSpi->transfer(inValue);
  unselect();
}

uint8_t Driver::read2515Register(const uint8_t inRegister) {
  select();
  mSpi->transfer(READ_COMMAND);
  mSpi->transfer(inRegister);
  const uint8_t readValue = mSpi->transfer(0);
  unselect();
  return readValue;
}

uint8_t Driver::read2515Status(void) {
  select();
  mSpi->transfer(READ_STATUS_COMMAND);
  const uint8_t readValue = mSpi->transfer(0);
  unselect();
  return readValue;
}

uint8_t Driver::read2515RxStatus(void) {
  select();
  mSpi->transfer(RX_STATUS_COMMAND);
  const uint8_t readValue = mSpi->transfer(0);
  unselect();
  return readValue;
}

void Driver::bitModify2515Register(const uint8_t inRegister,
                                   const uint8_t inMask, const uint8_t inData) {
  select();
  mSpi->transfer(BIT_MODIFY_COMMAND);
  mSpi->transfer(inRegister);
  mSpi->transfer(inMask);
  mSpi->transfer(inData);
  unselect();
}

void Driver::setupMaskRegister(const ACAN2515Mask inMask,
                               const uint8_t inRegister) {
  select();
  mSpi->transfer(WRITE_COMMAND);
  mSpi->transfer(inRegister);
  mSpi->transfer(inMask.mSIDH);
  mSpi->transfer(inMask.mSIDL);
  mSpi->transfer(inMask.mEID8);
  mSpi->transfer(inMask.mEID0);
  unselect();
}

uint8_t Driver::transmitErrorCounter(void) {
  mSpi->beginTransaction(mSPISettings);
  const uint8_t result = read2515Register(TEC_REGISTER);
  mSpi->endTransaction();
  return result;
}

uint8_t Driver::receiveErrorCounter(void) {
  mSpi->beginTransaction(mSPISettings);
  const uint8_t result = read2515Register(REC_REGISTER);
  mSpi->endTransaction();
  return result;
}

uint8_t Driver::errorFlagRegister(void) {
  mSpi->beginTransaction(mSPISettings);
  const uint8_t result = read2515Register(EFLG_REGISTER);
  mSpi->endTransaction();
  return result;
}

bool Driver::tryToSend(const Frame& inMessage) {
  //--- Fix send buffer index
  uint8_t idx = inMessage.idx;
  if (idx > 2) {
    idx = 0;
  }
  //--- Bug fix in 2.0.6 (thanks to Fergus Duncan): interrupts were only
  // disabled for Teensy boards
#ifndef ARDUINO_ARCH_ESP32
  noInterrupts();
#endif
  //---
  mSpi->beginTransaction(mSPISettings);
  bool ok = mTXBIsFree[idx];
  if (ok) {  // Transmit buffer and TXB are both free: transmit immediatly
    mTXBIsFree[idx] = false;
    internalSendMessage(inMessage, idx);
  } else {  // Enter in transmit buffer, if not full
    ok = mTransmitBuffer[idx].append(inMessage);
  }
  mSpi->endTransaction();
#ifndef ARDUINO_ARCH_ESP32
  interrupts();
#endif
  return ok;
}

bool Driver::tryToSend(const CanBusData_asukiaaa::Frame& inMessage) {
  return tryToSend(Frame(inMessage));
}

void Driver::sendReset() {
  mSpi->beginTransaction(mSPISettings);
  select();
  mSpi->transfer(RESET_COMMAND);
  unselect();
  mSpi->endTransaction();
  delay(5);
}

bool Driver::neededToHandleInterruptTask() {
  return !isAttachedInterrupt || mINT < 0 || digitalRead(mINT);
}

};  // namespace CanBusMCP2515_asukiaaa
