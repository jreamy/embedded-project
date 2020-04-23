//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "sam.h"
#include "timer.h"
#include "serial.h"
#include "servos.h"
#include "compute.h"
#include "adc.h"
#include "emg.h"
#include "hand.h"

#include <stddef.h>
#include <stdint.h>

//-----------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//-----------------------------------------------------------------------------

#define SERIAL_BAUD (1000000)
#define I2C_BAUD (1000000)

//-----------------------------------------------------------------------------
//     ___      __   ___  __   ___  ___  __
//      |  \ / |__) |__  |  \ |__  |__  /__`
//      |   |  |    |___ |__/ |___ |    .__/
//
//-----------------------------------------------------------------------------

typedef enum {
    M_EMG_SAMPLE, M_HAND_SAMPLE, M_COMPUTE, M_CONTROL, 
    M_TIMEOUTS, M_NUM_STATES
} machine_state_t;

typedef enum {
    EMG_SAMPLE, EMG_SAMPLE_END,
    EMG_TX, EMG_TX_END,
    EMG_NUM_STATES
} emg_state_t;

typedef enum {
    HAND_SAMPLE, HAND_SAMPLE_END,
    HAND_ANGLE, HAND_ANGLE_END,
    HAND_TX_WAIT, HAND_TX, HAND_TX_END,
    HAND_NUM_STATES
} hand_state_t;

typedef enum {
    CONTROL_BEGIN, CONTROL_END,
    CONTROL_WAIT
} control_state_t;

typedef enum {
   COMPUTE_TX, COMPUTE_TX_END,
   COMPUTE_RX, 
   COMPUTE_BEGIN, COMPUTE_END
} compute_state_t;

typedef enum {
    MODE_TRAIN    = 'T',
    MODE_CONTROL  = 'C',
    MODE_MIRROR   = 'M',
    MODE_SOLO     = 'S',
    MODE_WAIT     = 'W'
} machine_mode_t;

//-----------------------------------------------------------------------------
//                __          __        ___  __
//     \  /  /\  |__) |  /\  |__) |    |__  /__`
//      \/  /~~\ |  \ | /~~\ |__) |___ |___ .__/
//
//-----------------------------------------------------------------------------

uint64_t machine_counters[M_NUM_STATES];
uint64_t machine_delays[M_NUM_STATES];

uint8_t *tx_data;
uint8_t tx_length;

volatile machine_mode_t machine_mode = MODE_WAIT;
volatile uint8_t mode_change_flag = 0;

uint8_t tx_emg_flag = 0;
uint8_t tx_hand_flag = 0;
uint8_t tx_comp_flag = 0;
uint8_t* tx_flag;
uint8_t tx_text_flag = 0;

//-----------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//-----------------------------------------------------------------------------

uint8_t transmit(uint8_t command, uint8_t* data);
uint8_t tx_callback();

