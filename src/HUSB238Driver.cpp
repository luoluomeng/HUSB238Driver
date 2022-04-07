#include "HUSB238Driver.h"
#include "driver/i2c.h"

bool initlized = false;
float EstdU;
float EstdI;

#define I2C_NUM I2C_NUM_0    /*!< I2C port number for master dev */
#define I2C_TX_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_RX_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_FREQ_HZ 400000   // 400000   /*!< I2C master clock frequency */

#define WRITE_BIT I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ   /*!< I2C master read */
#define ACK_CHECK_EN 1             /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0            /*!< I2C master will not check ack from slave */
#define ACK_VAL (i2c_ack_type_t)0  /*!< I2C ack value */
#define NACK_VAL (i2c_ack_type_t)1 /*!< I2C nack value */

int IRAM_ATTR HUSB238_I2CReadBytes(uint8_t slaveAddr, uint8_t startAddress, uint16_t nBytesRead, uint8_t *data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, slaveAddr << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, startAddress, ACK_CHECK_EN);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, slaveAddr << 1 | READ_BIT, ACK_CHECK_EN);
    if (nBytesRead > 1)
        i2c_master_read(cmd, data, nBytesRead - 1, ACK_VAL);
    i2c_master_read_byte(cmd, data + nBytesRead - 1, NACK_VAL);
    i2c_master_stop(cmd);

    int ret = i2c_master_cmd_begin(I2C_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

int IRAM_ATTR HUSB238_WriteReg(uint8_t slaveAddr, uint8_t writeAddress, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (slaveAddr << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, writeAddress, ACK_CHECK_EN);

    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);

    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    static uint8_t dataCheck;
    HUSB238_I2CReadBytes(slaveAddr, writeAddress, 1, &dataCheck);

    if (dataCheck != data)
        return -2;

    return ret;
}
float bit2current(uint8_t x)
{
    switch (x & 0xf)
    {
    case PD_0_5A:
        return 0.5f;
    case PD_0_7A:
        return 0.7f;
    case PD_1_0A:
        return 1.0f;
    case PD_1_25A:
        return 1.25f;
    case PD_1_5A:
        return 1.5f;
    case PD_1_75A:
        return 1.75f;
    case PD_2A:
        return 2.0f;
    case PD_2_25A:
        return 2.25f;
    case PD_2_5A:
        return 2.5f;
    case PD_2_75A:
        return 2.75f;
    case PD_3A:
        return 3.0f;
    case PD_3_25A:
        return 3.25f;
    case PD_3_5A:
        return 3.5f;
    case PD_4A:
        return 4.0f;
    case PD_4_5A:
        return 4.5f;
    case PD_5A:
        return 5.0f;
    }
    return -1.0f;
}
void HUSB238_ExtractEstd(HUSB238_Reg_PD_STATUS0 status)
{
    // uint8_t buf = 0;
    // HUSB238_Reg_PD_STATUS0 *status = (HUSB238_Reg_PD_STATUS0 *)&buf;
    // HUSB238_I2CReadBytes(HUSB238_I2CAddress, 0x00, 1, &buf);
    EstdI = bit2current(status.bit.PD_SRC_CURRENT);
    uint8_t voltage = status.bit.PD_SRC_VOLTAGE;
    switch (voltage)
    {
    case 1:
        EstdU = 5.0f;
        break;
    case 2:
        EstdU = 9.0f;
        break;
    case 3:
        EstdU = 12.0f;
        break;
    case 4:
        EstdU = 15.0f;
        break;
    case 5:
        EstdU = 18.0f;
        break;
    case 6:
        EstdU = 20.0f;
        break;
    }
    Serial.printf("%.1fV ", EstdU);
    Serial.printf("%.2fA\n", EstdI);
    Serial.println();
}
void HUSB238_GetSrcCap()
{
    uint8_t cmd = Get_SRC_Cap;
    HUSB238_WriteReg(HUSB238_I2CAddress, Reg_GO_COMMAND, cmd);
    uint8_t buf = 0;
    HUSB238_Reg_PD_STATUS0 *status = (HUSB238_Reg_PD_STATUS0 *)&buf;
    HUSB238_I2CReadBytes(HUSB238_I2CAddress, 0x00, 1, &buf);
    HUSB238_ExtractEstd(*status);
}
void HUSB238_ReadAllReg(HUSB238_reg_t *regs)
{
    memset(regs, 0, 10);
    HUSB238_I2CReadBytes(HUSB238_I2CAddress, 0x00, 10, (uint8_t *)regs);
}

HUSB238_PDOList HUSB238_ExtractCap(HUSB238_reg_t *regs)
{
    HUSB238_Reg_SRC_PDO *cap = new HUSB238_Reg_SRC_PDO;
    HUSB238_PDOList pdoList;
    HUSB238_DetectedVoltage_t pdo;
    for (int i = 0; i < 6; ++i)
    {

        *(uint8_t *)cap = ((uint8_t *)regs)[i + 2];
        if (cap->bit.SRC_DETECTED)
        {
            float current = bit2current(cap->bit.SRC_CURRENT);
            pdo.detected = true;
            pdo.current = current;
        }
        else
        {
            pdo.detected = false;
            pdo.current = 0;
        }
        uint8_t voltage = 0;
        switch (i)
        {
        case 0:
            pdo.voltage = 5;
            pdoList.PDO_5V = pdo;
            break;
        case 1:
            pdo.voltage = 9;
            pdoList.PDO_9V = pdo;
            break;
        case 2:
            pdo.voltage = 12;
            pdoList.PDO_5V = pdo;
            break;
        case 3:
            pdo.voltage = 15;
            pdoList.PDO_15V = pdo;
            break;
        case 4:
            pdo.voltage = 18;
            pdoList.PDO_18V = pdo;
            break;
        case 5:
            pdo.voltage = 20;
            pdoList.PDO_20V = pdo;
            break;
        }
    }
    delete cap;
}

HUSB238_PDOList HUSB238_GetCapabilities()
{
    HUSB238_reg_t regs;

    HUSB238_ReadAllReg(&regs);
    HUSB238_ExtractEstd(regs.PD_STATUS0);
    return HUSB238_ExtractCap(&regs);
}

void HUSB238_RequestPDO()
{
    uint8_t cmd = Request_PDO;
    HUSB238_WriteReg(HUSB238_I2CAddress, Reg_GO_COMMAND, cmd);
}

void HUSB238_HardReset()
{
    uint8_t cmd = Hard_Reset;
    HUSB238_WriteReg(HUSB238_I2CAddress, Reg_GO_COMMAND, cmd);
}

void HUSB238_Init(int sda, int scl)
{
    if (initlized)
    {
        return;
    }
    i2c_port_t i2c_master_port = I2C_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda;
    conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
    conf.scl_io_num = scl;
    conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
    conf.master.clk_speed = I2C_FREQ_HZ;
    conf.clk_flags = 0;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode, I2C_RX_BUF_DISABLE, I2C_TX_BUF_DISABLE, 0);
    HUSB238_GetSrcCap();
    initlized = true;
}

void HUSB238_SelVoltage(HUSB238_SELECT_Voltage_e voltage)
{
    HUSB238_Reg_SRC_PDO_SEL targetPDO;
    targetPDO.all = 0;
    targetPDO.bit.PDO_SELECT = voltage;
    HUSB238_WriteReg(HUSB238_I2CAddress, Reg_SRC_PDO_SEL, targetPDO.all);
    HUSB238_RequestPDO();
}

void HUSB238_GetCurrentMode(float *voltage, float *current)
{
    *voltage = EstdU;
    *current = EstdI;
}