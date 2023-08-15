#ifndef SERIAL_H
#define SERIAL_H

#include "qhsm.h"

class QHsmSerial {
    public:
        QHsmSerial(unsigned long baud,  QHsm* qhsm);
        void init();
        void readByte();

        void print(char value[]);
        void print(int value);
        void println(char value[]);
        void println(int value);
        
        int lastByte;
        unsigned long _baud;
        QHsm* _qhsm;

};

#endif