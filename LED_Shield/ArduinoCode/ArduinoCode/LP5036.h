#ifndef LP5036_H
#define LP5036_H

//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <sam.h>

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

// Device addresses
#define LP5036_ADDR_00  (0x30)
#define LP5036_ADDR_01  (0x31)
#define LP5036_ADDR_10  (0x32)
#define LP5036_ADDR_11  (0x33)
#define LP5036_ADDR_ALL (0x34)

// Device registers
#define LP5036_DEV_CFG0 (0x00)
#define LP5036_DEV_CFG0_CHIP_EN         (1<<6)

#define LP5036_DEV_CFG1 (0x01)
#define LP5036_DEV_CFG1_LOG_EN          (1<<5)
#define LP5036_DEV_CFG1_PWR_SAVE_EN     (1<<4)
#define LP5036_DEV_CFG1_AUTO_INC_EN     (1<<3)
#define LP5036_DEV_CFG1_DITHERING_EN    (1<<2)
#define LP5036_DEV_CFG1_MAX_CURRENT     (1<<1)
#define LP5036_DEV_CFG1_GLOBAL_OFF      (1<<0)

#define LP5036_LED_CFG0 (0x02)
#define LP5036_LED_CFG1 (0x03)
#define LP5036_LED_BANK_REG(LED_NUM) ((LED_NUM) < 8 ? LP3036_LED_CFG0 : LP3036_LED_CFG1)
#define LP5036_LED_BANK_VAL(LED_NUM) (1<<((LED_NUM) & 0x7))

#define LP5036_BANK_BRIGHTNESS  (0x04)
#define LP5036_BANK_A_COLOR     (0x05)
#define LP5036_BANK_B_COLOR     (0x06)
#define LP5036_BANK_C_COLOR     (0x07)
#define LP5036_LED_BRIGHT_REG(LED_NUM)  (0x08 + (LED_NUM))
#define LP5036_LED_COLOR(LED_NUM, RGB)  (0x14 + 3 * (LED_NUM) + ((RGB) & 0x3))
#define LP5036_RESET_REG        (0x38)
#define LP5036_RESET_VAL        (0xFF)

// LED number definitions
#define LED_0   (0)
#define LED_1   (1)
#define LED_2   (2)
#define LED_3   (3)
#define LED_4   (4)
#define LED_5   (5)
#define LED_6   (6)
#define LED_7   (7)
#define LED_8   (8)
#define LED_9   (9)
#define LED_10  (10)
#define LED_11  (11)
#define LED_12  (12)
#define BANK_1  (13)
#define BANK_2  (14)
#define BANK_3  (15)

// LED color channel definitions
#define LED_R   (0)
#define LED_G   (1)
#define LED_B   (2)

#define LP5036_NUM_LEDS (12)

//------------------------------------------------------------------------------
//     ___      __   ___  __   ___  ___  __
//      |  \ / |__) |__  |  \ |__  |__  /__`
//      |   |  |    |___ |__/ |___ |    .__/
//
//------------------------------------------------------------------------------

typedef union {
    struct {
        uint8_t R;
        uint8_t G;
        uint8_t B;
        uint8_t brightness;
    };

    uint8_t bytes[4];
} led_t;

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

void LP5036_init(uint8_t addr);
void LP5036_stop(uint8_t addr);

uint8_t LP5036_write(uint8_t addr, uint8_t led_num, led_t* led);
uint8_t LP5036_write_complete();

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------


#endif /* LP5036_H */
