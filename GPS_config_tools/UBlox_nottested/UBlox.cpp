#import "Arduino.h"
#import "UBlox.h"

void UBlox::init()
{
    byte navMode;
    switch(_mode)
    {
    case 0: // Portable
        navMode = 0x00;
        break;
    case 1: // Stationary
        navMode = 0x02;
        break;
    case 3: // Automotive
        navMode = 0x04;
        break;
    case 4: // Sea
        navMode = 0x05;
        break;
    case 5: // <1G
        navMode = 0x06;
        break;
    case 6: // <2G
        navMode = 0x07;
        break;
    case 7: // <4G
        navMode = 0x08;
        break;
    default: // Pedestrian
        navMode = 0x03;
    }

    byte dataRate[2];
    switch(_rate)
    {
    case 0: // 1Hz
        dataRate[0] = 0xE8;
        dataRate[1] = 0x03;
        break;
    case 1: // 2Hz
        dataRate[0] = 0xF4;
        dataRate[1] = 0x01;
        break;
    case 2: // 3.33Hz
        dataRate[0] = 0x2C;
        dataRate[1] = 0x01;
        break;
    default: // 4Hz
        dataRate[0] = 0xFA;
        dataRate[1] = 0x00;
    }

    byte portRate[3];
    switch(_baudrate)
    {
    case 4800:
        portRate[0] = 0xC0;
        portRate[1] = 0x12;
        portRate[2] = 0x00;
        break;
    case 9600:
        portRate[0] = 0x80;
        portRate[1] = 0x25;
        portRate[2] = 0x00;
        break;
    case 19200:
        portRate[0] = 0x00;
        portRate[1] = 0x4B;
        portRate[2] = 0x00;
        break;
    case 57600:
        portRate[0] = 0x00;
        portRate[1] = 0xE1;
        portRate[2] = 0x00;
        break;
    case 115200:
        portRate[0] = 0x00;
        portRate[1] = 0xC2;
        portRate[2] = 0x01;
        break;
    case 230400:
        portRate[0] = 0x00;
        portRate[1] = 0x84;
        portRate[2] = 0x03;
        break;
    default:
        portRate[0] = 0x00;
        portRate[1] = 0x96;
        portRate[2] = 0x00;
    }

    byte settings[] = {navMode, dataRate[0], dataRate[1], portRate[0], portRate[1], portRate[2], 0x00, 0x00, 0x00, 0x00, 0x00};
    configureGPS(settings);
}

