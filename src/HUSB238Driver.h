
#ifndef __HUSB238_H__
#define __HUSB238_H__

#include "Arduino.h"

#define HUSB238_I2CAddress (uint8_t)(0x08)

typedef enum
{
    PD_0_5A = 0x00,
    PD_0_7A,
    PD_1_0A,
    PD_1_25A,
    PD_1_5A,
    PD_1_75A,
    PD_2A,
    PD_2_25A,
    PD_2_5A,
    PD_2_75A,
    PD_3A,
    PD_3_25A,
    PD_3_5A,
    PD_4A,
    PD_4_5A,
    PD_5A,
} HUSB238_CURRENT_e;

typedef enum
{
    Not_sel = 0x0,
    PDO_5V = 0x1,
    PDO_9V = 0x2,
    PDO_12V = 0x3,
    PDO_15V = 0x8,
    PDO_18V = 0x9,
    PDO_20V = 0xa,
} HUSB238_SELECT_Voltage_e;

typedef enum
{
    Voltage_5V = 0x0,
    Voltage_9V,
    Voltage_12V,
    Voltage_15V,
    Voltage_18V,
    Voltage_20V,
} HUSB238_Voltage_e;

enum HUSB238_CMD
{
    Request_PDO = 0b00001,
    Get_SRC_Cap = 0b00100,
    Hard_Reset = 0b10000,
};

typedef union
{
    struct
    {
        HUSB238_CURRENT_e PD_SRC_CURRENT : 4;
        HUSB238_Voltage_e PD_SRC_VOLTAGE : 4;
    } bit;
    uint8_t all;
} HUSB238_Reg_PD_STATUS0;

typedef union
{
    struct
    {
        uint8_t CURRENT_5V : 2;
        uint8_t VOLTAGE_5V : 1;
        uint8_t PD_RESPONSE : 3;
        uint8_t ATTACH : 1;
        uint8_t CC_DIR : 1;
    } bit;
    uint8_t all;
} HUSB238_Reg_PD_STATUS1;

typedef union
{
    struct
    {
        HUSB238_CURRENT_e SRC_CURRENT : 4;
        uint8_t RESERVED : 3;
        uint8_t SRC_DETECTED : 1;
    } bit;
    uint8_t all;
} HUSB238_Reg_SRC_PDO;

typedef union
{
    struct
    {
        uint8_t RESERVED : 4;
        HUSB238_SELECT_Voltage_e PDO_SELECT : 4;
    } bit;
    uint8_t all;
} HUSB238_Reg_SRC_PDO_SEL;

typedef struct
{
    HUSB238_Reg_PD_STATUS0 PD_STATUS0;
    HUSB238_Reg_PD_STATUS1 PD_STATUS1;
    HUSB238_Reg_SRC_PDO SRC_PDO_5V;
    HUSB238_Reg_SRC_PDO SRC_PDO_9V;
    HUSB238_Reg_SRC_PDO SRC_PDO_12V;
    HUSB238_Reg_SRC_PDO SRC_PDO_15V;
    HUSB238_Reg_SRC_PDO SRC_PDO_18V;
    HUSB238_Reg_SRC_PDO SRC_PDO_20V;
    HUSB238_Reg_SRC_PDO_SEL SRC_PDO;
    uint8_t GO_COMMAND;
} HUSB238_reg_t;

enum HUSB238_reg_addr
{
    Reg_PD_STATUS0 = 0x00,
    Reg_PD_STATUS1,
    Reg_SRC_PDO_5V,
    Reg_SRC_PDO_9V,
    Reg_SRC_PDO_12V,
    Reg_SRC_PDO_15V,
    Reg_SRC_PDO_18V,
    Reg_SRC_PDO_20V,
    Reg_SRC_PDO_SEL,
    Reg_GO_COMMAND,
};

typedef struct
{
    bool detected;
    HUSB238_CURRENT_e current;
    HUSB238_Voltage_e voltage;
} HUSB238_Capability_t;

void HUSB238_Init(int sda, int scl);
void HUSB238_GetCapabilities(HUSB238_Voltage_e *voltage = nullptr, HUSB238_CURRENT_e *current = nullptr, HUSB238_Capability_t *pdoList = nullptr);
void HUSB238_GetCurrentMode(HUSB238_Voltage_e *voltage, HUSB238_CURRENT_e *current);
void HUSB238_SelVoltage(HUSB238_SELECT_Voltage_e voltage);
HUSB238_SELECT_Voltage_e HUSB238_Voltage2PDO(HUSB238_Voltage_e voltage);
float to_current(HUSB238_CURRENT_e c);
void HUSB238_HardReset();
#endif /* __HUSB238_H */