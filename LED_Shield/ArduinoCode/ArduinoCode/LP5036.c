//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "LP5036.h"
#include "i2c.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

// Communication state names
#define LP5036_M_LED_NUM    (0)
#define LP5036_M_R_COLOR    (1)
#define LP5036_M_G_COLOR    (2)
#define LP5036_M_B_COLOR    (3)
#define LP5036_M_RESTART    (4)
#define LP5036_M_ADDR       (5)
#define LP5036_M_REG_CHNG   (6)
#define LP5036_M_BRIGHT     (7)
#define LP5036_M_TX_END     (8)
#define LP5036_M_DONE       (9)

#define LP5036_INIT         (0)
#define LP5036_INIT_TX      (1)
#define LP5036_INIT_DONE    (2)

#define LP5036_RESET        (0)
#define LP5036_RESET_TX     (1)
#define LP5036_RESET_DONE   (2)

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

led_t* _LP5036_led;
uint8_t _LP5036_led_num;
uint8_t _LP5036_addr;
volatile uint8_t _LP5036_state;

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

uint8_t _LP5036_init_callback();
uint8_t _LP5036_write_callback();
uint8_t _LP5036_reset_callback();

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//==============================================================================
void LP5036_init(uint8_t addr) {
    // First reset the LED driver
    LP5036_stop(addr);

    // Wait for I2C
    while (i2c_registered());
    _LP5036_state = LP5036_INIT;
    while (!i2c_register(_LP5036_init_callback));
    i2c_begin_write(addr);

    // Wait for callback to finish
    while (i2c_registered());
}

//==============================================================================
void LP5036_stop(uint8_t addr) {
    // Wait for I2C and send reset command
    while (i2c_registered());
    _LP5036_state = LP5036_RESET;
    while (!i2c_register(_LP5036_reset_callback));
    i2c_begin_write(addr);

    // Wait for callback to finish
    while (i2c_registered());
}

//==============================================================================
uint8_t LP5036_write(uint8_t addr, uint8_t led_num, led_t* led) {
    if (i2c_register(_LP5036_write_callback)) {
        _LP5036_led = led;
        _LP5036_led_num = led_num;
        _LP5036_addr = addr;
        _LP5036_state = LP5036_M_LED_NUM;
        i2c_begin_write(addr);
        return 1;
    }
    return 0;
}

//==============================================================================
uint8_t LP5036_write_complete() {
    return (_LP5036_state == LP5036_M_DONE);
}
//------------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//      __                  __        __        __
//     /  `  /\  |    |    |__)  /\  /  ` |__/ /__`
//     \__, /~~\ |___ |___ |__) /~~\ \__, |  \ .__/
//
//------------------------------------------------------------------------------

//==============================================================================
uint8_t _LP5036_init_callback() {
    uint8_t complete = 0;

    switch (_LP5036_state) {
        // Initialization callback
        case LP5036_INIT: {
            _LP5036_state = LP5036_INIT_TX;
            i2c_write(LP5036_DEV_CFG0);
        } break;

        case LP5036_INIT_TX: {
            _LP5036_state = LP5036_INIT_DONE;
            i2c_write(LP5036_DEV_CFG0_CHIP_EN);
        } break;

        case LP5036_INIT_DONE: {
            i2c_tx_stop();
            complete = 1;
        } break;
    }

    return complete;
}

//==============================================================================
uint8_t _LP5036_reset_callback() {
    uint8_t complete = 0;

    switch (_LP5036_state) {
        // Reset callback
        case LP5036_RESET: {
            _LP5036_state = LP5036_RESET_TX;
            i2c_write(LP5036_RESET_REG);
        } break;

        case LP5036_RESET_TX: {
            _LP5036_state = LP5036_RESET_DONE;
            i2c_write(LP5036_RESET_VAL);
        } break;

        case LP5036_RESET_DONE: {
            i2c_tx_stop();
            complete = 1;
        } break;
    }

    return complete;
}

//==============================================================================
uint8_t _LP5036_write_callback() {
    uint8_t complete = 0;

    // LED writing callback
    switch (_LP5036_state) {
        // Point at the correct register
        case LP5036_M_LED_NUM: {
            _LP5036_state = LP5036_M_R_COLOR;
            i2c_write(LP5036_LED_COLOR(_LP5036_led_num, LED_R));
        } break;

        // Write R
        case LP5036_M_R_COLOR: {
            _LP5036_state = LP5036_M_G_COLOR;
            i2c_write(_LP5036_led->R);
        } break;

        // Write G
        case LP5036_M_G_COLOR: {
            _LP5036_state = LP5036_M_B_COLOR;
            i2c_write(_LP5036_led->G);
        } break;

        // Write B
        case LP5036_M_B_COLOR: {
            _LP5036_state = LP5036_M_RESTART;
            i2c_write(_LP5036_led->B);
        } break;

        // Kill the com and reopen (would be removed)
        case LP5036_M_RESTART: {
            _LP5036_state = LP5036_M_ADDR;
            serial_write(TX_E);
        } break;

        // Restart com
        case LP5036_M_ADDR: {
            _LP5036_state = LP5036_M_REG_CHNG;
            i2c_begin_write(_LP5036_addr);
        } break;

        // Point at the brightness address
        case LP5036_M_REG_CHNG: {
            _LP5036_state = LP5036_M_BRIGHT;
            i2c_write(LP5036_LED_BRIGHT_REG(_LP5036_led_num));
        } break;

        // Write the brightness
        case LP5036_M_BRIGHT: {
            _LP5036_state = LP5036_M_TX_END;
            i2c_write(_LP5036_led->brightness);
        } break;

        // Clear and unregister the callback
        case LP5036_M_TX_END: {
            _LP5036_state = LP5036_M_DONE;
            i2c_tx_stop();
            complete = 1;
        } break;
    }

    return complete;
}

//------------------------------------------------------------------------------
//        __   __  , __
//     | /__` |__)  /__`   
//     | .__/ |  \  .__/
//
//------------------------------------------------------------------------------
