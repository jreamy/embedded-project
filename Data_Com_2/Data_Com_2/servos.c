//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "servos.h"
#include "i2c.h"
#include "timer.h"
#define ARM_MATH_CM0PLUS
#include <arm_math.h>

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

// Computation machine
#define SERVOS_COMP_PNT_TIP (0)
#define SERVOS_COMP_PNT_MID (1)
#define SERVOS_COMP_PNT_BS  (2)
#define SERVOS_COMP_MID_BS  (3)
#define SERVOS_COMP_MID_MID (4)
#define SERVOS_COMP_MID_TIP (5)
#define SERVOS_COMP_RNG_TIP (6)
#define SERVOS_COMP_RNG_MID (7)
#define SERVOS_COMP_RNG_BS  (8)
#define SERVOS_COMP_PNK_BS  (9)
#define SERVOS_COMP_PNK_MID (10)
#define SERVOS_COMP_PNK_TIP (11)
#define SERVOS_COMP_THM_BS  (12)
#define SERVOS_COMP_THM_MID (13)
#define SERVOS_COMP_THM_TIP (14)
#define SERVOS_COMP_DONE    (15)


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

hand_pos_t* _servos_comp_data;
uint8_t* _servos_comp_output;
uint8_t _servos_comp_idx;

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

uint8_t _servos_byte_callback();
uint8_t _servos_regs_callback();
uint8_t _servos_comp(joint_pos_t* joint1, joint_pos_t* joint2);
float _servos_dot(joint_pos_t* joint1, joint_pos_t* joint2);
float _servos_mag(joint_pos_t* joint);

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
uint8_t servos_write(uint8_t addr, hand_ang_t* bytes) {
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

//==============================================================================
uint8_t servos_compute(hand_pos_t* data, hand_ang_t* output) {
    _servos_comp_idx = 0;
    _servos_comp_data = data;
    _servos_comp_output = output;
    return 1;
}

//==============================================================================
uint8_t servos_compute_complete() {
    uint8_t complete;

    switch (_servos_comp_idx) {
        case SERVOS_COMP_PNT_TIP: {
            _servos_comp_output[_servos_comp_idx++] = _servos_comp(
                &_servos_comp_data->pnt.tip, 
                &_servos_comp_data->pnt.mid);
        } break;

        case SERVOS_COMP_PNT_MID: {
            _servos_comp_output[_servos_comp_idx++] = _servos_comp(
                &_servos_comp_data->pnt.mid,
                &_servos_comp_data->pnt.bs);
        } break;

        case SERVOS_COMP_PNT_BS: {
            _servos_comp_output[_servos_comp_idx++] = _servos_comp(
                &_servos_comp_data->pnt.bs,
                &_servos_comp_data->hnd);
        } break;

        case SERVOS_COMP_MID_TIP: {
            _servos_comp_output[_servos_comp_idx++] = _servos_comp(
                &_servos_comp_data->mid.tip,
                &_servos_comp_data->mid.mid);
        } break;

        case SERVOS_COMP_MID_MID: {
            _servos_comp_output[_servos_comp_idx++] = _servos_comp(
                &_servos_comp_data->mid.mid,
                &_servos_comp_data->mid.bs);
        } break;

        case SERVOS_COMP_MID_BS: {
            _servos_comp_output[_servos_comp_idx++] = _servos_comp(
                &_servos_comp_data->mid.bs,
                &_servos_comp_data->hnd);
        } break;

        case SERVOS_COMP_RNG_TIP: {
            _servos_comp_output[_servos_comp_idx++] = _servos_comp(
                &_servos_comp_data->rng.tip,
                &_servos_comp_data->rng.mid);
        } break;

        case SERVOS_COMP_RNG_MID: {
            _servos_comp_output[_servos_comp_idx++] = _servos_comp(
                &_servos_comp_data->rng.mid,
                &_servos_comp_data->rng.bs);
        } break;

        case SERVOS_COMP_RNG_BS: {
            _servos_comp_output[_servos_comp_idx++] = _servos_comp(
                &_servos_comp_data->rng.bs,
                &_servos_comp_data->hnd);
        } break;

        case SERVOS_COMP_PNK_TIP: {
            _servos_comp_output[_servos_comp_idx++] = _servos_comp(
                &_servos_comp_data->pnk.tip,
                &_servos_comp_data->pnk.mid);
        } break;

        case SERVOS_COMP_PNK_MID: {
            _servos_comp_output[_servos_comp_idx++] = _servos_comp(
                &_servos_comp_data->pnk.mid,
                &_servos_comp_data->pnk.bs);
        } break;

        case SERVOS_COMP_PNK_BS: {
            _servos_comp_output[_servos_comp_idx++] = _servos_comp(
                &_servos_comp_data->pnk.bs,
                &_servos_comp_data->hnd);
        } break;

        case SERVOS_COMP_THM_TIP: {
            _servos_comp_output[_servos_comp_idx++] = _servos_comp(
                &_servos_comp_data->thm.tip,
                &_servos_comp_data->thm.mid);
        } break;

        case SERVOS_COMP_THM_MID: {
            _servos_comp_output[_servos_comp_idx++] = _servos_comp(
                &_servos_comp_data->thm.mid,
                &_servos_comp_data->thm.bs);
        } break;

        case SERVOS_COMP_THM_BS: {
            _servos_comp_output[_servos_comp_idx++] = _servos_comp(
                &_servos_comp_data->thm.bs,
                &_servos_comp_data->hnd);
        } break;

        case SERVOS_COMP_DONE: {
            complete = 1;
        }
    }

    return complete;
}

//------------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//------------------------------------------------------------------------------

uint8_t _servos_comp(joint_pos_t* joint1, joint_pos_t* joint2) {
    float ang = _servos_dot(joint1, joint2);
    ang /= _servos_mag(joint1);
    ang /= _servos_mag(joint2);
    ang = acosf(ang);
    if (ang < 0.0) ang *= -1; // brute force absolute value

    // ang is 0 to pi -> convert to 0 to 255 (-90 to 90)
    return (255 * ang) / M_PI;
}

float _servos_dot(joint_pos_t* joint1, joint_pos_t* joint2) {
    float dot = 0;
    for (uint8_t x = 0; x < 3; x++) {
        dot += ((int8_t)(joint1->bytes[2*x+1])*(int8_t)(joint2->bytes[2*x+1]));
    }
    return dot;
}

float _servos_mag(joint_pos_t* joint) {
    return sqrtf(_servos_dot(joint, joint));
}

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
                    i2c_write(data+205);
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
