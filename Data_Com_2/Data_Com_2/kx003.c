//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "kx003.h"
#include "i2c.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

#define KX003_CTRL_PC1  (1<<7)
#define KX003_CTRL_RES  (1<<6)
#define KX003_CTRL_DRD  (1<<5)  
#define KX003_CTRL_GS1  (1<<4)
#define KX003_CTRL_GS0  (1<<3)
#define KX003_CTRL_E16  (1<<2)
#define KX003_CTRL_WUF  (1<<1)

#define KX003_INIT      (0)
#define KX003_TX_CLEAR  (1)
#define KX003_REG       (2)
#define KX003_RX_START  (3)
#define KX003_RX        (4)
#define KX003_RX_END    (5)

#define KX003_STOP      (0)

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

uint8_t _kx003_addr;
uint8_t _kx003_reg;
uint8_t* _kx003_dest;
uint8_t _kx003_idx;
volatile uint8_t _kx003_bytes;
volatile uint8_t _kx003_mode;

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

uint8_t _kx003_init_callback();
uint8_t _kx003_stop_callback();
uint8_t _kx003_read_callback();

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//==============================================================================
void kx003_init(uint8_t addr) {
    // First turn off the kx003
    kx003_stop(addr);

    // Set the callback mode
    _kx003_mode = KX003_INIT;
    
    // Wait and send the initial message
    while (i2c_registered());
    while (!i2c_register(_kx003_init_callback));
    _kx003_idx = 0;
    i2c_begin_write(addr);

    while (i2c_registered());

    // Reset state machine
    _kx003_reg = 0;
    _kx003_mode = KX003_INIT;
    _kx003_addr = addr;
    _kx003_idx = 0;
}

//==============================================================================
void kx003_stop(uint8_t addr) {
    // Set the callback mode
    _kx003_mode = KX003_STOP;
    
    // Wait and send the initial message
    while (i2c_registered());
    while (!i2c_register(_kx003_stop_callback));
    _kx003_idx = 0;
    i2c_begin_write(addr);

    while (i2c_registered());

    // Reset state machine
    _kx003_reg = 0;
    _kx003_mode = KX003_STOP;
    _kx003_addr = addr;
    _kx003_idx = 0;
}

//==============================================================================
uint8_t kx003_read(uint8_t addr, uint8_t reg, uint8_t* dest, uint8_t bytes) {
    if (bytes && i2c_register(_kx003_read_callback)) {
        _kx003_reg = reg;
        _kx003_bytes = bytes;
        _kx003_mode = KX003_REG;
        _kx003_addr = addr;
        _kx003_dest = dest;
        _kx003_idx = 0;
        i2c_begin_write(addr);
        return 1;
    }
    return 0;
}

//==============================================================================
uint8_t kx003_read_complete() {
    return (_kx003_mode == KX003_RX_END);
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
uint8_t _kx003_init_callback() {
    uint8_t complete = 0;

    switch (_kx003_mode) {
        // Initialization callback
        case KX003_INIT: {
            if (!i2c_tx_flag()) break;
            _kx003_mode = KX003_CTRL_REG;
            i2c_write(KX003_CTRL_REG);
        } break;

        case KX003_CTRL_REG: {
            if (!i2c_tx_flag()) break;
            _kx003_mode = KX003_TX_CLEAR;
            i2c_write(KX003_CTRL_PC1 | KX003_CTRL_RES | KX003_CTRL_GS0);
        } break;

        case KX003_TX_CLEAR: {
            if (!i2c_tx_flag()) break;
            i2c_tx_stop();
            complete = 1;
        } break;
    }

    return complete;
}

//==============================================================================
uint8_t _kx003_stop_callback() {
    uint8_t complete = 0;

    switch (_kx003_mode) {
        // Initialization callback
        case KX003_INIT: {
            if (!i2c_tx_flag()) break;
            i2c_write(KX003_CTRL_REG);
            _kx003_mode = KX003_CTRL_REG;
        } break;

        case KX003_CTRL_REG: {
            if (!i2c_tx_flag()) break;
            i2c_write(0x00);
            _kx003_mode = KX003_TX_CLEAR;
        } break;

        case KX003_TX_CLEAR: {
            if (!i2c_tx_flag()) break;
            i2c_tx_stop();
            complete = 1;
        } break;
    }

    return complete;
}

//==============================================================================
uint8_t _kx003_read_callback() {
    uint8_t complete = 0;

    switch (_kx003_mode) {
        // Write which register to read
        case KX003_REG: {
            if (!i2c_tx_flag()) break;
            _kx003_mode = KX003_TX_CLEAR;
            i2c_write(_kx003_reg);
        } break;

        // Remove this
        case KX003_TX_CLEAR: {
            if (!i2c_tx_flag()) break;
            _kx003_mode = KX003_RX_START;
            serial_write(TX_E);
        } break;

        // Begin the read
        case KX003_RX_START: {
            if (!i2c_tx_flag()) break;
            _kx003_mode = KX003_RX;
            i2c_begin_read(_kx003_addr);
        } break;

        // Continue reading
        case KX003_RX: {
            if (!i2c_rx_flag()) break;
            if (_kx003_idx < _kx003_bytes-1) {
                uint8_t data = i2c_read();
                _kx003_dest[_kx003_idx++] = data;
                break;
            }
            _kx003_mode = KX003_RX_END;
        } // Allow passing when enough bytes rx 

        // End reading
        case KX003_RX_END: {
            if (!i2c_rx_flag()) break;
            i2c_rx_stop();
            _kx003_dest[_kx003_idx] = i2c_read();
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
