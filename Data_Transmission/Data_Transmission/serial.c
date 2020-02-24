//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "sam.h"
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
	//////////////////////////////////////////////////////////////////////////////
	// Configure the PORTS for SERCOM5
	//////////////////////////////////////////////////////////////////////////////
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
	
	//////////////////////////////////////////////////////////////////////////////
	// Set up the PM and the GCLK
	//////////////////////////////////////////////////////////////////////////////
	PM->APBCMASK.reg |= PM_APBCMASK_SERCOM5;

	// Initialize the GCLK
	// Setting clock for the SERCOM5_CORE clock
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM5_CORE) |
	GCLK_CLKCTRL_GEN_GCLK0                                            |
	GCLK_CLKCTRL_CLKEN ;
	// Wait for the GCLK to be synchronized
	while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);
	
	//////////////////////////////////////////////////////////////////////////////
	// Reset the UART
	//////////////////////////////////////////////////////////////////////////////
	SERCOM5->USART.CTRLA.bit.SWRST = 1;
	while (SERCOM5->USART.CTRLA.bit.SWRST || SERCOM5->USART.SYNCBUSY.bit.SWRST)
	{
		// Wait for both bits to return to 0
	}
	
	//////////////////////////////////////////////////////////////////////////////
	// Set the Controls
	//////////////////////////////////////////////////////////////////////////////
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
	
    // Asynchronous fractional mode (Table 24-2 in datasheet)
    //   BAUD = fref / (sampleRateValue * fbaud)
    // (multiply by 8, to calculate fractional piece)
    uint32_t baudTimes8 = (48000000ul * 8.0) / (16 * baudrate);
    SERCOM5->USART.BAUD.FRAC.FP   = (baudTimes8 % 8);
    SERCOM5->USART.BAUD.FRAC.BAUD = (baudTimes8 / 8);

	// Enable the UART
	SERCOM5->USART.CTRLA.bit.ENABLE = 1;
	while (SERCOM5->USART.SYNCBUSY.bit.ENABLE)
	{
		;
	}
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
	while (! SERCOM5->USART.INTFLAG.bit.DRE)
	{
		// Wait for the buffer to be empty
	}
	SERCOM5->USART.DATA.bit.DATA = data;
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
