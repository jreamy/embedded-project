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
volatile i2c_callback_t _i2c_callback;
i2c_callback_t _i2c_default;

// Remove these, things for masking serial as i2c
volatile uint8_t _i2c_init;
volatile uint8_t _i2c_addr;
volatile uint8_t _i2c_mode;
volatile uint8_t _i2c_flag;
volatile uint8_t _reading;
volatile uint8_t _i2c_rx_buff;

// Timeout variables
volatile uint8_t _i2c_active;

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
void i2c_init(uint32_t baudrate) {
    // Set up i2c forwarding to serial
    I2C_callback.rx = _SERCOM3_Handler;
    I2C_callback.tx = _SERCOM3_Handler;

    // Remove these
    _i2c_init = 0;
    _i2c_addr = 0;
    _i2c_mode = 0;
    _i2c_flag = 0;
    _reading = 0;
    _i2c_rx_buff = 0;

    // Set the default rx/tx callbacks
    _i2c_default = _i2c_default_func;
    _i2c_callback = _i2c_default_func;

    /* Actual I2C setup vv

    // Set up the SCL Pin
    //Set the direction - it is an output, but we want the input enable on as well
    //so that we can read it at the same time ... because I2C.
    PORT->Group[I2C_SCL_GROUP].DIRSET.reg = I2C_SCL;
    PORT->Group[I2C_SCL_GROUP].PINCFG[I2C_SCL_PIN].bit.INEN = 1;
    // Set the pullup
    PORT->Group[I2C_SCL_GROUP].OUTSET.reg = I2C_SCL;
    PORT->Group[I2C_SCL_GROUP].PINCFG[I2C_SCL_PIN].bit.PULLEN = 1;
    //Set the PMUX
    PORT->Group[I2C_SCL_GROUP].PINCFG[I2C_SCL_PIN].bit.PMUXEN = 1;
    if (I2C_SCL_PIN & 1)
    PORT->Group[I2C_SCL_GROUP].PMUX[I2C_SCL_PMUX].bit.PMUXO = I2C_SERCOM_PMUX;
    else
    PORT->Group[I2C_SCL_GROUP].PMUX[I2C_SCL_PMUX].bit.PMUXE = I2C_SERCOM_PMUX;


    // Set up the SDA PIN
    //Set the direction - it is an output, but we want the input enable on as well
    //so that we can read it at the same time ... because I2C.
    PORT->Group[I2C_SDA_GROUP].DIRSET.reg = I2C_SDA;
    PORT->Group[I2C_SDA_GROUP].PINCFG[I2C_SDA_PIN].bit.INEN = 1;
    // Set the pullup
    PORT->Group[I2C_SDA_GROUP].OUTSET.reg = I2C_SDA;
    PORT->Group[I2C_SDA_GROUP].PINCFG[I2C_SDA_PIN].bit.PULLEN = 1;
    //Set the PMUX
    PORT->Group[I2C_SDA_GROUP].PINCFG[I2C_SDA_PIN].bit.PMUXEN = 1;
    if (I2C_SDA_PIN & 1)
    PORT->Group[I2C_SDA_GROUP].PMUX[I2C_SDA_PMUX].bit.PMUXO = I2C_SERCOM_PMUX;
    else
    PORT->Group[I2C_SDA_GROUP].PMUX[I2C_SDA_PMUX].bit.PMUXE = I2C_SERCOM_PMUX;
    
    // Turn on the clock
    PM->APBCMASK.reg |= I2C_SERCOM_APBCMASK;

    // Configure the clock
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(I2C_SERCOM_GCLK_ID) |
    GCLK_CLKCTRL_CLKEN |
    GCLK_CLKCTRL_GEN(I2C_SERCOM_CLK_GEN);

    //Turn off the I2C enable so that we can write the protected registers
    I2C_SERCOM->I2CM.CTRLA.bit.ENABLE = 0;
    while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

    // Turn on smart mode (because it is smart)
    I2C_SERCOM->I2CM.CTRLB.bit.SMEN = 1;
    while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

    // Set the baud rate - this is a confusing little formula as
    // it involves the actual rise time of SCL on the board
    // We would need to measure this to predict the outcome,
    // Or, we can just change it until we like it.
    // See 27.6.2.4 of the datasheet.
    I2C_SERCOM->I2CM.BAUD.reg = SERCOM_I2CM_BAUD_BAUD(232);
    while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

    // Set us up as a Master
    I2C_SERCOM->I2CM.CTRLA.bit.MODE = SERCOM_I2CM_CTRLA_MODE_I2C_MASTER_Val;
    while (I2C_SERCOM->I2CM.SYNCBUSY.reg);
    
    // Set the hold time to 600ns
    I2C_SERCOM->I2CM.CTRLA.bit.SDAHOLD == 3;
    while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

    //Turn on the I2C enable
    I2C_SERCOM->I2CM.CTRLA.bit.ENABLE = 1;
    while (I2C_SERCOM->I2CM.SYNCBUSY.reg);

    // Set the bus state to be IDLE (this has to be after the enable)
    I2C_SERCOM->I2CM.STATUS.reg |= SERCOM_I2CM_STATUS_BUSSTATE(1);
    while (I2C_SERCOM->I2CM.SYNCBUSY.reg);
    

    // Enable interrupts
    I2C_SERCOM->I2CM.INTENSET.bit.MB = 1;
    I2C_SERCOM->I2CM.INTENSET.bit.SB = 1;
    
    NVIC_SetPriority(SERCOM3_IRQn, 3);
    NVIC_EnableIRQ(SERCOM3_IRQn);
    */
}