void UBlox::configureGPS(byte *settingsArrayPointer)
{
    byte gpsSetSuccess = 0;
    Serial.println("Configuring u-Blox GPS initial state...");

    //Generate the configuration string for Navigation Mode
    byte setNav[] =
    {
        0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, *settingsArrayPointer, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    //byte setNav[] = {0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x53};
    calcChecksum(&setNav[2], sizeof(setNav) - 4);

    byte setSBAS[] =
    {
        0xB5, 0x62, 0x06, 0x16, 0x08, 0x00, 0x01, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2D, 0xC9
    };
    calcChecksum(&setSBAS[2], sizeof(setSBAS) - 4);

    //Generate the configuration string for Data Rate
    //byte setDataRate[] = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, settingsArrayPointer[1], settingsArrayPointer[2], 0x01, 0x00, 0x01, 0x00, 0x00, 0x00}; // Using GPS time
    byte setDataRate[] =
    {
        0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, settingsArrayPointer[1], settingsArrayPointer[2], 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
    }; // Using UTC time
    calcChecksum(&setDataRate[2], sizeof(setDataRate) - 4);

    //Generate the configuration string for Baud Rate
    byte setPortRate[] =
    {
        0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x02, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, settingsArrayPointer[3], settingsArrayPointer[4], settingsArrayPointer[5], 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    calcChecksum(&setPortRate[2], sizeof(setPortRate) - 4);

    byte setPortRate2[] =
    {
        0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, settingsArrayPointer[3], settingsArrayPointer[4], settingsArrayPointer[5], 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    calcChecksum(&setPortRate2[2], sizeof(setPortRate2) - 4);

    byte setGLL[] =
    {
        0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x2B
    };
    byte setGSA[] =
    {
        0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x32
    };
    byte setGSV[] =
    {
        0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x39
    };
    byte setRMC[] =
    {
        0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x40
    };
    byte setVTG[] =
    {
        0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x46
    };

    delay(2500);

    while(gpsSetSuccess < 3)
    {
        Serial.print("Setting Navigation Mode... ");
        sendUBX(&setNav[0], sizeof(setNav));  //Send UBX Packet
        gpsSetSuccess += getUBX_ACK(&setNav[2]); //Passes Class ID and Message ID to the ACK Receive function
        if (gpsSetSuccess == 5)   // If timeout
        {
            gpsSetSuccess -= 4;
            setBaud(settingsArrayPointer[4]);
            delay(1500);
            byte lowerPortRate[] =
            {
                0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x80, 0x25, 0x00, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA2, 0xB5
            };
            sendUBX(lowerPortRate, sizeof(lowerPortRate));
            gpsSerial->begin(9600);
            delay(2000);
        }
        if(gpsSetSuccess == 6) gpsSetSuccess -= 4;
        if (gpsSetSuccess == 10) gpsStatus[0] = true;
    }
    if (gpsSetSuccess == 3) Serial.println("Navigation mode configuration failed.");
    gpsSetSuccess = 0;

    while(gpsSetSuccess < 3)
    {
        Serial.print("Setting Data Update Rate... ");
        sendUBX(&setDataRate[0], sizeof(setDataRate));  //Send UBX Packet
        gpsSetSuccess += getUBX_ACK(&setDataRate[2]); //Passes Class ID and Message ID to the ACK Receive function
        if (gpsSetSuccess == 10) gpsStatus[1] = true;
        if (gpsSetSuccess == 5 | gpsSetSuccess == 6) gpsSetSuccess -= 4;
    }
    if (gpsSetSuccess == 3) Serial.println("Data update mode configuration failed.");
    gpsSetSuccess = 0;

    while(gpsSetSuccess < 3)
    {
        Serial.print("Setting SBAS... ");
        sendUBX(&setSBAS[0], sizeof(setSBAS));  //Send UBX Packet
        gpsSetSuccess += getUBX_ACK(&setSBAS[2]); //Passes Class ID and Message ID to the ACK Receive function
        //if (gpsSetSuccess == 10) ;
        if (gpsSetSuccess == 5 | gpsSetSuccess == 6) gpsSetSuccess -= 4;
    }
    if (gpsSetSuccess == 3) Serial.println("SBAS configuration failed.");
    gpsSetSuccess = 0;

    while(gpsSetSuccess < 3 && settingsArrayPointer[6] == 0x00)
    {
        Serial.print("Deactivating NMEA GLL Messages ");
        sendUBX(setGLL, sizeof(setGLL));
        gpsSetSuccess += getUBX_ACK(&setGLL[2]);
        if (gpsSetSuccess == 10) gpsStatus[2] = true;
        if (gpsSetSuccess == 5 | gpsSetSuccess == 6) gpsSetSuccess -= 4;
    }
    if (gpsSetSuccess == 3) Serial.println("NMEA GLL Message Deactivation Failed!");
    gpsSetSuccess = 0;

    while(gpsSetSuccess < 3 && settingsArrayPointer[7] == 0x00)
    {
        Serial.print("Deactivating NMEA GSA Messages ");
        sendUBX(setGSA, sizeof(setGSA));
        gpsSetSuccess += getUBX_ACK(&setGSA[2]);
        if (gpsSetSuccess == 10) gpsStatus[3] = true;
        if (gpsSetSuccess == 5 | gpsSetSuccess == 6) gpsSetSuccess -= 4;
    }
    if (gpsSetSuccess == 3) Serial.println("NMEA GSA Message Deactivation Failed!");
    gpsSetSuccess = 0;

    while(gpsSetSuccess < 3 && settingsArrayPointer[8] == 0x00)
    {
        Serial.print("Deactivating NMEA GSV Messages ");
        sendUBX(setGSV, sizeof(setGSV));
        gpsSetSuccess += getUBX_ACK(&setGSV[2]);
        if (gpsSetSuccess == 10) gpsStatus[4] = true;
        if (gpsSetSuccess == 5 | gpsSetSuccess == 6) gpsSetSuccess -= 4;
    }
    if (gpsSetSuccess == 3) Serial.println("NMEA GSV Message Deactivation Failed!");
    gpsSetSuccess = 0;

    while(gpsSetSuccess < 3 && settingsArrayPointer[9] == 0x00)
    {
        Serial.print("Deactivating NMEA RMC Messages ");
        sendUBX(setRMC, sizeof(setRMC));
        gpsSetSuccess += getUBX_ACK(&setRMC[2]);
        if (gpsSetSuccess == 10) gpsStatus[5] = true;
        if (gpsSetSuccess == 5 | gpsSetSuccess == 6) gpsSetSuccess -= 4;
    }
    if (gpsSetSuccess == 3) Serial.println("NMEA RMC Message Deactivation Failed!");
    gpsSetSuccess = 0;

    while(gpsSetSuccess < 3 && settingsArrayPointer[10] == 0x00)
    {
        Serial.print("Deactivating NMEA VTG Messages ");
        sendUBX(setVTG, sizeof(setVTG));
        gpsSetSuccess += getUBX_ACK(&setVTG[2]);
        if (gpsSetSuccess == 10) gpsStatus[6] = true;
        if (gpsSetSuccess == 5 | gpsSetSuccess == 6) gpsSetSuccess -= 4;
    }
    if (gpsSetSuccess == 3) Serial.println("NMEA VTG Message Deactivation Failed!");

    gpsSetSuccess = 0;
    if (settingsArrayPointer[4] != 0x25)
    {
        Serial.print("Setting Port Baud Rate... ");
        sendUBX(&setPortRate[0], sizeof(setPortRate));
        setBaud(settingsArrayPointer[4]);
        Serial.println("Success!");
        delay(500);
    }

    Serial.println("Setting Port Baud Rate 2...");
    sendUBX_Msg(setPortRate2, sizeof(setPortRate2));

    Serial.println("Finished setup.");
}

void UBlox::calcChecksum(byte *checksumPayload, byte payloadSize)
{
    byte CK_A = 0, CK_B = 0;
    for (int i = 0; i < payloadSize ; i++)
    {
        CK_A = CK_A + *checksumPayload;
        CK_B = CK_B + CK_A;
        checksumPayload++;
    }
    *checksumPayload = CK_A;
    checksumPayload++;
    *checksumPayload = CK_B;
}

void UBlox::sendUBX(byte *UBXmsg, byte msgLength)
{
    for(int i = 0; i < msgLength; i++)
    {
        gpsSerial->write(UBXmsg[i]);
        gpsSerial->flush();
    }
    gpsSerial->println();
    gpsSerial->flush();
}

byte UBlox::getUBX_ACK(byte *msgID)
{
    byte CK_A = 0, CK_B = 0;
    byte incoming_char;
    boolean headerReceived = false;
    unsigned long ackWait = millis();
    byte ackPacket[10] =
    {
        0xB5, 0x62, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    int i = 0;
    while (1)
    {
        if (gpsSerial->available())
        {
            incoming_char = gpsSerial->read();
            if (incoming_char == ackPacket[i])
            {
                i++;
            }
            else if (i > 2)
            {
                ackPacket[i] = incoming_char;
                i++;
            }
        }
        if (i > 9) break;
        if ((millis() - ackWait) > 1500)
        {
            Serial.println("ACK Timeout");
            return 5; // 00000101
        }
        if (i == 4 && ackPacket[3] == 0x00)
        {
            Serial.println("NAK Received");
            return 1;
        }
    }

    for (i = 2; i < 8 ; i++)
    {
        CK_A = CK_A + ackPacket[i];
        CK_B = CK_B + CK_A;
    }
    if (msgID[0] == ackPacket[6] && msgID[1] == ackPacket[7] && CK_A == ackPacket[8] && CK_B == ackPacket[9])
    {
        Serial.println("Success!");
        Serial.print("ACK Received! ");
        printHex(ackPacket, sizeof(ackPacket));
        return 10; // 00001010
    }
    else
    {
        Serial.print("ACK Checksum Failure: ");
        printHex(ackPacket, sizeof(ackPacket));
        delay(1000);
        return 1;
    }
}

void UBlox::printHex(uint8_t *data, uint8_t length) // prints 8-bit data in hex
{
    char tmp[length*2+1];
    byte first ;
    int j=0;
    for (byte i = 0; i < length; i++)
    {
        first = (data[i] >> 4) | 48;
        if (first > 57) tmp[j] = first + (byte)7;
        else tmp[j] = first ;
        j++;

        first = (data[i] & 0x0F) | 48;
        if (first > 57) tmp[j] = first + (byte)7;
        else tmp[j] = first;
        j++;
    }
    tmp[length*2] = 0;
    for (byte i = 0, j = 0; i < sizeof(tmp); i++)
    {
        Serial.print(tmp[i]);
        if (j == 1)
        {
            Serial.print(" ");
            j = 0;
        }
        else j++;
    }
    Serial.println();
}

void UBlox::setBaud(byte baudSetting)
{
    if (baudSetting == 0x12) gpsSerial->begin(4800);
    if (baudSetting == 0x4B) gpsSerial->begin(19200);
    if (baudSetting == 0x96) gpsSerial->begin(38400);
    if (baudSetting == 0xE1) gpsSerial->begin(57600);
    if (baudSetting == 0xC2) gpsSerial->begin(115200);
    if (baudSetting == 0x84) gpsSerial->begin(230400);
}
