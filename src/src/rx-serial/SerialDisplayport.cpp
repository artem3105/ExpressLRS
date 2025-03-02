/************************************************************************************
Credits:
  This software is based on and uses software published by Richard Amiss 2023,
  QLiteOSD, which is based on work by Paul Kurucz (pkuruz):opentelem_to_bst_bridge 
  as well as software from d3ngit : djihdfpv_mavlink_to_msp_V2 and 
  crashsalot : VOT_to_DJIFPV

THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES, WHETHER EXPRESS, 
IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF 
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE 
COMPANY SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR 
CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
************************************************************************************/

#if defined(TARGET_RX)

#include "SerialDisplayport.h"
#include "crsf_protocol.h"
#include "OTA.h"

void SerialDisplayport::send(uint8_t messageID, void * payload, uint8_t size, Stream * _stream)
{
    _stream->write('$');
    _stream->write('M');
    _stream->write('>');
    _stream->write(size);
    _stream->write(messageID);
    uint8_t checksum = size ^ messageID;
    uint8_t * payloadPtr = (uint8_t*)payload;
    for (uint8_t i = 0; i < size; ++i)
    {
      uint8_t b = *(payloadPtr++);
      checksum ^= b;
    }
    _stream->write((uint8_t*)payload, size);
    _stream->write(checksum);
}

uint32_t SerialDisplayport::sendRCFrame(bool frameAvailable, bool frameMissed, uint32_t *channelData)
{
    bool armed = getArmedState();

    msp_status_t status;
    status.task_delta_time = 0;
    status.i2c_error_count = 0;
    status.sensor_status = 0;
    status.flight_mode_flags = armed ? 0x1 : 0x0;
    status.pid_profile = 0;
    status.system_load = 0;
    status.gyro_cycle_time = 0;
    status.box_mode_flags = 0;
    status.arming_disable_flags_count = 1;
    status.arming_disable_flags = armed ? 0x0 : 0x1;
    status.extra_flags = 0;

    // Send status MSP
    send(MSP_STATUS, &status, sizeof(status), _outputPort);

    // Send extended status MSP
    send(MSP_STATUS_EX, &status, sizeof(status), _outputPort);

    return MSP_MSG_PERIOD_MS;   // Send MSP msgs to DJI at 10Hz
}

void SerialDisplayport::processBytes(uint8_t *bytes, u_int16_t size)
{
    // Super-basic air-unit detection:
    // Wait for at least 6 bytes to be received (minimum MSP msg length)
    // before we decide we are connected to a DJI air unit
    if (m_receivedBytes < 6)
    {
      m_receivedBytes += size;
    }
}

bool SerialDisplayport::getArmedState()
{
    if (firmwareOptions.dji_permanently_armed)
    {
        // If we are using permanent arming then we need to make sure the air-unti is connected
        // The O3 needs to see the arm state change (i.e. false --> true) after is has fully booted
        // Wait for activity on the UART from the air-unit, then wait 10 seconds before arming

        if (m_receivedBytes >= 6)
        {
            // Start the timer if it's the first time receiving 6 or more bytes
            if (m_receivedTimestamp == 0)
            {
                m_receivedTimestamp = millis();
            }
            // Arm permanently after 10 seconds
            return millis() > m_receivedTimestamp + 10000;
        }

        return false;
    }
    else
    {
        // If we are not using permanent arming then we don't need to wait for the air-unit to be connected
        // The arm channel will provide the required state change to arm the O3
        return isArmed;
    }
}

#endif // defined(TARGET_RX)
