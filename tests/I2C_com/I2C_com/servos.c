//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "servos.h"
#include "i2c.h"
#include "timer.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

#define PWM_MODE_1      (0x00)
#define PWM_MODE_2      (0x01)
#define PWM0_ON_L       (0x06)
#define PWM_ON_L        (0x06%4)
#define PWM_ON_H        (0x07%4)
#define PWM_OFF_L       (0x08%4)
#define PWM_OFF_H       (0x09%4)
#define ALL_ON_L        (0xFA)
#define ALL_ON_H        (0xFB)
#define ALL_OFF_L       (0xFC)
#define ALL_OFF_H       (0xFD)
#define PRE_SCALE       (0xFE)

// Mode 1
#define PWM_MODE_1_RESTART  (1<<7)
#define PWM_MODE_1_EXTCLK   (1<<6)
#define PWM_MODE_1_AI       (1<<5)
#define PWM_MODE_1_SLEEP    (1<<4)
#define PWM_MODE_1_ALLCALL  (1<<0)

// State machine
#define SERVOS_REG  (0)
#define SERVOS_TX   (1)
#define SERVOS_STOP (2)
#define SERVOS_DONE (3)


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

volatile uint8_t _servos_mode;
volatile uint8_t _servos_reg;
uint8_t _servos_data;
uint8_t* _servos_list;
uint8_t _servos_idx;

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

uint8_t _servos_byte_callback();
uint8_t _servos_regs_callback();

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//==============================================================================
void servos_init(uint8_t addr) {
    // Turn the device off (sleep)
    servos_stop(addr);

    timer_delay(50);

    // Set prescale
    while (i2c_registered());
    while (!i2c_register(_servos_byte_callback));
    _servos_mode = SERVOS_REG;
    _servos_reg = PRE_SCALE;    // 20ms period -> 50 Hz
    _servos_data = 0x79;        // 50Hz = round(25MHz/4096* 50) - 1
    i2c_begin_write(addr);

    timer_delay(10);

    // Turn the device back on
    while (i2c_registered());
    while (!i2c_register(_servos_byte_callback));
    _servos_mode = SERVOS_REG;
    _servos_reg = PWM_MODE_1;
    _servos_data = PWM_MODE_1_AI;
    i2c_begin_write(addr);
    while (i2c_registered());

    timer_delay(10);
}

//==============================================================================
void servos_stop(uint8_t addr) {
    // Turn the device off
    while (i2c_registered());
    while (!i2c_register(_servos_byte_callback));
    _servos_mode = SERVOS_REG;
    _servos_reg = PWM_MODE_1;
    _servos_data = PWM_MODE_1_SLEEP;
    i2c_begin_write(addr);
    while (i2c_registered());
}

//==============================================================================
uint8_t servos_write(uint8_t addr, uint8_t* bytes) {
    // 20ms -> 50Hz
    // 1ms = -90 deg
    // 2ms = +90 deg
    // 1ms = 4096/20 ~= 205
    // pw = 205+(ang*205/255) <- convert angle to steps
    if (i2c_register(_servos_regs_callback)) {
        _servos_mode = SERVOS_REG;
        _servos_reg = PWM0_ON_L;
        _servos_list = bytes;
        i2c_begin_write(addr);
        return 1;
    }

    return 0;
}

//==============================================================================
uint8_t servos_write_complete() {
    return (_servos_mode == SERVOS_DONE);
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

uint8_t _servos_byte_callback() {
    uint8_t complete = 0;

    switch (_servos_mode) {
        case SERVOS_REG: {
            _servos_mode = SERVOS_TX;
            i2c_write(_servos_reg);
        } break;

        case SERVOS_TX: {
            _servos_mode = SERVOS_STOP;
            i2c_write(_servos_data);
        } break;

        case SERVOS_STOP: {
            i2c_tx_stop();
            complete = 1;
        } break;
    } // end switch

    return complete;
}

uint8_t _servos_regs_callback() {
    uint8_t complete = 0;
    uint32_t data;

    switch (_servos_mode) {
        case SERVOS_REG: {
            _servos_mode = SERVOS_TX;
            _servos_idx = 0;
            i2c_write(_servos_reg);
        } break;

        case SERVOS_TX: {
            switch (_servos_reg++%4) {
                case PWM_ON_L: {
                    i2c_write(0);
                } break;

                case PWM_ON_H: {
                    i2c_write(0);
                } break;

                case PWM_OFF_L: {
                    data = _servos_list[_servos_idx];
                    data = (data * 4) / 5; // convert 255 -> 204
                    _servos_data = (data + 205) >> 8; // next reg
                    i2c_write(data);
                } break;
                
                case PWM_OFF_H: {
                    i2c_write(_servos_data);
                    if (++_servos_idx >= 15) {
                        _servos_mode = SERVOS_STOP;
                    }
                } break;
            } // end switch
        } break;

        case SERVOS_STOP: {
            _servos_mode = SERVOS_DONE;
            i2c_tx_stop();
            complete = 1;
        } break;
    } // end switch

    return complete;
}

//------------------------------------------------------------------------------
//        __   __  , __
//     | /__` |__)  /__`   
//     | .__/ |  \  .__/
//
//------------------------------------------------------------------------------
