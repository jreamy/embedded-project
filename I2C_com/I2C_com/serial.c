//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "serial.h"

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

serial_t _serial_callback = {0, 0};
serial_t _serial_default = {0, 0};

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

uint8_t _serial_default_tx();
uint8_t _serial_default_rx();

uint8_t _serial_rx_registered();
uint8_t _serial_tx_registered();

void _serial_tx_clear();

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

void serial_init(uint32_t baudrate)
{
	// Turn on transmitter and receiver

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
	SERCOM5->USART.INTENSET.bit.RXC = 1;
	NVIC_EnableIRQ(SERCOM5_IRQn);
	NVIC_SetPriority(SERCOM5_IRQn, 3);

	// Enable the UART and sync
	SERCOM5->USART.CTRLA.bit.ENABLE = 1;
	while (SERCOM5->USART.SYNCBUSY.bit.ENABLE);

    // Set the defaults
    _serial_default.rx = _serial_default_rx;
    _serial_default.tx = _serial_default_tx;
    _serial_callback = _serial_default;
}

//==============================================================================
uint8_t serial_tx_flag()
{
    return SERCOM5->USART.INTFLAG.bit.TXC;
}

//==============================================================================
uint8_t serial_rx_flag()
{
	return SERCOM5->USART.INTFLAG.bit.RXC;
}

//==============================================================================
uint8_t serial_read()
{
    uint8_t data = SERCOM5->USART.DATA.bit.DATA;
    //serial_write(data);
    return data;
}

//==============================================================================
void serial_write(uint8_t data)
{
	// Wait for buffer to be empty
	while (! SERCOM5->USART.INTFLAG.bit.DRE);
	SERCOM5->USART.DATA.bit.DATA = data;
}

//==============================================================================
uint8_t serial_register(serial_t callback)
{
    if (callback.rx && callback.tx) {
        if (!_serial_rx_registered() && !_serial_tx_registered()) {
            _serial_callback = callback;
            return 1;
        }
    }

    else if (callback.rx) {
        return serial_register_rx(callback.rx);
    }

    else if (callback.tx) {
        return serial_register_tx(callback.tx);
    }

    return 0;
}

//==============================================================================
void serial_unregister(serial_t callback) {
    serial_unregister_rx(callback.rx);
    serial_unregister_tx(callback.tx);
}

//==============================================================================
void serial_set_default(serial_t callback) {
    _serial_default = callback;
}

//==============================================================================
uint8_t serial_register_rx(serial_callback_t callback) {
    if (!_serial_rx_registered()) {
        _serial_callback.rx = callback;
        return 1;
    }
    return 0;
}

//==============================================================================
void serial_unregister_rx(serial_callback_t callback) {
    if (_serial_callback.rx == callback) {
        _serial_callback.rx = _serial_default.rx;
    }
}

//==============================================================================
void serial_set_default_rx(serial_callback_t callback) {
    serial_register_rx(callback);
    _serial_default.rx = callback;
}

//==============================================================================
uint8_t serial_register_tx(serial_callback_t callback) {
    if (!_serial_tx_registered()) {
        _serial_callback.tx = callback;
        return 1;
    }

    return 0;
}

//==============================================================================
void serial_unregister_tx(serial_callback_t callback) {
    if (_serial_callback.tx == callback) {
        _serial_callback.tx = _serial_default.tx;
    }
}

//==============================================================================
void serial_set_default_tx(serial_callback_t callback) {
    serial_register_tx(callback);
    _serial_default.tx = callback;
}

//------------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//------------------------------------------------------------------------------

uint8_t _serial_rx_registered() {
    return (_serial_callback.rx != _serial_default.rx);
}

uint8_t _serial_tx_registered() {
    return (_serial_callback.tx != _serial_default.tx);
}

void _serial_tx_clear() {
    SERCOM5->USART.INTFLAG.bit.TXC = 1;
}

//------------------------------------------------------------------------------
//      __                  __        __        __
//     /  `  /\  |    |    |__)  /\  /  ` |__/ /__`
//     \__, /~~\ |___ |___ |__) /~~\ \__, |  \ .__/
//
//------------------------------------------------------------------------------

uint8_t _serial_default_tx()
{
    return 0;
}

uint8_t _serial_default_rx()
{
    serial_read();
    return 0;
}

//------------------------------------------------------------------------------
//        __   __  , __
//     | /__` |__)  /__`
//     | .__/ |  \  .__/
//
//------------------------------------------------------------------------------

void SERCOM5_Handler()
{
	if (serial_tx_flag()) {
		// Clear the TX flag and call callback
		if (_serial_callback.tx()) {
            _serial_callback.tx = _serial_default.tx;
        }

        _serial_tx_clear();
	} else if (serial_rx_flag()) {
		if (_serial_callback.rx()) {
            _serial_callback.rx = _serial_default.rx;
        }
	}
}
