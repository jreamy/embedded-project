//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "serial.h"
#include <avr/io.h>
#include <avr/interrupt.h>

//------------------------------------------------------------------------------
//                __          __        ___  __
//     \  /  /\  |__) |  /\  |__) |    |__  /__`
//      \/  /~~\ |  \ | /~~\ |__) |___ |___ .__/
//
//------------------------------------------------------------------------------

static uint8_t _string[MAX_STRING_SIZE];
volatile uint8_t serial_transfer_state = TRANSFER_READY;
volatile uint8_t _str_idx = 0;
static uint8_t _str_len = 0;

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

void _serial_send_byte();

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//=============================================================================
// USART0 init
// Communication Parameters: 8 data, 1 stop, 0 parity
// USART0 Receiver: on
// USART0 Transmitter: On
// USART0 Mode: Asynchronous
// USART0 Baud Rate: given by baud_rate
//
// The serial_byte_receiver must be set before stable use.
//=============================================================================
void serial_init(uint16_t baud_rate)
{
	// Turn on transmitter and receiver
	UCSR0A = 0;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	
	// Enable 8 bit data
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	
	// Set baud rate
	UBRR0H = baud_rate >> 8;
	UBRR0L = baud_rate;
	
	// Enable interrupts
	UCSR0B |= (1 << 6) | (1 << 7);
}

//=============================================================================
// Waits until the serial com link is free and then writes the byte.
//  Returns 1 if transfer accepted, 0 if not.
//=============================================================================
uint8_t serial_write(uint8_t data)
{
	if (serial_transfer_state == TRANSFER_READY)
	{
		// Wait for empty transmit buffer
		while (!(UCSR0A & (1 << UDRE0))){
			;
		}
	
		serial_transfer_state = TRANSFER_COMPLETE;
	
		// Put data in the buffer
		UDR0 = data;
		
		return 1;
	}
	else
	{
		return 0;
	}
}

//=============================================================================
// Waits until there is data to return, then returns it.
//=============================================================================
uint8_t serial_read()
{	
	// Wait for empty transmit buffer
	while (!(UCSR0A & (1 << RXC0))){
		;
	}
	
	return UDR0;
}

//=============================================================================
// Copies the given string then prints it using interrupts.
//=============================================================================
uint8_t serial_print(uint8_t *string)
{
	uint8_t i = 0;
	uint8_t c = 0;
	
	if (serial_transfer_state == TRANSFER_READY)
	{
		// Iterate through the characters in the string until null (0)
		do {
			// Copy each byte from the input to the global storage
			c = *(string + i);
			if (c != 0) _string[i++] = c;
		} while (c != 0);
		
		// Set the iteration indices
		_str_len = i;
		_str_idx = 0;
		
		// Begin transfer and return 'accepted'
		serial_transfer_state = TRANSFER_BUSY;
		_serial_send_byte();
		return 1;
	}
	else
		return 0;
}

//------------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//------------------------------------------------------------------------------

//==============================================================================
//  Private method that sends the next byte.
//==============================================================================
void _serial_send_byte()
{
	UDR0 = _string[_str_idx++];
}

//------------------------------------------------------------------------------
//        __   __  , __
//     | /__` |__)  /__`
//     | .__/ |  \  .__/
//
//------------------------------------------------------------------------------

//==============================================================================
// Private method that continues to send bytes as long as there are more in
//  the buffer, then sets the flag to TRANSFER_READY.
//==============================================================================
ISR(USART_TX_vect)
{
	// If there are more bytes to send, keep sending
	if (serial_transfer_state == TRANSFER_BUSY) {
		// Send next byte
		if (_str_idx < _str_len)
			_serial_send_byte();
			
		// Change state if needed
		if (_str_idx >= _str_len)
			serial_transfer_state = TRANSFER_COMPLETE;
	
	// If the final transfer completes, reset flag
	} else if (serial_transfer_state == TRANSFER_COMPLETE)
		serial_transfer_state = TRANSFER_READY;
}

//==============================================================================
// Calls the set serial receiver each time a byte is received from serial.
//==============================================================================
ISR(USART_RX_vect)
{
	serial_byte_receiver(UDR0);
}
