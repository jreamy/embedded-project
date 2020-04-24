//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "sam.h"
#include "timer.h"
#include "i2c.h"
#include "serial.h"
#include "LP5036.h"

#include <stddef.h>
#include <stdint.h>

//-----------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//-----------------------------------------------------------------------------

#define SERIAL_BAUD (1000000)
#define I2C_BAUD    (1000000)

//-----------------------------------------------------------------------------
//     ___      __   ___  __   ___  ___  __
//      |  \ / |__) |__  |  \ |__  |__  /__`
//      |   |  |    |___ |__/ |___ |    .__/
//
//-----------------------------------------------------------------------------

typedef enum {
    LED_UPDATE,
    LED_TX,
    LED_TX_END,
    LED_NUM_STATES
} led_state_t;

//-----------------------------------------------------------------------------
//                __          __        ___  __
//     \  /  /\  |__) |  /\  |__) |    |__  /__`
//      \/  /~~\ |  \ | /~~\ |__) |___ |___ .__/
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//-----------------------------------------------------------------------------

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
    uint64_t previous_time = 0;
    uint64_t delay_time = 50;

    uint8_t led_state = LED_UPDATE;
    uint8_t led_idx = 0;
    led_t leds[LP5036_NUM_LEDS];
    
    SystemInit();
    timer_init();

    serial_init(SERIAL_BAUD);
    i2c_init(I2C_BAUD);

    // Initial LED values
    uint8_t vals[] = { 0, 20, 60, 140, 200, 100, 0, 0, 0, 0};

    // Load all the LEDs with default values
    for (uint8_t idx; idx < LP5036_NUM_LEDS; idx++) {
        leds[idx].R = vals[idx];
        leds[idx].G = vals[(idx+4)%12];
        leds[idx].B = vals[(idx+8)%12];
        leds[idx].brightness = 0xFF;
    }

    uint8_t led_phase = 0;

    LP5036_init(LP5036_ADDR_00);

    while (1) {
        current_time = timer_get();

        if (current_time - previous_time > delay_time) {
            switch (led_state) {
                case LED_UPDATE: {
                    // Change LED values
                    for (uint8_t idx = 0; idx < LP5036_NUM_LEDS; idx++) {
                        leds[idx].R = vals[((idx + led_phase) % 12)];
                        leds[idx].G = vals[((idx + led_phase + 4) % 12)];
                        leds[idx].B = vals[((idx + led_phase + 8) % 12)];
                        leds[idx].brightness = 0xFF;
                    }
                    led_phase = (led_phase + 1) % 12;
                    led_idx = 0;
                    led_state = LED_TX;
                } break;

                case LED_TX: {
                    // Send the LED data
                    if (LP5036_write(LP5036_ADDR_00, led_idx, &leds[led_idx])) {
                        led_state = LED_TX_END;
                        led_idx++;
                    }
                } break;

                case LED_TX_END: {
                    // Wait for LED data send complete
                    if (LP5036_write_complete()) {
                        if (led_idx >= LP5036_NUM_LEDS) {
                            previous_time = current_time;
                            led_state = LED_UPDATE;
                        } else {
                            led_state = LED_TX;
                        }
                    }
                } break;
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

//-----------------------------------------------------------------------------
//        __   __   __
//     | /__` |__) /__`
//     | .__/ |  \ .__/
//
//-----------------------------------------------------------------------------