uint8_t rx_mode_setter();
uint8_t rx_hand_pos();

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
    uint64_t num_samples = 0;
    char* message;

    emg_data_t emg_data;
    emg_data_t* emg_ref = &emg_data;
    hand_pos_t hand_data;
    hand_ang_t hand_ang;

    emg_state_t emg_state = EMG_SAMPLE;
    hand_state_t hand_state = HAND_SAMPLE;
    control_state_t control_state = CONTROL_BEGIN;
    compute_state_t compute_state = COMPUTE_BEGIN;

    for (machine_state_t x = 0; x < M_NUM_STATES; x++) {
        machine_counters[x] = 0;
    }

    machine_delays[M_EMG_SAMPLE] = 1;
    machine_delays[M_HAND_SAMPLE] = 20;
    machine_delays[M_COMPUTE] = 20;
    machine_delays[M_CONTROL] = 100;

    /* Initialize the SAM system */
    SystemInit();
    timer_init();

    serial_init(SERIAL_BAUD);
    serial_set_default_rx(rx_mode_setter);

    adc_init();

    /* Replace with your application code */
    while (1)
    {
        switch (machine_mode) {

            /******************************************
             *                                        *
             *      Main for fully autonomous run     *
             *                                        *
             ******************************************/

            case MODE_SOLO: {
                // Acknowledge the mode reset
                message = "beginning solo operation";
                serial_init(SERIAL_BAUD);
                timer_delay(500);
                serial_set_default_rx(rx_mode_setter);
                while (!transmit('t', message));

                // Enable i2c com
                i2c_init(I2C_BAUD);

                // Reset the emg sampling machine
                emg_state = EMG_SAMPLE;
                machine_counters[M_EMG_SAMPLE] = 0;
                machine_delays[M_EMG_SAMPLE] = 1;
                num_samples = 1;

                // Enable EMG hardware
                adc_init();
                emg_setup(1, 1, 1);

                // Reset the computation machine
                compute_state = COMPUTE_BEGIN;
                machine_counters[M_COMPUTE] = 0;
                machine_delays[M_COMPUTE] = INTERNAL_COMP_SAMPLES;

                // Init internal computation machine
                compute_init(COMP_INTERNAL);
                emg_ref = compute_get_emg_ref();

                // Reset the hand controller machine
                control_state = CONTROL_BEGIN;
                machine_counters[M_CONTROL] = 0;
                machine_delays[M_COMPUTE] = 50;

                // Enable prosthetic hardware
                servos_init(SERVOS_ADDR);

                // Reset the mode flag and enter the 'main' loop
                mode_change_flag = 0;
                while (!mode_change_flag) {
                    // Get the current time
                    current_time = timer_get();

                    /*
                     * EMG sampling machine
                     */
                    if (current_time - machine_counters[M_EMG_SAMPLE] >
                            machine_delays[M_EMG_SAMPLE]) {
                        switch (emg_state) {
                            case EMG_SAMPLE: {
                                // Begin EMG sample
                                if (emg_sample(emg_ref)) {
                                    emg_state = EMG_SAMPLE_END;
                                }
                            } break;

                            case EMG_SAMPLE_END: {
                                if (emg_sample_complete()) {
                                    emg_ref = compute_get_emg_ref();
                                    machine_counters[M_EMG_SAMPLE] += machine_delays[M_EMG_SAMPLE];
                                    emg_state = EMG_SAMPLE;
                                    num_samples++;
                                }
                            } break;
                        } // end switch
                    }

                    /*
                     * Computation machine
                     */
                    if (num_samples - machine_counters[M_COMPUTE] >
                            machine_delays[M_COMPUTE]) {
                        switch (compute_state) {
                            case COMPUTE_BEGIN: {
                                // Begin computation on emg data
                                if (compute_begin(&hand_ang)) {
                                    compute_state = COMPUTE_END;
                                }
                            } break;

                            case COMPUTE_END: {
                                // Continue computation until complete
                                if (compute_complete()) {
                                    machine_counters[M_COMPUTE] += machine_delays[M_COMPUTE];
                                    compute_state = COMPUTE_BEGIN;
                                }
                            } break;
                        } // end switch
                    }

                    /*
                     * Hand controlling machine
                     */
                    if (num_samples - machine_counters[M_CONTROL] >
                            machine_delays[M_CONTROL]) {
                        switch (control_state) {
                            case CONTROL_BEGIN: {
                                // Begin I2C control of hand
                                if (servos_write(SERVOS_ADDR, &hand_ang)) {
                                    control_state = CONTROL_END;
                                }
                            } break;

                            case CONTROL_END: {
                                // Wait for I2C control of hand to complete
                                if (servos_write_complete()) {
                                    machine_counters[M_CONTROL] += machine_delays[M_CONTROL];
                                    control_state = CONTROL_BEGIN;
                                }
                            }
                        } // end switch
                    }
                } // end solo-controlled loop

                // Reset i2c
                i2c_stop();
                i2c_init(I2C_BAUD);

                // Disable EMG hardware
                adc_stop();
                emg_stop();

                // Disable computation machine
                compute_stop();

                // Disable prosthetic hardware
                servos_stop(SERVOS_ADDR);

                // Disable i2c hardware com
                i2c_stop();

            } break;

            /******************************************
             *                                        *
             *    Main for computer controlled run    *
             *                                        *
             ******************************************/

            case MODE_CONTROL: {
                // Acknowledge the mode reset
                message = "beginning hand controller";
                serial_init(SERIAL_BAUD);
                timer_delay(500);
                serial_set_default_rx(rx_mode_setter);

                while (!transmit('t', message));

                // Enable i2c com
                i2c_init(I2C_BAUD);

                // Reset the emg sampling machine
                emg_state = EMG_SAMPLE;
                machine_counters[M_EMG_SAMPLE] = 0;
                machine_delays[M_EMG_SAMPLE] = 1;
                num_samples = 0;

                // Enable EMG hardware
                adc_init();
                emg_setup(1, 1, 1);

                // Reset the computer connection machine
                compute_state = COMPUTE_TX;
                machine_counters[M_COMPUTE] = 0;
                machine_delays[M_COMPUTE] = 50;

                // Init computation module
                compute_init(COMP_EXTERNAL);

                // Reset the prosthetic controlling machine
                control_state = CONTROL_BEGIN;
                machine_counters[M_CONTROL] = 0;
                machine_delays[M_CONTROL] = 50;

                // Enable prosthetic hardware
                servos_init(SERVOS_ADDR);

                // Reset the timeout values
                machine_counters[M_TIMEOUTS] = timer_get();
                machine_delays[M_TIMEOUTS] = 5000;

                // Reset the mode flag and enter the 'main' loop
                mode_change_flag = 0;
                while (!mode_change_flag) {
                    // Get the current time
                    current_time = timer_get();

                    /*
                     * EMG sampling machine
                     */
                    if (current_time - machine_counters[M_EMG_SAMPLE] >
                            machine_delays[M_EMG_SAMPLE]) {
                        switch (emg_state) {
                            case EMG_SAMPLE: {
                                // Begin emg sample
                                if (emg_sample(&emg_data)) {
                                    emg_state = EMG_SAMPLE_END;
                                }
                            } break;

                            case EMG_SAMPLE_END: {
                                // Wait for emg  sample to end
                                if (emg_sample_complete()) {
                                    emg_state = EMG_TX;
                                }
                            } break;

                            case EMG_TX: {
                                // Begin transmitting emg data
                                if (transmit('e', emg_data.bytes)) {
                                    emg_state = EMG_TX_END;
                                }
                            } break;

                            case EMG_TX_END: {
                                if (tx_emg_flag) {
                                    emg_state = EMG_SAMPLE;
                                    machine_counters[M_EMG_SAMPLE] += machine_delays[M_EMG_SAMPLE];
                                    num_samples++;
                                }
                            }
                        } // end switch
                    }

                    /*
                     * External computation machine
                     */
                    if (num_samples - machine_counters[M_COMPUTE] >
                            machine_delays[M_COMPUTE]) {
                        switch (compute_state) {
                            case COMPUTE_TX: {
                                // Initiate external computation
                                if (compute_begin(&hand_ang)) {
                                    compute_state = COMPUTE_TX_END;
                                }
                            } break;

                            case COMPUTE_TX_END: {
                                // Wait for external computation to be returned
                                if (compute_complete()) {
                                    machine_counters[M_COMPUTE] += machine_delays[M_COMPUTE];
                                    compute_state = COMPUTE_TX;
                                }
                            } break;
                        } // end switch
                    }

                    /*
                     * Prosthetic controlling machine
                     */
                    if (num_samples - machine_counters[M_CONTROL] > 
                            machine_delays[M_CONTROL]) {
                        switch (control_state) {
                            case CONTROL_BEGIN: {
                                // Begin I2C control of servos
                                if (servos_write(SERVOS_ADDR, &hand_ang)) {
                                    control_state = CONTROL_END;
                                }
                            } break;

                            case CONTROL_END: {
                                // Wait for end of I2C control of servos
                                if (servos_write_complete()) {
                                    machine_counters[M_CONTROL] += machine_delays[M_CONTROL];
                                    control_state = CONTROL_BEGIN;
                                }
                            } break;
                        } // end switch
                    }

                    // Timeout check for registered things
                    if (current_time - machine_counters[M_TIMEOUTS] >
                            machine_delays[M_TIMEOUTS]) {
                        // Call timeout functions
                        if (i2c_timeout() || serial_timeout()) {
                            machine_mode = MODE_WAIT;
                            mode_change_flag = 1;
                        }
                        machine_counters[M_TIMEOUTS] += machine_delays[M_TIMEOUTS];
                    }
                } // end external-control loop

                // Reset i2c
                i2c_stop();
                i2c_init(I2C_BAUD);

                // Disable EMG hardware
                adc_stop();
                emg_stop();

                // Disable computation machine
                compute_stop();

                // Disable prosthetic control hardware
                servos_stop(SERVOS_ADDR);

                // Disable i2c hardware com
                i2c_stop();
            } break;

            /******************************************
             *                                        *
             *    Main for mirroring hand position    *
             *                                        *
             ******************************************/

            case MODE_MIRROR: {
                // Acknowledge the mode reset
                serial_init(SERIAL_BAUD);
                i2c_init(I2C_BAUD);
                timer_delay(500);
                serial_set_default_rx(rx_mode_setter);
                message = "beginning hand mirror";
                while (!transmit('t', message));

                // Enable i2c com
                i2c_init(I2C_BAUD);

                // Reset the hand sampling machine
                hand_state = HAND_SAMPLE;
                machine_counters[M_HAND_SAMPLE] = 0;
                machine_delays[M_HAND_SAMPLE] = 50;
                num_samples = 0;

                // Enable hand sampling hardware
                hand_init();

                // Reset the hand controlling machine
                control_state = CONTROL_BEGIN;
                machine_counters[M_CONTROL] = 0;
                machine_delays[M_CONTROL] = 1;

                // Enable the prosthetic hardware
                servos_init(SERVOS_ADDR);

                // Reset the timeout values
                machine_counters[M_TIMEOUTS] = timer_get();
                machine_delays[M_TIMEOUTS] = 5000;

                // Reset the mode flag and enter the 'main' loop
                mode_change_flag = 0;
                while (!mode_change_flag) {
                    // Get the current time
                    current_time = timer_get();

                    /*
                     * Hand sampling machine
                     */
                     if (current_time - machine_counters[M_HAND_SAMPLE] > 
                            machine_delays[M_HAND_SAMPLE]) {
                        switch (hand_state) {
                            case HAND_SAMPLE: {
                                // Begin hand sample via I2C
                                if (hand_read(&hand_data)) {
                                    hand_state = HAND_SAMPLE_END;
                                }
                            } break;

                            case HAND_SAMPLE_END: {
                                // Wait for I2C hand sample end
                                if (hand_read_complete()) {
                                    hand_state = HAND_ANGLE;
                                }
                            } break;

                            case HAND_ANGLE: {
                                // Begin hand angle calculation
                                if (servos_compute(&hand_data, &hand_ang)) {
                                    hand_state = HAND_ANGLE_END;
                                }
                            } break;

                            case HAND_ANGLE_END: {
                                // Wait for hand angle computation to finish
                                if (servos_compute_complete()) {
                                    hand_state = HAND_SAMPLE;
                                    machine_counters[M_HAND_SAMPLE] += machine_delays[M_HAND_SAMPLE];
                                    num_samples++;
                                }
                            } break;
                        } // end switch
                    }

                    /*
                     * Prosthetic controlling machine
                     */
                    if (num_samples - machine_counters[M_CONTROL] >
                            machine_delays[M_CONTROL]) {
                        switch (control_state) {
                            case CONTROL_BEGIN: {
                                // Begin I2C control of servos
                                if (servos_write(SERVOS_ADDR, &hand_ang)) {
                                    control_state = CONTROL_END;
                                }
                            } break;

                            case CONTROL_END: {
                                // Wait for I2C control of servos to finish
                                if (servos_write_complete()) {
                                    control_state = CONTROL_BEGIN;
                                    machine_counters[M_CONTROL] += machine_delays[M_CONTROL];
                                }
                            } break;
                        } // end switch
                    }

                    // Timeout check for registered things
                    if (current_time - machine_counters[M_TIMEOUTS] >
                            machine_delays[M_TIMEOUTS]) {
                        // Call timeout functions
                        if (i2c_timeout() || serial_timeout()) {
                            machine_mode = MODE_WAIT;
                            mode_change_flag = 1;
                        }
                        machine_counters[M_TIMEOUTS] += machine_delays[M_TIMEOUTS];
                    }
                    
                } // end mirroring loop

                // Reset i2c
                i2c_stop();
                i2c_init(I2C_BAUD);

                // Disable hand sampling hardware
                hand_stop();

                // Disable prosthetic hardware
                servos_stop(SERVOS_ADDR);

                // Disable i2c hardware com
                i2c_stop();
            } break;

            /******************************************
             *                                        *
             *    Main for collecting training data   *
             *                                        *
             ******************************************/

            case MODE_TRAIN: {
                // Acknowledge the mode reset
                message = "beginning data collection";
                serial_init(SERIAL_BAUD);
                timer_delay(500);
                serial_set_default_rx(rx_mode_setter);
                while (!transmit('t', message));

                // Enable i2c com
                i2c_init(I2C_BAUD);

                // Reset the EMG machine
                num_samples = 0;
                emg_state = EMG_SAMPLE;
                machine_counters[M_EMG_SAMPLE] = 0;
                machine_delays[M_EMG_SAMPLE] = 1;

                // Enable EMG hardware
                adc_init();
                emg_setup(1, 1, 1);

                // Reset the hand sampling machine
                hand_state = HAND_SAMPLE;
                machine_counters[M_HAND_SAMPLE] = 0;
                machine_delays[M_HAND_SAMPLE] = 20;

                // Enable hand sampling hardware
                hand_init();

                // Reset the timeout values
                machine_counters[M_TIMEOUTS] = timer_get();
                machine_delays[M_TIMEOUTS] = 5000;

                // Reset the mode flag and enter 'main' loop
                mode_change_flag = 0;
                while (!mode_change_flag) {
                    // Get the current time
                    current_time = timer_get();

                    /*
                     * EMG sampling machine
                     */
                    if (current_time - machine_counters[M_EMG_SAMPLE] >
                            machine_delays[M_EMG_SAMPLE]) {
                        switch (emg_state) {
                            case EMG_SAMPLE: {
                                // Begin the EMG sample
                                if (emg_sample(&emg_data)) {
                                    emg_state = EMG_SAMPLE_END;
                                }
                            } break;

                            case EMG_SAMPLE_END: {
                                // Wait for the EMG sample to complete
                                if (emg_sample_complete()) {
                                    emg_state = EMG_TX;
                                }
                            } break;

                            case EMG_TX: {
                                // Transmit the EMG data
                                if (transmit('e', emg_data.bytes)) {
                                    emg_state = EMG_TX_END;
                                }
                            } break;

                            case EMG_TX_END: {
                                // Wait for transmission complete
                                if (tx_emg_flag) {
                                    emg_state = EMG_SAMPLE;
                                    machine_counters[M_EMG_SAMPLE] += machine_delays[M_EMG_SAMPLE];
                                    num_samples++;
                                }
                            } break;
                        } // end switch
                    }

                    /*
                     * Hand sampling machine
                     */
                    if (num_samples - machine_counters[M_HAND_SAMPLE] > 
                            machine_delays[M_HAND_SAMPLE]) {

                        switch (hand_state) {
                            case HAND_SAMPLE: {
                                // Begin hand sample via I2C
                                if (hand_read(&hand_data)) {
                                    hand_state = HAND_SAMPLE_END;
                                }
                            } break;

                            case HAND_SAMPLE_END: {
                                // Wait for I2C hand sample end
                                if (hand_read_complete()) {
                                    hand_state = HAND_TX;
                                }
                            } break;

                            case HAND_TX: {
                                // Begin transmitting hand data
                                if (transmit('h', hand_data.bytes)) {
                                    hand_state = HAND_TX_END;
                                }
                            } break;

                            case HAND_TX_END: {
                                // Wait for transmission complete
                                if (tx_hand_flag) {
                                    hand_state = HAND_SAMPLE;
                                    machine_counters[M_HAND_SAMPLE] += machine_delays[M_HAND_SAMPLE];
                                }
                            } break;
                        } // end switch
                    }

                    // Timeout check for registered things
                    if (current_time - machine_counters[M_TIMEOUTS] >
                            machine_delays[M_TIMEOUTS]) {
                        // Call timeout functions
                        if (i2c_timeout() || serial_timeout()) {
                            machine_mode = MODE_WAIT;
                            mode_change_flag = 1;
                        }
                        machine_counters[M_TIMEOUTS] += machine_delays[M_TIMEOUTS];
                    }

                } // end training loop

                // Reset i2c
                i2c_stop();
                i2c_init(I2C_BAUD);

                // Disable hand sampling hardware
                hand_stop();

                // Disable EMG hardware
                adc_stop();
                emg_stop();

                // Disable i2c hardware com
                i2c_stop();
            } break;

            default: {
                mode_change_flag = 0;
                machine_mode = MODE_WAIT;
                while(!mode_change_flag) {
                    // Do nothing... wait for instruction
                }
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
            case 't' : {
                tx_length = strlen(data)+1;
                tx_flag = &tx_text_flag;
                tx_text_flag = 0;
            } break;
            
            case 'e' : {
                tx_length = 6; 
                tx_flag = &tx_emg_flag; 
                tx_emg_flag = 0;
            } break;

            case 'h' : {
                tx_length = 6*16;
                tx_flag = &tx_hand_flag;
                tx_hand_flag = 0;
            } break;

            default  : tx_length = 0; break;
        }
        tx_data = data;
        serial_write(command);
        return 1;
    } else {
        return 0;
    }
}

uint8_t tx_callback()
{
    if (tx_length-- != 0){
        uint8_t data = *(tx_data++);
        serial_write(data);
        return 0;
    }
    else {
        (*tx_flag) = 1;
        return 1;
    }
}

uint8_t rx_mode_setter() {
    // Signal that the mode changed!
    mode_change_flag = 1;

    // Sets the machine mode upon receiving a command char
    //  Defaults to sampling
    switch (serial_read()) {
        case MODE_CONTROL: {
            machine_mode = MODE_CONTROL;
        } break;

        case MODE_MIRROR: {
            machine_mode = MODE_MIRROR;
        } break;

        case MODE_TRAIN: {
            machine_mode = MODE_TRAIN;
        } break;

        case MODE_SOLO: {
            machine_mode = MODE_SOLO;
        } break;

        default: {
            machine_mode = MODE_WAIT;
        } break;
    }

    return 1;
}

//-----------------------------------------------------------------------------
//        __   __   __
//     | /__` |__) /__`
//     | .__/ |  \ .__/
//
//-----------------------------------------------------------------------------
