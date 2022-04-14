#ifndef _POWERADAPTER_H_
#define _POWERADAPTER_H_
#include "Arduino.h"
#include "HUSB238Driver.h"
HUSB238_Voltage_e initPowerAdapter(int sda, int scl, HUSB238_Voltage_e *vmax = nullptr);
bool isVoltageSupported(HUSB238_Voltage_e voltage);
void setVoltage(HUSB238_Voltage_e v);
void resetAdapter();
HUSB238_Voltage_e currentVoltage();
String generateStatus();
#endif