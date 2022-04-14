#include "HUSB238Driver.h"
#include "driver/i2c.h"

bool initlized = false;
HUSB238_SELECT_Voltage_e votlage_Table[] = {Not_sel, PDO_5V, PDO_9V, PDO_12V, PDO_15V, PDO_18V, PDO_20V};
float current_Table[] = {0.5f, 0.7f, 1.0f, 1.25f, 1.5f, 1.75f, 2.0f, 2.25f, 2.5f, 2.75f, 3.0f, 3.25f, 3.5f, 4.0f, 4.5f, 5.0f};

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
float to_current(HUSB238_CURRENT_e c)
{
    uint8_t i = c & 0xf;
    if (i <= PD_5A)
    {
        return current_Table[i];
    }
    else
    {
        return -1.0f;
    }
}
void HUSB238_ExtractEstd(HUSB238_Reg_PD_STATUS0 status, HUSB238_Voltage_e *voltage, HUSB238_CURRENT_e *current)
{
    if (!voltage || !current)
    {
        return;
    }
    *current = status.bit.PD_SRC_CURRENT;
    *voltage = status.bit.PD_SRC_VOLTAGE;
}

void HUSB238_RefreshSrcCap()
{
    uint8_t cmd = Get_SRC_Cap;
    HUSB238_WriteReg(HUSB238_I2CAddress, Reg_GO_COMMAND, cmd);
}

void HUSB238_ReadAllReg(uint8_t *regs)
{
    memset(regs, 0, 10);
    HUSB238_I2CReadBytes(HUSB238_I2CAddress, 0x00, 10, (uint8_t *)regs);
}

void HUSB238_ExtractCap(uint8_t *regs, HUSB238_Capability_t *pdoList)
{
    if (!pdoList || !regs)
    {
        return;
    }
    HUSB238_Reg_SRC_PDO *reg;
    HUSB238_Capability_t cap;
    for (int i = 0; i < 6; i++)
    {
        reg = (HUSB238_Reg_SRC_PDO *)(regs + i + 2);
        // memcpy(&reg, &regs[i + 2], sizeof(uint8_t));
        // reg = static_cast<HUSB238_Reg_SRC_PDO>(regs[i + 2]);
        if (reg->bit.SRC_DETECTED)
        {
            HUSB238_CURRENT_e current = reg->bit.SRC_CURRENT;
            cap.detected = true;
            cap.current = current;
                }
        else
        {
            cap.detected = false;
        }
        cap.voltage = static_cast<HUSB238_Voltage_e>(i + 1);
        pdoList[i] = cap;
    }
}

void HUSB238_GetCapabilities(HUSB238_Voltage_e *voltage, HUSB238_CURRENT_e *current, HUSB238_Capability_t *pdoList)
{
    uint8_t regs[10];
    HUSB238_RefreshSrcCap();
    delay(500);
    HUSB238_ReadAllReg(regs);
    HUSB238_ExtractEstd(*(HUSB238_Reg_PD_STATUS0 *)&regs[0], voltage, current);
    return HUSB238_ExtractCap(regs, pdoList);
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
    initlized = true;
}

void HUSB238_SelVoltage(HUSB238_SELECT_Voltage_e pdo)
{
    HUSB238_Reg_SRC_PDO_SEL targetPDO;
    targetPDO.all = 0xA;
    targetPDO.bit.PDO_SELECT = pdo;
    HUSB238_WriteReg(HUSB238_I2CAddress, Reg_SRC_PDO_SEL, targetPDO.all);
    HUSB238_RequestPDO();
}

void HUSB238_GetCurrentMode(HUSB238_Voltage_e *voltage, HUSB238_CURRENT_e *current)
{
    HUSB238_Reg_PD_STATUS0 status;
    HUSB238_I2CReadBytes(HUSB238_I2CAddress, 0x00, 1, (uint8_t *)&status);
    HUSB238_ExtractEstd(status, voltage, current);
}

HUSB238_SELECT_Voltage_e HUSB238_Voltage2PDO(HUSB238_Voltage_e voltage)
{
    return votlage_Table[voltage];
}