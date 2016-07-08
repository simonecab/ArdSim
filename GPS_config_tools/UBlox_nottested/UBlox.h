/*
* Author        : Ahmad Shahril Bin Mohd Ghani
* Date          : 27/6/2014
* Description   : This library is made to set up uBlox Neo-6M GPS Module distributed by GI Electronic using Arduino.
*                 This library is derived from the example at http://playground.arduino.cc/UBlox/GPS
*                 Note that this library only set up the GPS, so you could still use library like TinyGPS to process the data.
*                 It also only allows RX and TX pins which are not the hardware RX and TX pins.
* Product       : http://www.gie.com.my/products.php?action=wireless/gps/ublox_neo_6m
*/

#ifndef UBlox_h
#define UBlox_h

#include "Arduino.h"
#include "SoftwareSerial.h"

class UBlox
{
public:
    UBlox(byte rx, byte tx, unsigned int baudrate, byte mode = 2, byte rate = 4)
    {
        gpsSerial = new SoftwareSerial(rx, tx);
        _baudrate = baudrate;
        _mode = mode;
        _rate = rate;

        gpsStatus = new bool[7];

        for(int i=0; i<7; i++)
        {
            gpsStatus[i] = false;
        }
    }

    void init();

    unsigned int getBaudrate()
    {
        return _baudrate;
    }

    bool available()
    {
        return gpsSerial->available();
    }

    char read()
    {
        return gpsSerial->read();
    }

    bool sendUBX_Msg(byte *UBXmsg, byte length)
    {
        calcChecksum(&UBXmsg[2], length - 4);
        byte gpsSetSuccess = 0;

        while(gpsSetSuccess < 3)
        {
            sendUBX(&UBXmsg[0], length);  //Send UBX Packet
            gpsSetSuccess += getUBX_ACK(&UBXmsg[2]); //Passes Class ID and Message ID to the ACK Receive function

            if (gpsSetSuccess == 5 | gpsSetSuccess == 6)
                gpsSetSuccess -= 4;
        }

        if (gpsSetSuccess == 10)
            return true;
        else
            return false;
    }
private:
    SoftwareSerial *gpsSerial;
    unsigned int _baudrate;
    byte _mode, _rate;
    bool *gpsStatus;

    void configureGPS(byte *settingsArrayPointer);
    void calcChecksum(byte *checksumPayload, byte payloadSize);
    void sendUBX(byte *UBXmsg, byte msgLength);
    byte getUBX_ACK(byte *msgID);
    void printHex(uint8_t *data, uint8_t length);
    void setBaud(byte baudSetting);
};

#endif
