//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "i2c.h"
#include "serial.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

#define I2C_SCL (PORT_PA23)
#define I2C_SCL_GROUP (0)
#define I2C_SCL_PIN (PIN_PA23%32)
#define I2C_SCL_PMUX (I2C_SCL_PIN/2)

#define I2C_SDA (PORT_PA22)
#define I2C_SDA_GROUP (0)
#define I2C_SDA_PIN (PIN_PA22%32)
#define I2C_SDA_PMUX (I2C_SDA_PIN/2)


#define I2C_SERCOM            SERCOM3
#define I2C_SERCOM_PMUX       PORT_PMUX_PMUXE_C_Val
#define I2C_SERCOM_GCLK_ID    SERCOM3_GCLK_ID_CORE
#define I2C_SERCOM_CLK_GEN    0
#define I2C_SERCOM_APBCMASK   PM_APBCMASK_SERCOM3

#define I2C_MAX_LENGTH 255

enum
{
	I2C_TRANSFER_WRITE = 0,
	I2C_TRANSFER_READ  = 1,
};

//------------------------------------------------------------------------------
//     ___      __   ___  __   ___  ___  __
//      |  \ / |__) |__  |  \ |__  |__  /__`
//      |   |  |    |___ |__/ |___ |    .__/
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//                __          __        ___  __
//     \  /  /\  |__) |  /\  |__) |    |__  /__`
//      \/  /~~\ |  \ | /~~\ |__) |___ |___ .__/
//
//------------------------------------------------------------------------------

serial_t I2C_callback;
i2c_callback_t _i2c_callback;
i2c_callback_t _i2c_default;

uint8_t _i2c_tx_begun = 0;
uint8_t _i2c_rx_begun = 0;

// Remove this, tells serial to act like i2c
volatile uint8_t _i2c_init;
volatile uint8_t _i2c_addr;
volatile uint8_t _i2c_mode;
volatile uint8_t _i2c_flag;

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

uint8_t _i2c_failed_flag();
uint8_t _i2c_default_func();
uint8_t _SERCOM3_Handler();

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//==============================================================================
void i2c_init(uint32_t baudrate)
{
    // Set up i2c
    I2C_callback.rx = _SERCOM3_Handler;
    I2C_callback.tx = _SERCOM3_Handler;

    // Set the default rx/tx callbacks
    _i2c_default = _i2c_default_func;
    _i2c_callback = _i2c_default_func;

    _i2c_init = 0;
    _i2c_flag = 0;
}

//==============================================================================
uint8_t i2c_begin_read(uint8_t addr)
{
    // Remove this, initiates computer acting as i2c
    _i2c_addr = addr;
    _i2c_init = 1;
    serial_write(RX_S);

    // I2C_SERCOM->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;
	// I2C_SERCOM->I2CM.ADDR.reg = addr | I2C_TRANSFER_READ;
}

//==============================================================================
uint8_t i2c_begin_write(uint8_t addr)
{
    // Remove this, initiates computer acting as i2c
    _i2c_addr = addr;
    _i2c_init = 1;
    serial_write(TX_S);

    // I2C_SERCOM->I2CM.ADDR.reg = addr | I2C_TRANSFER_WRITE;
}

//==============================================================================
uint8_t i2c_tx_flag()
{
    // Check if tx is free
    return serial_tx_flag(); // SERCOM3->I2CM.INTFLAG.bit.MB;
}

//==============================================================================
uint8_t i2c_rx_flag()
{
    // Check if there is rx data
    return serial_rx_flag(); // SERCOM3->I2CM.INTFLAG.bit.SB;
}

//==============================================================================
uint8_t i2c_read()
{
    uint8_t data = serial_read();
    _i2c_flag = 1;
    serial_write(RX_C);
    return data;

    // return I2C_SERCOM->I2CM.DATA.reg;
}

//==============================================================================
void i2c_write(uint8_t data) {
    _i2c_addr = data;
    _i2c_init = 1;
    serial_write(TX_C);
    // I2C_SERCOM->I2CM.DATA.reg = data;
}

//==============================================================================
uint8_t i2c_register(i2c_callback_t callback)
{
    if (!i2c_registered()) {
        // Remove this 
        if (serial_register(I2C_callback)) {
            // Leave this
            _i2c_callback = callback;
            return 1;
        }
    }
    
    return 0;
}

//==============================================================================
void i2c_unregister(i2c_callback_t callback)
{
    // Remove this
    serial_unregister(I2C_callback);

    // Leave this
    _i2c_callback = _i2c_default;
}

//==============================================================================
void i2c_set_default(i2c_callback_t callback) {
    i2c_register(callback);
    _i2c_default = callback;
}

//==============================================================================
void i2c_rx_stop() {
    // Remove this
    _i2c_flag = 1;
    serial_write(RX_E);

    // I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
    // I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
}

//==============================================================================
void i2c_tx_stop() {
    // Remove this
    _i2c_flag = 1;
    serial_write(TX_E);

    // I2C_SERCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
}

//==============================================================================
uint8_t i2c_registered() {
    return (_i2c_callback != _i2c_default);
}

//------------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//------------------------------------------------------------------------------

//==============================================================================
uint8_t _i2c_failed_flag()
{
    return 0; // SERCOM3->I2CM.STATUS.bit.RXNACK;
}

//------------------------------------------------------------------------------
//      __                  __        __        __
//     /  `  /\  |    |    |__)  /\  /  ` |__/ /__`
//     \__, /~~\ |___ |___ |__) /~~\ \__, |  \ .__/
//
//------------------------------------------------------------------------------

//==============================================================================
uint8_t _i2c_default_func(){
    if (i2c_tx_flag()) {
        return 0;
    }
    else if (i2c_rx_flag()) {
        i2c_read();
        return 0;
    }
}

//------------------------------------------------------------------------------
//        __   __  , __
//     | /__` |__)  /__`   
//     | .__/ |  \  .__/
//
//------------------------------------------------------------------------------


//==============================================================================
uint8_t _SERCOM3_Handler() {
    // Remove this
    if (_i2c_init) {
        serial_write(_i2c_addr);
        return _i2c_init = 0;
    } else if (_i2c_flag) {
        return _i2c_flag = 0;
    }

    if (_i2c_failed_flag()) {
        return 1;
    }

    else if (_i2c_callback()) {
        // Remove this
        serial_unregister(I2C_callback);

        // Leave this
        i2c_unregister(_i2c_callback);
    }

    return 0;
}
