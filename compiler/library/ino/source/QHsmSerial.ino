#include "QHsmSerial.h"

QHsmSerial::QHsmSerial(unsigned long baud){
  _baud = baud;
  lastByte = -1;
}

bool QHsmSerial::byteReceived(){
  return lastByte != -1; 
}

bool QHsmSerial::noByteReceived(){
  return lastByte == -1; 
}

void QHsmSerial::readByte(){
  lastByte = Serial.read();
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