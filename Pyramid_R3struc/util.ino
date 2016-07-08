

void ReadAccelMPU()
{
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  int Tmp = Wire.read() << 8 | Wire.read(); //  IGNORED 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}

void TestSensors()
{
#ifndef TESTSENSORS
  char a;
  Serial.println("Press x to stop");
  while (Serial.read() != 'x') {

    // MPU 6050
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
    AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    int Tmp = Wire.read() << 8 | Wire.read(); //  IGNORED 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
    Serial.print("MPU6050 Accel X = "); Serial.print(AcX);
    Serial.print(" Y = "); Serial.print(AcY) ;
    Serial.print(" Z = "); Serial.print(AcZ);
    Serial.print("  Gyro X = "); Serial.print(GyX);
    Serial.print(" Y = "); Serial.print(GyY) ;
    Serial.print(" Z = "); Serial.println(GyZ);


#ifdef ADXL345
    //ADXL345
    int x, y, z;
    adxl.readAccel(&AcX, &AcY, &AcZ); //read the accelerometer values and store them in variables  x,y,z
    Serial.print("ADXL345 Accel X = "); Serial.print(AcX);
    Serial.print(" Y = "); Serial.print(AcY) ;
    Serial.print(" Z = "); Serial.println(AcZ);
#endif



    {
      int Mx, My, Mz; //triple axis magnetic data

      //Tell the HMC5883L where to begin reading data
      Wire.beginTransmission(HMC5883_addr);
      Wire.write((uint8_t)0x03); //select register 3, X MSB register
      Wire.endTransmission();

      //Read data from each axis, 2 registers per axis
      Wire.requestFrom(HMC5883_addr, 6);
      if (6 <= Wire.available()) {
        Mx = Wire.read() << 8  | Wire.read(); //X msb ,  lsb
        Mz = Wire.read() << 8  | Wire.read(); //Z msb ,  lsb
        My = Wire.read() << 8  | Wire.read(); //Y msb ,  lsb
      }

      //Print out values of each axis
      Serial.print("HMC5883 COMPASS x : ");
      Serial.print(Mx);
      Serial.print("  y : ");
      Serial.print(My);
      Serial.print("  z : ");
      Serial.println(Mz);


  int Volt = analogRead(VOLTINPIN);
      Serial.print("  VOLTPIN ");       Serial.println(Volt);
    }
    delay(800);
  }
#endif
}

//////////////////////////////
// ON BOARD LED
/////////////////////////////
void blink(int tempo, int velocita)
{
  long int start = millis();
  while (millis() < (start + 1000 * (long int)tempo))
  {
    digitalWrite(LEDPIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay( velocita);            // wait for a time
    digitalWrite(LEDPIN, LOW);    // turn the LED off by making the voltage LOW
    delay( velocita);
  }
}



//////////////////////////////
// SYSTEM UTILITIES
/////////////////////////////
#ifdef UTILITIES
void EraseEEprom()
{
  int Eptr;
  Serial.println(F("Erase EEprom"));
  for (Eptr = 0; Eptr < 510; Eptr++)EEPROM[Eptr] = 0;
}


int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
#endif
