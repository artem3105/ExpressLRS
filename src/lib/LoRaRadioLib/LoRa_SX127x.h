#pragma once

#include <Arduino.h>
#include "LoRa_SX127x_Regs.h"

typedef enum
{
    CURR_OPMODE_FSK_OOK = 0b00000000,
    CURR_OPMODE_LORA = 0b10000000, //removed CURR_OPMODE_ACCESS_SHARED_REG_OFF and CURR_OPMODE_ACCESS_SHARED_REG_ON for now
    CURR_OPMODE_SLEEP = 0b00000000,
    CURR_OPMODE_STANDBY = 0b00000001,
    CURR_OPMODE_FSTX = 0b00000010,
    CURR_OPMODE_TX = 0b00000011,
    CURR_OPMODE_FSRX = 0b00000100,
    CURR_OPMODE_RXCONTINUOUS = 0b00000101,
    CURR_OPMODE_RXSINGLE = 0b00000110,
    CURR_OPMODE_CAD = 0b00000111
} RadioOPmodes;

typedef enum
{
    CH_SX1272,
    CH_SX1273,
    CH_SX1276,
    CH_SX1277,
    CH_SX1278,
    CH_SX1279
} Chip;
typedef enum
{
    BW_7_80_KHZ = 0,
    BW_10_40_KHZ = 1,
    BW_15_60_KHZ = 2,
    BW_20_80_KHZ = 3,
    BW_31_25_KHZ = 4,
    BW_41_70_KHZ = 5,
    BW_62_50_KHZ = 6,
    BW_125_00_KHZ = 7,
    BW_250_00_KHZ = 8,
    BW_500_00_KHZ = 9
} Bandwidth;
typedef enum
{
    SF_6,
    SF_7,
    SF_8,
    SF_9,
    SF_10,
    SF_11,
    SF_12
} SpreadingFactor;
typedef enum
{
    CR_4_5,
    CR_4_6,
    CR_4_7,
    CR_4_8
} CodingRate;
typedef enum
{
    RFMOD_SX1278,
    RFMOD_SX1276
} RFmodule_;

/*typedef enum
{
    RADIO_IDLE,
    RADIO_BUSY
} RadioState_;
*/

class SX127xDriver
{

public:
    SX127xDriver();

    ///////Callback Function Pointers/////
    static void (*RXdoneCallback1)(); //function pointer for callback
    static void (*RXdoneCallback2)(); //function pointer for callback
    static void (*TXdoneCallback1)(); //function pointer for callback
    static void (*TXdoneCallback2)(); //function pointer for callback
    static void (*TXdoneCallback3)(); //function pointer for callback
    static void (*TXdoneCallback4)(); //function pointer for callback

    //static void (*TXtimeout)(); //function pointer for callback
    //static void (*RXtimeout)(); //function pointer for callback

    ////////Hardware/////////////
    int8_t _RXenablePin;
    int8_t _TXenablePin;

    static uint8_t SX127x_dio0;
    static uint8_t SX127x_dio1;
    static uint8_t SX127x_RST;

    /////////////////////////////

    ///////////Radio Variables////////
    volatile uint8_t TXdataBuffer[16];
    volatile uint8_t RXdataBuffer[16];

    uint8_t TXbuffLen = 8;
    uint8_t RXbuffLen = 8;

    uint32_t PacketCount;

    bool headerExplMode;

    RFmodule_ RFmodule;
    Bandwidth currBW;
    SpreadingFactor currSF;
    CodingRate currCR;
    uint8_t _syncWord;
    uint32_t currFreq;
    uint8_t currPWR;
    //uint8_t maxPWR;
    //static RadioOPmodes _opmode;
    //static RadioState_ RadioState;
    ///////////////////////////////////

    /////////////Packet Stats//////////
    volatile int8_t LastPacketRSSI;
    volatile int8_t LastPacketSNR;
    volatile uint8_t NonceTX;
    volatile uint8_t NonceRX;
    //static uint32_t TimeOnAir;
    //static uint32_t TXstartMicros;
    //static uint32_t TXspiTime;
    //static uint32_t HeadRoom;
    //static uint32_t LastTXdoneMicros;
    //static uint32_t TXdoneMicros;
    /////////////////////////////////

    ////////////////Configuration Functions/////////////
    void SetPins(int rst, int dio0, int dio1);
    uint8_t Begin(bool HighPowerModule = false, int txpin = -1, int rxpin = -1);
    uint8_t Config(Bandwidth bw, SpreadingFactor sf, CodingRate cr, uint32_t freq, uint8_t syncWord);
    uint8_t SX127xConfig(uint8_t bw, uint8_t sf, uint8_t cr, uint32_t freq, uint8_t syncWord);

    uint8_t SetBandwidth(Bandwidth bw);
    uint32_t getCurrBandwidth();
    uint32_t getCurrBandwidthNormalisedShifted();
    uint8_t SetSyncWord(uint8_t syncWord);
    uint8_t SetOutputPower(uint8_t Power);
    uint8_t SetPreambleLength(uint8_t PreambleLen);
    uint8_t SetSpreadingFactor(SpreadingFactor sf);
    uint8_t SetCodingRate(CodingRate cr);
    uint8_t SetFrequency(uint32_t freq);
    int32_t GetFrequencyError();
    void setPPMoffsetReg(int32_t offset);

    uint8_t SX127xBegin();
    uint8_t SetMode(uint8_t mode);
    uint8_t TX(uint8_t *data, uint8_t length);
    ////////////////////////////////////////////////////

    void ICACHE_RAM_ATTR TXnbISR(); //ISR for non-blocking TX routine
    void ICACHE_RAM_ATTR RXnbISR(); //ISR for non-blocking RC routine

    /////////////////Utility Funcitons//////////////////
    void ClearIRQFlags();

    //////////////RX related Functions/////////////////

    uint8_t RunCAD();

    uint8_t ICACHE_RAM_ATTR GetLastPacketRSSIUnsigned() const;
    int8_t ICACHE_RAM_ATTR GetLastPacketRSSI() const;
    int8_t ICACHE_RAM_ATTR GetLastPacketSNR() const;
    int8_t ICACHE_RAM_ATTR GetCurrRSSI() const;

    ////////////Non-blocking TX related Functions/////////////////
    uint8_t ICACHE_RAM_ATTR TXnb(const volatile uint8_t *data, uint8_t length);

    /////////////Non-blocking RX related Functions///////////////
    void ICACHE_RAM_ATTR StopContRX();
    void ICACHE_RAM_ATTR RXnb();

    uint8_t ICACHE_RAM_ATTR RXsingle(uint8_t *data, uint8_t length);
    uint8_t ICACHE_RAM_ATTR RXsingle(uint8_t *data, uint8_t length, uint32_t timeout);

private:
};

extern SX127xDriver Radio;
