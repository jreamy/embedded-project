//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include <sam.h>
#include "adc.h"

//-----------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//-----------------------------------------------------------------------------

#define ADC_READ_1 (1)
#define ADC_READ_2 (2)

//------------------------------------------------------------------------------
//                __          __        ___  __
//     \  /  /\  |__) |  /\  |__) |    |__  /__`
//      \/  /~~\ |  \ | /~~\ |__) |___ |___ .__/
//
//------------------------------------------------------------------------------

adc_callback_t _adc_callback;
adc_callback_t _adc_default;

uint8_t _adc_read_state;

//-----------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//-----------------------------------------------------------------------------

uint8_t _adc_default_func(uint16_t);

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//==============================================================================
void adc_init()
{
	// Enable power ADC in manager
	PM->APBCMASK.bit.ADC_ = 1;

	// Enable GCLK for ADC
	GCLK->CLKCTRL.reg = (GCLK_CLKCTRL_ID_ADC | GCLK_CLKCTRL_CLKEN |
		GCLK_CLKCTRL_GEN_GCLK0);

	// =================================== //

	// Enable the ADC
	ADC->CTRLA.bit.ENABLE = 1;
	while (ADC->STATUS.bit.SYNCBUSY);

	// Set the reference
	ADC->REFCTRL.bit.REFSEL = 0x2;
	while (ADC->STATUS.bit.SYNCBUSY);

	// Cut INTVCC1 in half
	ADC->INPUTCTRL.bit.GAIN = 0x0;
	while (ADC->STATUS.bit.SYNCBUSY);

	// Pre-scalar 16, 12-bit resolution
	ADC->CTRLB.reg = (ADC_CTRLB_PRESCALER_DIV16 | ADC_CTRLB_RESSEL_12BIT);
	while (ADC->STATUS.bit.SYNCBUSY);

	// Set negative input to ground
	ADC->INPUTCTRL.bit.MUXNEG = 0x18;
	while (ADC->STATUS.bit.SYNCBUSY);

	// =================================== //

	// Enable interrupt for result ready
	ADC->INTENSET.bit.RESRDY = 1;
	while (ADC->STATUS.bit.SYNCBUSY);

	// Enable ADC interrupts
	NVIC_SetPriority(ADC_IRQn, 3);
	NVIC_EnableIRQ(ADC_IRQn);

	// Prep ADC
    _adc_read_state = ADC_READ_1;
	_adc_callback = _adc_default = _adc_default_func;
}


//==============================================================================
void adc_read(uint8_t pin) {
	// Set positive input to source
	ADC->INPUTCTRL.bit.MUXPOS = pin;
	while (ADC->STATUS.bit.SYNCBUSY);

	// Start conversion
	ADC->SWTRIG.bit.START = 1;
	while (ADC->STATUS.bit.SYNCBUSY);
}

uint8_t adc_register(adc_callback_t callback) {
    if (_adc_callback == _adc_default) {
        _adc_callback = callback;
        return 1;
    }
    return 0;
}

void adc_unregister(adc_callback_t callback) {
    if (_adc_callback == callback) {
        _adc_callback = _adc_default;
    }
}

void adc_set_default(adc_callback_t callback) {
    adc_register(callback);
    _adc_default = callback;
}

//-----------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//-----------------------------------------------------------------------------

//==============================================================================
uint8_t _adc_default_func(uint16_t data) {
    return 0;
}


//------------------------------------------------------------------------------
//        __   __  , __
//     | /__` |__)  /__`
//     | .__/ |  \  .__/
//
//------------------------------------------------------------------------------

//==============================================================================
void ADC_Handler() {
	// First read state (will be thrown out)
	if (_adc_read_state == ADC_READ_1)
	{
		// Read the value to clear the ADC flag
		while (ADC->STATUS.bit.SYNCBUSY);
		ADC->RESULT.reg;
		while (ADC->STATUS.bit.SYNCBUSY);

		// Start conversion
		ADC->SWTRIG.bit.START = 1;
		while (ADC->STATUS.bit.SYNCBUSY);

		// Update ADC status
		_adc_read_state = ADC_READ_2;
	}

	// Second read will be processed
	else if (_adc_read_state == ADC_READ_2)
	{
		// Handle the result
		while (ADC->STATUS.bit.SYNCBUSY);
		if (_adc_callback(ADC->RESULT.reg)) {
            adc_unregister(_adc_callback);
        }
		while (ADC->STATUS.bit.SYNCBUSY);

		// Clear the ADC
		ADC->SWTRIG.bit.FLUSH;
		while (ADC->STATUS.bit.SYNCBUSY);

		// Reset ADC status
		_adc_read_state = ADC_READ_1;
	}
}
