//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "emg.h"
#include "adc.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

#define CHANEL_1_PORT (PORT_PA02)
#define CHANEL_1_GROUP (0)
#define CHANEL_1_PIN (PIN_PA02 % 32)
#define CHANEL_1_ADC_PIN (0)

#define CHANEL_2_PORT (PORT_PB08)
#define CHANEL_2_GROUP (1)
#define CHANEL_2_PIN (PIN_PB08 % 32)
#define CHANEL_2_ADC_PIN (2)

#define CHANEL_3_PORT (PORT_PB09)
#define CHANEL_3_GROUP (1)
#define CHANEL_3_PIN (PIN_PB09 % 32)
#define CHANEL_3_ADC_PIN (3)

#define EMG_NUM_PINS (3)

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

emg_data_t* _emg_data;
uint8_t _emg_channel_active[3];
uint8_t _emg_channel_pins[] = {CHANEL_1_ADC_PIN, CHANEL_2_ADC_PIN, CHANEL_3_ADC_PIN};
volatile uint8_t _emg_idx;

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

uint8_t _emg_callback(uint16_t data);

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//==============================================================================
void emg_setup(uint8_t channel1, uint8_t channel2, uint8_t channel3) {
    // Save which channels are active
    _emg_channel_active[0] = channel1;
    _emg_channel_active[1] = channel2;
    _emg_channel_active[2] = channel3;

    // Enable channel 1 as input
    PORT->Group[CHANEL_1_GROUP].PINCFG[CHANEL_1_PIN].bit.INEN = 1;
    PORT->Group[CHANEL_1_GROUP].DIRCLR.reg = CHANEL_1_PORT;

    // Enable channel 2 as input
    PORT->Group[CHANEL_2_GROUP].PINCFG[CHANEL_2_PIN].bit.INEN = 1;
    PORT->Group[CHANEL_2_GROUP].DIRCLR.reg = CHANEL_2_PORT;

    // Enable channel 3 as input
    PORT->Group[CHANEL_3_GROUP].PINCFG[CHANEL_3_PIN].bit.INEN = 1;
    PORT->Group[CHANEL_3_GROUP].DIRCLR.reg = CHANEL_3_PORT;

    // Reset the index
    _emg_idx = 0;
}

//==============================================================================
void emg_stop() {
    _emg_idx = 0;
}

//==============================================================================
uint8_t emg_sample(emg_data_t* output)
{
    if (adc_register(_emg_callback)) {
        _emg_data = output;
        _emg_idx = 0;
        adc_read(_emg_channel_pins[_emg_idx]);
        return 1;
    }
    return 0;
}

//==============================================================================
uint8_t emg_sample_complete() {
    return _emg_idx == EMG_NUM_PINS;
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
uint8_t _emg_callback(uint16_t data) {
    // Store the value if the channel is active
    if (_emg_channel_active[_emg_idx]) {
        _emg_data->data[_emg_idx++] = data;
    }

    // Store 0 if the channel inactive
    else {
        _emg_data->data[_emg_idx++] = 0;
    }

    // Start the next adc read if appropriate
    if (_emg_idx < EMG_NUM_PINS) {
        adc_read(_emg_channel_pins[_emg_idx]);
        return 0;
    }

    // Otherwise free adc callback
    return 1;
}

//------------------------------------------------------------------------------
//        __   __  , __
//     | /__` |__)  /__`
//     | .__/ |  \  .__/
//
//------------------------------------------------------------------------------
