#include "QHsmSerial.h"

QHsmSerial::QHsmSerial(unsigned long baud, QHsm* qhsm){
  _qhsm = qhsm;
  _baud = baud;
  lastByte = -1;
}

void QHsmSerial::readByte(){
  lastByte = Serial.read();
  if(lastByte == -1){
    SIGNAL_DISPATCH(_qhsm, SERIAL_NO_DATA_RECEIVED_SIG);
  }
  else{
    SIGNAL_DISPATCH(_qhsm, SERIAL_RECEIVED_BYTE_SIG);
  }
}

void QHsmSerial::print(char msg[]){
  Serial.print(msg);
}

void QHsmSerial::print(int msg){
  Serial.print(msg);
}

void QHsmSerial::println(char msg[]){
  Serial.println(msg);
}

void QHsmSerial::println(int msg){
  Serial.println(msg);
}


void QHsmSerial::init(){
  Serial.begin(_baud);
}