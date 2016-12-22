
/**************************************************************************//**
 * \file WriteReadByte.ino
 ******************************************************************************/
 
/******************************************************************************
 * Header file inclusions.
 ******************************************************************************/

#include <Wire.h>

#include <Eeprom24XX512.h>

/******************************************************************************
 * Private macro definitions.
 ******************************************************************************/

/**************************************************************************//**
 * \def EEPROM_ADDRESS
 * \brief Address of EEPROM memory on TWI bus.
 ******************************************************************************/
#define EEPROM_ADDRESS  0x57

/******************************************************************************
 * Private variable definitions.
 ******************************************************************************/

static Eeprom24XX512 eeprom(EEPROM_ADDRESS);

/******************************************************************************
 * Public function definitions.
 ******************************************************************************/

/**************************************************************************//**
 * \fn void setup()
 *
 * \brief
 ******************************************************************************/
void setup()
{
    // Initialize serial communication.
    Serial.begin(9600);
        
    // Initialize EEPROM library.
    eeprom.initialize();
    
    // Write a byte at address 0 in EEPROM memory.
    Serial.println("Write byte to EEPROM memory walking bit address...");
int addr=1; int data=1;
while (addr&0xFFFF)
{
    eeprom.writeByte(addr,data);
    delay(10);
    data++;
    addr <<= 1;
}
    // Write cycle time (tWR). See EEPROM memory datasheet for more details.
    delay(10);
    
    // Read a byte at address 0 in EEPROM memory.
    Serial.println("Read byte from EEPROM memory...");

addr=1; 
while (addr&0xFFFF)
{
    Serial.println (eeprom.readByte(addr));
    addr <<= 1;
}

 
    Serial.println("");
}

/**************************************************************************//**
 * \fn void loop()
 *
 * \brief
 ******************************************************************************/
void loop()
{

}