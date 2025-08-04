#include "i2c.c"

#define ACCEL_DIR_DN 0
#define ACCEL_DIR_UP 1
#define ACCEL_DIR_LEFT 2
#define ACCEL_DIR_RIGHT 3
#define ACCEL_DIR_FACE 4
#define ACCEL_DIR_BACK 5
#define ACCEL_DIR_NONE 6
#define THR 800.0f
#define CTRL_REG4_BLE 0b01000000;

uint8_t
closestDir(void)
{
  uint8_t dir = ACCEL_DIR_NONE;
  uint8_t tmp = 0;
  if (accel.dn > tmp)
  {
    tmp = accel.dn;
    dir = ACCEL_DIR_DN;
  }
  if (accel.dn < -tmp)
  {
    tmp = -accel.dn;
    dir = ACCEL_DIR_UP;
  }
  if (accel.lt > tmp)
  {
    tmp = accel.lt;
    dir = ACCEL_DIR_LEFT;
  }
  if (accel.lt < -tmp)
  {
    tmp = -accel.lt;
    dir = ACCEL_DIR_RIGHT;
  }
  if (accel.bk > tmp)
  {
    tmp = accel.bk;
    dir = ACCEL_DIR_BACK;
  }
  if (accel.bk < -tmp)
  {
    tmp = -accel.bk;
    dir = ACCEL_DIR_FACE;
  }
  return dir;
}

uint8_t
simpleDir(void)
{
  if (accel.dn > THR)
    return ACCEL_DIR_DN;
  if (accel.dn < -THR)
    return ACCEL_DIR_UP;
  if (accel.lt > THR)
    return ACCEL_DIR_LEFT;
  if (accel.lt < -THR)
    return ACCEL_DIR_RIGHT;
  if (accel.bk > THR)
    return ACCEL_DIR_BACK;
  if (accel.bk < -THR)
    return ACCEL_DIR_FACE;
  return ACCEL_DIR_NONE;
}

uint8_t
useAccel(void)
{
  if (currentDir == ACCEL_DIR_NONE)
  {
    currentDir = closestDir();
    return currentDir;
  }
  uint8_t tmp = simpleDir();
  if (tmp != ACCEL_DIR_NONE)
    currentDir = tmp;
  return currentDir;
}

bool accelConnected(void)
{
  return accelRead1(0x0f) == 0b00110011;
}

class Accel
{
public:
  Accel()
  {
    uint8_t accelRate = 0b0101; // 100 Гц, LIS2DH12 p35 t31
    uint8_t base = 0b0111;
    uint8_t value = (accelRate << 4) | base;
    accelWrite1(0x20, value);
    delay(10); // Без этого не начинает работать сразу<
    // accelWrite1(0x23, 0b00000000 );
    delay(10);        // Без этого не начинает работать сразу
    accelRead1(0x26); // read REFERENCE to reset some stuff
    // accelWrite1(0x20, 0b00100111 );
    delay(10); // Без этого не начинает работать сразу
  }

  void readAccel(void)
  {
    accelReadN(0x28, 6);
    int16_t rawX = (uint16_t)i2cReadData[0] | ((uint16_t)i2cReadData[1] << 8);
    int16_t rawY = (uint16_t)i2cReadData[2] | ((uint16_t)i2cReadData[3] << 8);
    int16_t rawZ = (uint16_t)i2cReadData[4] | ((uint16_t)i2cReadData[5] << 8);
    accel.dn = ((float)rawX / 64.0f) * 4.0f;
    accel.lt = ((float)rawY / 64.0f) * 4.0f;
    accel.bk = ((float)rawZ / 64.0f) * 4.0f;
  }

private:
  float dn = 0.0;
  float lt = 0.0;
  float bk = 0.0;
  uint8_t currentDir = ACCEL_DIR_NONE;
};
