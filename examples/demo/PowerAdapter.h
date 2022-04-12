#ifndef _POWERADAPTER_H_
#define _POWERADAPTER_H_
#include "Arduino.h"
#include "HUSB238Driver.h"
void initPowerAdapter(int sda, int scl);
bool isVoltageSupported(uint8_t voltage);
void setVoltage(HUSB238_Voltage_e v);
void resetAdapter();
#endif