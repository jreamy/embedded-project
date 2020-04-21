//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "sam.h"
#include "jstick.h"
#include "adc.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

#define EMG_NUM_PINS (3)

// ADC things
#define JX_PORT (PORT_PA02)
#define JX_GROUP (0)
#define JX_PIN (PIN_PA02 % 32)
#define JX_ADC_PIN (0)

#define JY_PORT (PORT_PB08)
#define JY_GROUP (1)
#define JY_PIN (PIN_PB08 % 32)
#define JY_ADC_PIN (2)

#define A1_PORT (PORT_PA03)
#define A1_GROUP (0)
#define A1_PIN (PIN_PA03)
#define A1_ADC_PIN (1)

//------------------------------------------------------------------------------
//                __          __        ___  __
//     \  /  /\  |__) |  /\  |__) |    |__  /__`
//      \/  /~~\ |  \ | /~~\ |__) |___ |___ .__/
//
//------------------------------------------------------------------------------

uint8_t _emg_pins[] = {JY_PIN, JX_PIN, A1_PIN};
volatile uint16_t* _emg_data;
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
void emg_init()
{
	// =================================== //

	// Enable joystick x as input
	PORT->Group[JX_GROUP].PINCFG[JX_PIN].bit.INEN = 1;
	PORT->Group[JX_GROUP].DIRCLR.reg = JX_PORT;

	// Enable joystick y as input
	PORT->Group[JY_GROUP].PINCFG[JY_PIN].bit.INEN = 1;
	PORT->Group[JY_GROUP].DIRCLR.reg = JY_PORT;

    // Enable A1 as input
    PORT->Group[A1_GROUP].PINCFG[A1_PIN].bit.INEN = 1;
    PORT->Group[A1_GROUP].DIRCLR.reg = A1_PORT;
}


//==============================================================================
uint8_t emg_read(uint16_t* output) {
    if (adc_register(_emg_callback)) {
        _emg_data = output;
        _emg_idx = 0;
        adc_read(_emg_pins[_emg_idx]);
        return 1;
    }
    return 0;
}

//==============================================================================
uint8_t emg_read_complete() {
    return _emg_idx == EMG_NUM_PINS;
}


//------------------------------------------------------------------------------
//      __                  __        __        __
//     /  `  /\  |    |    |__)  /\  /  ` |__/ /__`
//     \__, /~~\ |___ |___ |__) /~~\ \__, |  \ .__/
//
//------------------------------------------------------------------------------

//==============================================================================
uint8_t _emg_callback(uint16_t data) {
	_emg_data[_emg_idx++] = data;
    if (_emg_idx < EMG_NUM_PINS) {
        adc_read(_emg_pins[_emg_idx]);
        return 0;
    }
    return 1;
}
