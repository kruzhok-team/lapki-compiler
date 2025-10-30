#pragma once

#include "i2c.c"

extern "C" {

typedef struct {
    float dn;
    float lt;
    float bk;
} AccelData;


void
__initAccel
( void )
{
  uint8_t accelRate = 0b0101; //100 Гц, LIS2DH12 p35 t31
  uint8_t base = 0b0111;
  uint8_t value = ( accelRate << 4 ) | base;
  accelWrite1(0x20, value );
  delay(10); //Без этого не начинает работать сразу
  #define CTRL_REG4_BLE 0b01000000;
  //accelWrite1(0x23, 0b00000000 );
  delay(10); //Без этого не начинает работать сразу
  accelRead1(0x26); //read REFERENCE to reset some stuff
  //accelWrite1(0x20, 0b00100111 );
  delay(10); //Без этого не начинает работать сразу
}

void
__readAccel
( AccelData* accel )
{
  accelReadN(0x28,6);
  int16_t rawX = (uint16_t)i2cReadData[0] | ( (uint16_t)i2cReadData[1] << 8 );
  int16_t rawY = (uint16_t)i2cReadData[2] | ( (uint16_t)i2cReadData[3] << 8 );
  int16_t rawZ = (uint16_t)i2cReadData[4] | ( (uint16_t)i2cReadData[5] << 8 );
  accel->dn =  ((float)rawX / 64.0f ) *  4.0f;
  accel->lt =  ((float)rawY / 64.0f ) *  4.0f;
  accel->bk =  ((float)rawZ / 64.0f ) *  4.0f;
}

bool
accelConnected
( void )
{
    return accelRead1(0x0f) == 0b00110011;
}
}