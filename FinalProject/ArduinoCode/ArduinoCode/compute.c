//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "compute.h"
#include "serial.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

#define COMP_EXT    (0)
#define COMP_INT    (1)
#define COMP_RX     (2)
#define COMP_DONE   (3)

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

uint8_t _comp_mode;

volatile uint8_t _comp_state;
uint8_t _comp_idx;
uint8_t* _comp_output;

serial_t _comp_serial;

uint8_t _comp_intern_sample_idx;
uint8_t _comp_intern_sample_row;
comp_model_t _comp_models[NUM_JOINTS];
emg_data_t _comp_data[2][INTERNAL_COMP_SAMPLES];

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

uint8_t _comp_callback();
void _comp_internal();
uint8_t _comp_current_sample_row();
uint8_t _comp_current_model_row();
void _comp_toggle_row();

// Model declarations
uint8_t _comp_model_0(emg_data_t* data);
uint8_t _comp_model_1(emg_data_t* data);
uint8_t _comp_model_2(emg_data_t* data);
uint8_t _comp_model_3(emg_data_t* data);
uint8_t _comp_model_4(emg_data_t* data);
uint8_t _comp_model_5(emg_data_t* data);
uint8_t _comp_model_6(emg_data_t* data);
uint8_t _comp_model_7(emg_data_t* data);
uint8_t _comp_model_8(emg_data_t* data);
uint8_t _comp_model_9(emg_data_t* data);
uint8_t _comp_model_10(emg_data_t* data);
uint8_t _comp_model_11(emg_data_t* data);
uint8_t _comp_model_12(emg_data_t* data);
uint8_t _comp_model_13(emg_data_t* data);
uint8_t _comp_model_14(emg_data_t* data);

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//==============================================================================
void compute_init(uint8_t mode) {
    _comp_mode = mode;

    switch (mode) {
        case COMP_INTERNAL: {
            // Setup internal computation mode
            _comp_intern_sample_row = 0;
            _comp_intern_sample_idx = 0;

            // Register the model functions
            _comp_models[0] = _comp_model_0;
            _comp_models[1] = _comp_model_1;
            _comp_models[2] = _comp_model_2;
            _comp_models[3] = _comp_model_3;
            _comp_models[4] = _comp_model_4;
            _comp_models[5] = _comp_model_5;
            _comp_models[6] = _comp_model_6;
            _comp_models[7] = _comp_model_7;
            _comp_models[8] = _comp_model_8;
            _comp_models[9] = _comp_model_9;
            _comp_models[10] = _comp_model_10;
            _comp_models[11] = _comp_model_11;
            _comp_models[12] = _comp_model_12;
            _comp_models[13] = _comp_model_13;
            _comp_models[14] = _comp_model_14;
        } break;

        case COMP_EXTERNAL: {
            // Setup external computation mode
            _comp_serial.rx = _comp_callback;
            _comp_serial.tx = _comp_callback;
        } break;

        default: {

        } break;
    }
}

//==============================================================================
void compute_stop() {
    _comp_serial.rx = 0;
    _comp_serial.tx = 0;
}

//==============================================================================
uint8_t compute_begin(hand_ang_t* output) {
    switch (_comp_mode) {
        // Trigger external computation
        case COMP_EXTERNAL: {
            if (serial_register(_comp_serial)) {
                _comp_output = output;
                _comp_idx = 0;
                _comp_state = COMP_EXT;
                serial_write(EXTERNAL_COMP_COMMAND);
                return 1;
            }
        } break;

        // Register for internal computation
        case COMP_INTERNAL: {
            _comp_toggle_row();
            _comp_state = COMP_INT;
            _comp_output = output;
            _comp_idx = 0;
            return 1;
        }
    }

    return 0;
}

//==============================================================================
uint8_t compute_complete() {
    if (_comp_mode == COMP_INTERNAL) _comp_internal();
    return (_comp_state == COMP_DONE);
}

//==============================================================================
emg_data_t* compute_get_emg_ref() {
    return &_comp_data[_comp_current_sample_row()][_comp_intern_sample_idx++];
}

//------------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//------------------------------------------------------------------------------

//==============================================================================
void _comp_internal() {
    // Get the model for the particular joint
    comp_model_t model = _comp_models[_comp_idx];

    // Compute the joing angle from the emg data
    _comp_output[_comp_idx++] = model(
        _comp_data[_comp_current_model_row()]);

    // Update the state if complete
    if (_comp_idx >= NUM_JOINTS) {
        _comp_state = COMP_DONE;
    }
}

//==============================================================================
uint8_t _comp_current_sample_row() {
    return (1-_comp_intern_sample_row);
}

//==============================================================================
uint8_t _comp_current_model_row() {
    return _comp_intern_sample_row;
}

//==============================================================================
void _comp_toggle_row() {
    _comp_intern_sample_idx = 0;
    _comp_intern_sample_row = (1-_comp_intern_sample_row);
}

//------------------------------------------------------------------------------
//      __                  __        __        __
//     /  `  /\  |    |    |__)  /\  /  ` |__/ /__`
//     \__, /~~\ |___ |___ |__) /~~\ \__, |  \ .__/
//
//------------------------------------------------------------------------------

//==============================================================================
uint8_t _comp_callback() {
    switch (_comp_state) {
        case COMP_EXT: {
            // Do nothing on the initial tx callback
            _comp_state = COMP_RX;
        } break;

        case COMP_RX: {
            // Receive data
            _comp_output[_comp_idx++] = serial_read();
            if (_comp_idx >= NUM_JOINTS) {
                _comp_state = COMP_DONE;
                serial_unregister(_comp_serial);
            }
        } break;
        
        default: {
            serial_unregister(_comp_serial);
        } break;
    } // end switch

    return 0;
}

//------------------------------------------------------------------------------
//           __   __   ___       __ 
//     |\/| /  \ |  \ |__  |    /__` 
//     |  | \__/ |__/ |___ |___ .__/ 
// 
//------------------------------------------------------------------------------

//==============================================================================
uint8_t _comp_model_0(emg_data_t* data) {
    return 0;
}

//==============================================================================
uint8_t _comp_model_1(emg_data_t* data) {
    return 10;
}

//==============================================================================
uint8_t _comp_model_2(emg_data_t* data) {
    return 20;
}

//==============================================================================
uint8_t _comp_model_3(emg_data_t* data) {
    return 30;
}

//==============================================================================
uint8_t _comp_model_4(emg_data_t* data) {
    return 40;
}

//==============================================================================
uint8_t _comp_model_5(emg_data_t* data) {
    return 50;
}

//==============================================================================
uint8_t _comp_model_6(emg_data_t* data) {
    return 60;
}

//==============================================================================
uint8_t _comp_model_7(emg_data_t* data) {
    return 70;
}

//==============================================================================
uint8_t _comp_model_8(emg_data_t* data) {
    return 80;
}

//==============================================================================
uint8_t _comp_model_9(emg_data_t* data) {
    return 90;
}

//==============================================================================
uint8_t _comp_model_10(emg_data_t* data) {
    return 100;
}

//==============================================================================
uint8_t _comp_model_11(emg_data_t* data) {
    return 110;
}

//==============================================================================
uint8_t _comp_model_12(emg_data_t* data) {
    return 120;
}

//==============================================================================
uint8_t _comp_model_13(emg_data_t* data) {
    return 130;
}

//==============================================================================
uint8_t _comp_model_14(emg_data_t* data) {
    return 140;
}


