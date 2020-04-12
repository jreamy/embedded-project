//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "sam.h"
#include "timer.h"
#include "serial.h"
#include "emg.h"

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

typedef enum {
	EMG_SAMPLE, HAND_SAMPLE, COMPUTE, CONTROL, NUM_STATES
} machine_state_t;

// Finger struct for holding finger position data
typedef union {
	struct {
		uint16_t base;
		uint16_t middle;
		uint16_t tip;
	};
	uint16_t data[3];
} finger_t;


// Hand struct for holding hand position data
typedef union {
	struct {
		struct {
			finger_t thumb;
			finger_t pointer;
			finger_t middle;
			finger_t ring;
			finger_t pinky;
		} fingers;
		uint16_t base;
	};
	
	uint16_t data[16];
	uint8_t bytes[32];
} hand_t;

//-----------------------------------------------------------------------------
//                __          __        ___  __
//     \  /  /\  |__) |  /\  |__) |    |__  /__`
//      \/  /~~\ |  \ | /~~\ |__) |___ |___ .__/
//
//-----------------------------------------------------------------------------

uint64_t previous_times[NUM_STATES];
uint64_t delay_times[NUM_STATES];

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
void rx_hand_data(uint8_t data);

hand_t* rx_hand;
uint8_t rx_hand_idx;
uint8_t rx_hand_flag;

//-----------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//-----------------------------------------------------------------------------

//=============================================================================
int main(void)
{ 
	uint64_t current_time = 0;
	uint8_t *data_string = "Text\n";
	uint8_t data_ints[] = {90, 85, 10};
	uint8_t other[] = {9, 50, 255, 98};
		
	hand_t current_hand = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	hand_t computed_hand;
		
	for (machine_state_t x = 0; x < NUM_STATES; x++) {
		previous_times[x] = 0;
	}
		
	delay_times[EMG_SAMPLE] = 1;
	delay_times[HAND_SAMPLE] = 5;
	delay_times[COMPUTE] = 20;
	delay_times[CONTROL] = 20;
	
    /* Initialize the SAM system */
    SystemInit();
	timer_init();
	serial_init(1000000);

    /* Replace with your application code */
    while (1)
    {
		current_time = timer_get();
		
		// Sample EMG
		if (current_time - previous_times[EMG_SAMPLE] > delay_times[EMG_SAMPLE]) {
			emg_sample();
		}
		
		// Sample hand position
		if (current_time - previous_times[HAND_SAMPLE] > delay_times[HAND_SAMPLE]) {
			if (transmit('h', current_hand.bytes)) {
				previous_times[HAND_SAMPLE] += delay_times[HAND_SAMPLE];
			}
		}
		
		// Transmit / receive or compute
		if (current_time - previous_times[COMPUTE] > delay_times[COMPUTE]) {
			
		}
		
		// Control
		if (current_time - previous_times[CONTROL] > delay_times[CONTROL]) {
			if (transmit('c', computed_hand.bytes)) {
				current_hand = computed_hand;
				previous_times[CONTROL] += delay_times[CONTROL];
			}
		}
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
			case 'h' : tx_length = 32;
				rx_hand_idx = 0;
				break;
			case 'c' : tx_length = 32; break;
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

void rx_hand_data(uint8_t data)
{
	rx_hand->bytes[rx_hand_idx++] = data;
	if (rx_hand_idx == 32) {
		serial_unregister_rx(rx_hand_data);
	}
}

//-----------------------------------------------------------------------------
//        __   __   __
//     | /__` |__) /__`
//     | .__/ |  \ .__/
//
//-----------------------------------------------------------------------------
