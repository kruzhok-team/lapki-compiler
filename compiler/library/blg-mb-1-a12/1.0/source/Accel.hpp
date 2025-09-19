#pragma once
#include "i2c.c"

#define THR 800.0f

class Accel {
    uint8_t currentDir = ACCEL_DIR_NONE;
    uint8_t ACCEL_DIR_DN = 0;
    uint8_t ACCEL_DIR_UP = 1;
    uint8_t ACCEL_DIR_LEFT = 2;
    uint8_t ACCEL_DIR_RIGHT = 3;
    uint8_t ACCEL_DIR_FACE  = 4;
    uint8_t ACCEL_DIR_BACK = 5;
    uint8_t ACCEL_DIR_NONE = 6;
    float dn;
    float lt;
    float bk;

    Accel() {
        dn = 0;
        lt = 0;
        bk = 0;
    }

    void
    initAccel
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
    readAccel
    ( void )
    {
        accelReadN(0x28,6);
        int16_t rawX = (uint16_t)i2cReadData[0] | ( (uint16_t)i2cReadData[1] << 8 );
        int16_t rawY = (uint16_t)i2cReadData[2] | ( (uint16_t)i2cReadData[3] << 8 );
        int16_t rawZ = (uint16_t)i2cReadData[4] | ( (uint16_t)i2cReadData[5] << 8 );
        dn =  ((float)rawX / 64.0f ) *  4.0f;
        lt =  ((float)rawY / 64.0f ) *  4.0f;
        bk =  ((float)rawZ / 64.0f ) *  4.0f;
    }

    uint8_t
    closestDir
    ( void )
    {
        uint8_t dir = ACCEL_DIR_NONE;
        uint8_t tmp = 0;
        if ( dn > tmp ) { tmp = dn; dir = ACCEL_DIR_DN; }
        if ( dn < -tmp ) { tmp = -dn; dir = ACCEL_DIR_UP; }
        if ( lt > tmp ) { tmp = lt; dir = ACCEL_DIR_LEFT; }
        if ( lt < -tmp ) { tmp = -lt; dir = ACCEL_DIR_RIGHT; }
        if ( bk > tmp ) { tmp = bk; dir = ACCEL_DIR_BACK; }
        if ( bk < -tmp ) { tmp = -bk; dir = ACCEL_DIR_FACE; }
        return dir;
    }

    uint8_t
    simpleDir
    ( void )
    {
        if ( dn > THR ) return ACCEL_DIR_DN;
        if ( dn < -THR ) return ACCEL_DIR_UP;
        if ( lt > THR ) return ACCEL_DIR_LEFT;
        if ( lt < -THR ) return ACCEL_DIR_RIGHT;
        if ( bk > THR ) return ACCEL_DIR_BACK;
        if ( bk < -THR ) return ACCEL_DIR_FACE;
        return ACCEL_DIR_NONE;
    }
    uint8_t isOrientationChanged() {
        if ( currentDir == ACCEL_DIR_NONE ) {
            currentDir = closestDir();
            return true;
        }
        uint8_t tmp = simpleDir();
        
        if ( tmp != ACCEL_DIR_NONE ) { 
            bool isChanged = currentDir != tmp;
            currentDir = tmp;
            return isChanged;
        }

        return false;
    }
};

