//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "sam.h"
#include "timer.h"
#include "serial.h"

//-----------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//     ___      __   ___  __   ___  ___  __
//      |  \ / |__) |__  |  \ |__  |__  /__`
//      |   |  |    |___ |__/ |___ |    .__/
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//                __          __        ___  __
//     \  /  /\  |__) |  /\  |__) |    |__  /__`
//      \/  /~~\ |  \ | /~~\ |__) |___ |___ .__/
//
//-----------------------------------------------------------------------------

uint8_t *tx_data;
uint8_t tx_length;

//-----------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//-----------------------------------------------------------------------------

uint8_t transmit(uint8_t command, uint8_t* data);

void tx_callback();
void rx_callback(uint8_t data);

//-----------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//-----------------------------------------------------------------------------

//=============================================================================
int main(void)
{
	uint64_t last_time = 0;
	uint64_t current_time = 0;
	uint8_t *data_string = "Text\n";
	uint8_t data_ints[] = {90, 85, 10};
	uint8_t other[] = {9, 50, 255, 98};
	
    /* Initialize the SAM system */
    SystemInit();
	timer_init();
	serial_init(115200);

    /* Replace with your application code */
    while (1)
    {
		transmit('t', data_string);
		last_time = timer_get();
		while (timer_get() < last_time + 500) {}
		
		transmit('i', data_ints);
		last_time = timer_get();
		while (timer_get() < last_time + 500) {}
    }
}

//-----------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//-----------------------------------------------------------------------------

uint8_t transmit(uint8_t command, uint8_t* data)
{
	if (serial_register_tx(tx_callback)) {
		switch (command) {
			case 't' : tx_length = strlen(data); break;
			case 'i' : tx_length = 3; break;
			default  : tx_length = 0; break; 
		}
		tx_data = data;
		serial_write(command);
		return 1;
	} else {
		return 0;
	}
}

void tx_callback()
{
	if (tx_length-- != 0){
		uint8_t data = *(tx_data++);
		serial_write(data);
	}
	else serial_unregister_tx(tx_callback);
}

void rx_callback(uint8_t data)
{
	
}

//-----------------------------------------------------------------------------
//        __   __   __
//     | /__` |__) /__`
//     | .__/ |  \ .__/
//
//-----------------------------------------------------------------------------
