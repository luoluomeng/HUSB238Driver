

#include "Arduino.h"

#include "PowerAdapter.h"

#define husb238_sda 22
#define husb238_scl 21
void setup()
{
    Serial.begin(115200);
    Serial.println("start...");
    initPowerAdapter(husb238_sda, husb238_scl);
    setVoltage(Voltage_20V);
    Serial.println("setup end...");
}

int loppno = 0;
void loop()
{
    switch (loppno)
    {
    case 0:
        setVoltage(Voltage_9V);
        loppno++;
        break;
    case 1:
        setVoltage(Voltage_15V);
        loppno++;
        break;
    case 2:
        setVoltage(Voltage_20V);
        loppno = 0;
        break;
    }
    delay(5000);
}
