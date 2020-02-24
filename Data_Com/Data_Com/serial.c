//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "sam.h"
#include "serial.h"
#include <stddef.h>

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

#define SERIAL_TX (PORT_PB22)
#define SERIAL_TX_GROUP (1)
#define SERIAL_TX_PIN (PIN_PB22%32)
#define SERIAL_TX_PMUX (SERIAL_TX_PIN/2)

#define SERIAL_RX (PORT_PB23)
#define SERIAL_RX_GROUP (1)
#define SERIAL_RX_PIN (PIN_PB23%32)
#define SERIAL_RX_PMUX (SERIAL_RX_PIN/2)

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

serial_tx_callback _tx_callback;
serial_rx_callback _rx_callback;

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

void serial_init(uint32_t baudrate)
{
	// Turn on transmitter and reciever

	// TX (Pad0)
	#if (SERIAL_TX_PIN % 2) // Odd Pin
	PORT->Group[SERIAL_TX_GROUP].PMUX[SERIAL_TX_PMUX].bit.PMUXO = PORT_PMUX_PMUXO_D_Val;
	#else                  // Even Pin
	PORT->Group[SERIAL_TX_GROUP].PMUX[SERIAL_TX_PMUX].bit.PMUXE = PORT_PMUX_PMUXE_D_Val;
	#endif
	// Enable the PMUX
	PORT->Group[SERIAL_TX_GROUP].PINCFG[SERIAL_TX_PIN].bit.PMUXEN = 1;

	// RX (Pad 1)
	#if (SERIAL_RX_PIN % 2) // Odd Pin
	PORT->Group[SERIAL_RX_GROUP].PMUX[SERIAL_RX_PMUX].bit.PMUXO = PORT_PMUX_PMUXO_D_Val;
	#else                  // Even Pin
	PORT->Group[SERIAL_RX_GROUP].PMUX[SERIAL_RX_PMUX].bit.PMUXE = PORT_PMUX_PMUXE_D_Val;
	#endif
	// Enable the PMUX
	PORT->Group[SERIAL_RX_GROUP].PINCFG[SERIAL_RX_PIN].bit.PMUXEN = 1;

	// Enable the power manager
	PM->APBCMASK.reg |= PM_APBCMASK_SERCOM5;

	// Initialize the GCLK
	// Setting clock for the SERCOM5_CORE clock
	GCLK->CLKCTRL.reg = (GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM5_CORE) |
		GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN);
	// Wait for the GCLK to be synchronized
	while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

	// Reset the USART and wait for synch
	SERCOM5->USART.CTRLA.bit.SWRST = 1;
	while (SERCOM5->USART.CTRLA.bit.SWRST || SERCOM5->USART.SYNCBUSY.bit.SWRST);

	// Set the USART controls
	SERCOM5->USART.CTRLA.bit.MODE   = 1; // Internal clock
	SERCOM5->USART.CTRLA.bit.CMODE  = 0; // Asynchronous
	SERCOM5->USART.CTRLA.bit.RXPO   = 3; // Pad 3
	SERCOM5->USART.CTRLA.bit.TXPO   = 1; // Pad 2
	SERCOM5->USART.CTRLA.bit.SAMPR  = 1; // 16x with fractional generation
	SERCOM5->USART.CTRLA.bit.DORD   = 1; // LSB first
	SERCOM5->USART.CTRLA.bit.FORM   = 0; // USART Frame - No Parity

	SERCOM5->USART.CTRLB.bit.CHSIZE = 0; // 8 bit
	SERCOM5->USART.CTRLB.bit.SBMODE = 0; // 1 Stop bit
	SERCOM5->USART.CTRLB.bit.RXEN   = 1;
	SERCOM5->USART.CTRLB.bit.TXEN   = 1;

	// Calculate the baud rate
    // Asynchronous fractional mode (Table 24-2 in datasheet)
    //   BAUD = fref / (sampleRateValue * fbaud)
    // (multiply by 8, to calculate fractional piece)
    uint32_t baudTimes8 = (48000000ul * 8.0) / (16 * baudrate);
    SERCOM5->USART.BAUD.FRAC.FP   = (baudTimes8 % 8);
    SERCOM5->USART.BAUD.FRAC.BAUD = (baudTimes8 / 8);
	
	// Enable interrupts
	SERCOM5->USART.INTENSET.bit.TXC = 1;
	NVIC_EnableIRQ(SERCOM5_IRQn);
	NVIC_SetPriority(SERCOM5_IRQn, 3);

	// Enable the UART and sync
	SERCOM5->USART.CTRLA.bit.ENABLE = 1;
	while (SERCOM5->USART.SYNCBUSY.bit.ENABLE);

	// Initialize the transfer callback to null
	_tx_callback = (serial_tx_callback) 0;
}

//==============================================================================
uint8_t serial_available()
{
	return SERCOM5->USART.INTFLAG.bit.RXC;
}

//==============================================================================
uint8_t serial_read()
{
	return SERCOM5->USART.DATA.bit.DATA;
}

//==============================================================================
void serial_write(uint8_t data)
{
	// Wait for buffer to be empty
	while (! SERCOM5->USART.INTFLAG.bit.DRE);
	SERCOM5->USART.DATA.bit.DATA = data;
}

uint8_t serial_register_tx(serial_tx_callback tx_func)
{
	if (*(_tx_callback) == 0) {
		return _tx_callback = tx_func;
	} else {
		return 0;
	}
}

uint8_t serial_unregister_tx(serial_tx_callback tx_func)
{
	if (_tx_callback == tx_func) {
		_tx_callback = NULL;
		return 1;
	} else {
		return 0;
	}
}

uint8_t serial_register_rx(serial_rx_callback rx_func)
{
	if (!_rx_callback) {
		return _rx_callback = rx_func;
	} else {
		return 0;
	}
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

//------------------------------------------------------------------------------
//        __   __  , __
//     | /__` |__)  /__`
//     | .__/ |  \  .__/
//
//------------------------------------------------------------------------------

void SERCOM5_Handler()
{
	if (SERCOM5->USART.INTFLAG.bit.TXC) {
		// Clear the TX flag and call callback
		SERCOM5->USART.INTFLAG.bit.TXC = 1;
		_tx_callback();
	} else if (SERCOM5->USART.INTFLAG.bit.RXC) {
		_rx_callback(SERCOM5->USART.DATA.bit.DATA);
	}
}
