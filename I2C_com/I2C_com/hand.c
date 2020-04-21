//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "hand.h"
#include "timer.h"
#include "i2c.h"
#include "i2c_mux.h"
#include "kx003.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

#define HAND_PNT_TOP    (0)
#define HAND_PNT_TIP    (1)
#define HAND_PNT_MID    (2)
#define HAND_MID_PNT    (3)
#define HAND_PNT_BS     (4)
#define HAND_MID_BS     (5)
#define HAND_MID_TOP    (6)
#define HAND_MID_MID    (7)
#define HAND_MID_TIP    (8)
#define HAND_RNG_TOP    (9)
#define HAND_RNG_TIP    (10)
#define HAND_RNG_MID    (11)
#define HAND_RNG_PNK    (12)
#define HAND_RNG_BS     (13)
#define HAND_PNK_BS     (14)
#define HAND_PNK_TOP    (15)
#define HAND_PNK_MID    (16)
#define HAND_PNK_TIP    (17)
#define HAND_THM_TOP    (18)
#define HAND_THM_TIP    (19)
#define HAND_THM_MID    (20)
#define HAND_THM_HND    (21)
#define HAND_THM_BS     (22)
#define HAND_HND_BS     (23)
#define HAND_COMPLETE   ('z') //(24)

#define HAND_I2C_REQUEST    (0)
#define HAND_I2C_WAITING    (1)


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

uint8_t _hand_mode;
uint8_t _hand_idx;
uint8_t _hand_joint;
uint8_t _hand_i2c_stat;

hand_pos_t* _hand_output;

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------
 
inline void _hand_i2c_mux(uint8_t addr, uint8_t data);
inline void _hand_kx003(uint8_t addr, joint_pos_t* output);

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//==============================================================================
void hand_init() {
    // Init all kx003 units
    for (uint8_t idx = 1; idx; idx=idx<<1) {
        i2c_mux_init(I2C_MUX_ADDR, idx);
        kx003_init(0x1E);
        kx003_init(0x1C);
    }
}

//==============================================================================
uint8_t hand_read(hand_pos_t* output) {
    _hand_output = output;
    _hand_mode = HAND_PNT_TOP;
    _hand_i2c_stat = HAND_I2C_REQUEST;
    return 1;
}


//==============================================================================
uint8_t hand_read_complete() {
    uint8_t complete = 0;
    switch (_hand_mode) {
        case HAND_PNT_TOP: {
            _hand_i2c_mux(I2C_MUX_ADDR, MUX_PNT_TOP);
        } break;

        case HAND_PNT_TIP: {
            _hand_kx003(PNT_TIP_ADDR, &_hand_output->pnt.tip.bytes);
        } break;

        case HAND_PNT_MID: {
            _hand_kx003(PNT_MID_ADDR, &_hand_output->pnt.mid.bytes);
        } break;

        case HAND_MID_PNT: {
            _hand_i2c_mux(I2C_MUX_ADDR, MUX_PNT_MID);
        } break;

        case HAND_PNT_BS: {
            _hand_kx003(PNT_BS_ADDR, &_hand_output->pnt.bs.bytes);
        } break;

        case HAND_MID_BS: {
            _hand_kx003(MID_BS_ADDR, &_hand_output->mid.bs.bytes);
        } break;

        case HAND_MID_TOP: {
            _hand_i2c_mux(I2C_MUX_ADDR, MUX_MID_TOP);
        } break;

        case HAND_MID_MID: {
            _hand_kx003(MID_MID_ADDR, &_hand_output->mid.mid.bytes);
        } break;

        case HAND_MID_TIP: {
            _hand_kx003(MID_TIP_ADDR, &_hand_output->mid.tip.bytes);
        } break;

        case HAND_RNG_TOP: {
            _hand_i2c_mux(I2C_MUX_ADDR, MUX_RNG_TOP);
        } break;

        case HAND_RNG_TIP: {
            _hand_kx003(RNG_TIP_ADDR, &_hand_output->rng.tip.bytes);
        } break;

        case HAND_RNG_MID: {
            _hand_kx003(RNG_MID_ADDR, &_hand_output->rng.mid.bytes);
        } break;

        case HAND_RNG_PNK: {
            _hand_i2c_mux(I2C_MUX_ADDR, MUX_RNG_PNK);
        } break;

        case HAND_RNG_BS: {
            _hand_kx003(RNG_BS_ADDR, &_hand_output->rng.bs.bytes);
        } break;

        case HAND_PNK_BS: {
            _hand_kx003(PNK_BS_ADDR, &_hand_output->pnk.bs.bytes);
        } break;

        case HAND_PNK_TOP: {
            _hand_i2c_mux(I2C_MUX_ADDR, MUX_PNK_TOP);
        } break;

        case HAND_PNK_MID: {
            _hand_kx003(PNK_MID_ADDR, &_hand_output->pnk.mid.bytes);
        } break;

        case HAND_PNK_TIP: {
            _hand_kx003(PNK_TIP_ADDR, &_hand_output->pnk.tip.bytes);
        } break;

        case HAND_THM_TOP: {
            _hand_i2c_mux(I2C_MUX_ADDR, MUX_THM_TOP);
        } break;

        case HAND_THM_TIP: {
            _hand_kx003(THM_TIP_ADDR, &_hand_output->thm.tip.bytes);
        } break;

        case HAND_THM_MID: {
            _hand_kx003(THM_MID_ADDR, &_hand_output->thm.mid.bytes);
        } break;

        case HAND_THM_HND: {
            _hand_i2c_mux(I2C_MUX_ADDR, MUX_THM_HND);
        } break;

        case HAND_THM_BS: {
            _hand_kx003(THM_BS_ADDR, &_hand_output->thm.bs.bytes);
        } break;

        case HAND_HND_BS: {
            _hand_kx003(HND_BS_ADDR, &_hand_output->hnd.bytes);
        } break;

        case HAND_COMPLETE: {
            complete = 1;
        } break;
    }

    return complete;
}


//------------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//------------------------------------------------------------------------------

void _hand_i2c_mux(uint8_t addr, uint8_t data) {
    if (_hand_i2c_stat == HAND_I2C_REQUEST) {
        if (i2c_mux_write(addr, data)) {
            _hand_i2c_stat = HAND_I2C_WAITING;
        }
    } else if (_hand_i2c_stat == HAND_I2C_WAITING) {
        if (i2c_mux_write_complete()) {
            _hand_i2c_stat = HAND_I2C_REQUEST;
            _hand_mode++;
        }
    }
}

void _hand_kx003(uint8_t addr, joint_pos_t* output) {
    if (_hand_i2c_stat == HAND_I2C_REQUEST) {
        if (kx003_read(addr, KX003_XOUT_L, output->bytes, 6)) {
            _hand_i2c_stat = HAND_I2C_WAITING;
        }
    } else if (_hand_i2c_stat == HAND_I2C_WAITING) {
        if (kx003_read_complete()) {
            _hand_i2c_stat = HAND_I2C_REQUEST;
            _hand_mode++;
        }
    }
}

//------------------------------------------------------------------------------
//      __                  __        __        __
//     /  `  /\  |    |    |__)  /\  /  ` |__/ /__`
//     \__, /~~\ |___ |___ |__) /~~\ \__, |  \ .__/
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//        __   __  , __
//     | /__` |__)  /__`   
//     | .__/ |  \  .__/
//
//------------------------------------------------------------------------------
