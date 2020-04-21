//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "sam.h"
#include "buttons.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

#define BUTTON_0 (PORT_PB09)
#define BUTTON_0_GROUP (1)
#define BUTTON_0_PIN (PIN_PB09 % 32)

#define BUTTON_1 (PORT_PA04)
#define BUTTON_1_GROUP (0)
#define BUTTON_1_PIN (PIN_PA04 % 32)

#define BTN_ACTIVE (PORT_PA08)
#define BTN_ACTIVE_GROUP (0)
#define BTN_ACTIVE_PIN (PIN_PA08 % 32)

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//==============================================================================
void buttons_init()
{
	// Enable the pins as inputs
	PORT->Group[BUTTON_0_GROUP].PINCFG[BUTTON_0_PIN].bit.INEN = 1;
	PORT->Group[BUTTON_1_GROUP].PINCFG[BUTTON_1_PIN].bit.INEN = 1;
	PORT->Group[BTN_ACTIVE_GROUP].PINCFG[BTN_ACTIVE_PIN].bit.INEN = 1;

	// Clear the pins
	PORT->Group[BUTTON_0_GROUP].DIRCLR.reg = BUTTON_0;
	PORT->Group[BUTTON_1_GROUP].DIRCLR.reg = BUTTON_1;
	PORT->Group[BTN_ACTIVE_GROUP].DIRCLR.reg = BTN_ACTIVE;
}


//==============================================================================
uint8_t buttons_get(uint8_t button)
{
	// Read the button pins
	uint16_t button_0 = PORT->Group[BUTTON_0_GROUP].IN.reg;
	uint16_t button_1 = PORT->Group[BUTTON_1_GROUP].IN.reg;
	uint16_t button_a = PORT->Group[BTN_ACTIVE_GROUP].IN.reg;

	// Convert pin reading to button id
	button_a = ((button_a & (BTN_ACTIVE)) != 0);
	button_0 = ((button_0 & (BUTTON_0)) != 0);
	button_1 = ((button_1 & (BUTTON_1)) != 0);
	button_0 += (button_1 << 1);

	// Return true if correct button id and buttons active
	return (button_0 == button) && button_a;
}

//==============================================================================
uint8_t buttons_active()
{
	// Read the button active pin
	uint16_t button_a = PORT->Group[BTN_ACTIVE_GROUP].IN.reg;
	
	// Return true if a button is active
	return ((button_a & (BTN_ACTIVE)) != 0);
}

//==============================================================================
uint8_t buttons_get_button()
{
	uint16_t button_0 = PORT->Group[BUTTON_0_GROUP].IN.reg;
	uint16_t button_1 = PORT->Group[BUTTON_1_GROUP].IN.reg;

	// Convert pin reading to button id
	button_0 = ((button_0 & (BUTTON_0)) != 0);
	button_1 = ((button_1 & (BUTTON_1)) != 0);
	return button_0 + (button_1 << 1);
}
