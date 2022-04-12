#include "PowerAdapter.h"

HUSB238_Capability_t PDCapabilities[6];

void initPowerAdapter(int sda, int scl)
{
    HUSB238_Voltage_e v;
    HUSB238_CURRENT_e i;
    delay(500);
    HUSB238_Init(sda, scl);
    // delay(500);
    HUSB238_GetCapabilities(&v, &i, PDCapabilities);
    Serial.printf("v:%d,i:%.2f \r\n", v, to_current(i));
    for (int i = 0; i < 6; i++)
    {
        if (PDCapabilities[i].detected)
        {
            Serial.printf("voltage:%d,current:%.2f \r\n", PDCapabilities[i].voltage, to_current(PDCapabilities[i].current));
        }
    }
}

uint8_t currentVoltage()
{
    HUSB238_Voltage_e v;
    HUSB238_CURRENT_e i;
    HUSB238_GetCurrentMode(&v, &i);
    return v;
}

bool isVoltageSupported(HUSB238_Voltage_e v)
{
    bool ret = false;
    if (v <= Voltage_20V)
    {
        ret = PDCapabilities[v].detected;
    }
    return ret;
}

void setVoltage(HUSB238_Voltage_e v)
{
    // if (!isVoltageSupported(v))
    // {
    //     return;
    // }
    HUSB238_SELECT_Voltage_e req = PDO_5V;
    if (v <= Voltage_20V)
    {
        req = HUSB238_Voltage2PDO(v);
    }
    HUSB238_SelVoltage(req);
}

void resetAdapter()
{
    HUSB238_HardReset();
}