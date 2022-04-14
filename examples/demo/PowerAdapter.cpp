#include "PowerAdapter.h"

HUSB238_Capability_t PDCapabilities[6];

HUSB238_Voltage_e initPowerAdapter(int sda, int scl, HUSB238_Voltage_e *vmax)
{
    HUSB238_Voltage_e v = Voltage_unknown;
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
            if (!vmax)
            {
                *vmax = PDCapabilities[i].voltage;
            }

            Serial.printf("voltage:%d,current:%.2f \r\n", PDCapabilities[i].voltage, to_current(PDCapabilities[i].current));
        }
    }
    return v;
}

HUSB238_Voltage_e currentVoltage()
{
    HUSB238_Voltage_e v = Voltage_unknown;
    HUSB238_CURRENT_e i = PD_0_5A;
    HUSB238_GetCurrentMode(&v, &i);
    return v;
}

bool isVoltageSupported(HUSB238_Voltage_e v)
{
    bool ret = false;
    if (v > Voltage_unknown && v <= Voltage_20V)
    {
        ret = PDCapabilities[v - 1].detected;
    }
    return ret;
}

void setVoltage(HUSB238_Voltage_e v)
{
    if (!isVoltageSupported(v))
    {
        return;
    }
    HUSB238_SELECT_Voltage_e req = PDO_5V;
    if (v > Voltage_unknown && v <= Voltage_20V)
    {
        req = HUSB238_Voltage2PDO(v);
    }
    HUSB238_SelVoltage(req);
}

void resetAdapter()
{
    HUSB238_HardReset();
}
int voltages[] = {5, 9, 12, 15, 18, 20};
String generateStatus()
{
    String ret = "";
    ret += "{\"capabilities\":[";

    for (int i = 0; i < 6; i++)
    {
        if (i != 0)
        {
            ret += ",";
        }
        ret += "{\"Voltage\":";
        ret += "\"";
        ret += String(voltages[PDCapabilities[i].voltage - 1]);
        ret += "\",";

        ret += "\"Detected\":";
        ret += "\"";
        ret += PDCapabilities[i].detected ? "true" : "false";
        ret += "\",";

        ret += "\"Current\":";
        ret += "\"";
        ret += String(to_current(PDCapabilities[i].current));
        ret += "\"";
        ret += "}";
    }
    ret += ",";
    ret += "{\"CurrentVoltage\":";
    ret += "\"";

    ret += String(voltages[currentVoltage() - 1]);
    ret += "\"}";
    ret += "]}";
    return ret;
}