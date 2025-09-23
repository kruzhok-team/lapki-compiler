#pragma once
#include "accel.c"
#define THR 800.0f

class Accel {
private:
uint8_t timeout = 50;
unsigned long _previous;
public:
    uint8_t orientation = ACCEL_DIR_NONE;
    uint8_t ACCEL_DIR_DN = 0;
    uint8_t ACCEL_DIR_UP = 1;
    uint8_t ACCEL_DIR_LEFT = 2;
    uint8_t ACCEL_DIR_RIGHT = 3;
    uint8_t ACCEL_DIR_FACE  = 4;
    uint8_t ACCEL_DIR_BACK = 5;
    uint8_t ACCEL_DIR_NONE = 6;
    AccelData accelData;

    Accel() {
        accelData.dn = 0;
        accelData.lt = 0;
        accelData.bk = 0;
    }

    void
    initAccel
    ( void )
    {
        __initAccel();
    }

    void
    readAccel
    ( void )
    {
        if (millis() - _previous > timeout) {
            __readAccel(&accelData);
            _previous = millis();
        }
    }

    uint8_t
    closestDir
    ( void )
    {
        uint8_t dir = ACCEL_DIR_NONE;
        uint8_t tmp = 0;
        if ( accelData.dn > tmp ) { tmp = accelData.dn; dir = ACCEL_DIR_DN; }
        if ( accelData.dn < -tmp ) { tmp = -accelData.dn; dir = ACCEL_DIR_UP; }
        if ( accelData.lt > tmp ) { tmp = accelData.lt; dir = ACCEL_DIR_LEFT; }
        if ( accelData.lt < -tmp ) { tmp = -accelData.lt; dir = ACCEL_DIR_RIGHT; }
        if ( accelData.bk > tmp ) { tmp = accelData.bk; dir = ACCEL_DIR_BACK; }
        if ( accelData.bk < -tmp ) { tmp = -accelData.bk; dir = ACCEL_DIR_FACE; }
        return dir;
    }

    uint8_t
    simpleDir
    ( void )
    {
        if ( accelData.dn > THR ) return ACCEL_DIR_DN;
        if ( accelData.dn < -THR ) return ACCEL_DIR_UP;
        if ( accelData.lt > THR ) return ACCEL_DIR_LEFT;
        if ( accelData.lt < -THR ) return ACCEL_DIR_RIGHT;
        if ( accelData.bk > THR ) return ACCEL_DIR_BACK;
        if ( accelData.bk < -THR ) return ACCEL_DIR_FACE;
        return ACCEL_DIR_NONE;
    }
    uint8_t isOrientationChanged() {
        if ( orientation == ACCEL_DIR_NONE ) {
            orientation = closestDir();
            return true;
        }
        uint8_t tmp = simpleDir();
        
        if ( tmp != ACCEL_DIR_NONE ) { 
            bool isChanged = orientation != tmp;
            orientation = tmp;
            return isChanged;
        }

        return false;
    }
};