//==============================================================================
void i2c_stop() {
    i2c_unregister(_i2c_callback);

    // Remove these
    _i2c_init = 0;
    _i2c_addr = 0;
    _i2c_mode = 0;
    _i2c_flag = 0;
    _reading = 0;
    _i2c_rx_buff = 0;
}

//==============================================================================
uint8_t i2c_timeout() {
    if (_i2c_active) return _i2c_active = 0;
    else return i2c_registered();
}

//==============================================================================
uint8_t i2c_begin_read(uint8_t addr)
{
    // Remove this, initiates computer acting as i2c
    _i2c_init = 1;
    _i2c_addr = addr;
    _reading = 1;
    serial_write(RX_S);

    // Leave this, timeout update
    _i2c_active = 1;

    // I2C_SERCOM->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;
	// I2C_SERCOM->I2CM.ADDR.reg = addr | I2C_TRANSFER_READ;
}

//==============================================================================
uint8_t i2c_begin_write(uint8_t addr)
{
    // Remove this, initiates computer acting as i2c
    _i2c_init = 1;
    _i2c_addr = addr;
    serial_write(TX_S);

    // Leave this, timeout update
    _i2c_active = 1;

    // I2C_SERCOM->I2CM.ADDR.reg = addr | I2C_TRANSFER_WRITE;
}

//==============================================================================
uint8_t i2c_tx_flag()
{
    // Check if tx is free
    return serial_tx_flag(); // SERCOM3->I2CM.INTFLAG.bit.MB;
}

//==============================================================================
uint8_t i2c_rx_flag() {
    // Check if there is rx data
    return serial_rx_flag(); // SERCOM3->I2CM.INTFLAG.bit.SB;
}

//==============================================================================
uint8_t i2c_read() {
    if (_reading) {
        uint8_t data = serial_read();
        _i2c_flag = 1;
        serial_write(RX_C);
        return data;
    } else {
        return _i2c_rx_buff;
    }

    // return I2C_SERCOM->I2CM.DATA.reg;
}

//==============================================================================
void i2c_write(uint8_t data) {
    _i2c_init = 1;
    _i2c_addr = data;
    serial_write(TX_C);
    // I2C_SERCOM->I2CM.DATA.reg = data;
}

//==============================================================================
uint8_t i2c_register(i2c_callback_t callback) {
    if (!i2c_registered()) {
        // Remove this 
        if (serial_register(I2C_callback)) {
            // Leave this
            _i2c_active = 1;
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
    _reading = 0;
    _i2c_rx_buff = serial_read();
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
uint8_t _i2c_failed_flag() {
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
uint8_t _SERCOM3_Handler()
{
    // Leave this, timeout update
    _i2c_active = 1;

    // Remove this
    if (_i2c_init) {
        _i2c_init = 0;
        serial_write(_i2c_addr);
        return _i2c_flag = _i2c_addr = 0;
    } else if (_i2c_flag) {
        return _i2c_flag = 0;
    }

    // Leave this
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
