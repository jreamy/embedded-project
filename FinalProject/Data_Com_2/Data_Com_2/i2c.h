#ifndef I2C_H
#define I2C_H

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

// Remove these
#define TX_S ('1')
#define TX_C ('2')
#define TX_E ('3')
#define RX_S ('4')
#define RX_C ('5')
#define RX_E ('6')

//------------------------------------------------------------------------------
//     ___      __   ___  __   ___  ___  __
//      |  \ / |__) |__  |  \ |__  |__  /__`
//      |   |  |    |___ |__/ |___ |    .__/
//
//------------------------------------------------------------------------------

typedef uint8_t (*i2c_callback_t)(void);

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

void i2c_init(uint32_t baudrate);
void i2c_stop();
uint8_t i2c_timeout();

uint8_t i2c_tx_flag();
uint8_t i2c_rx_flag();

uint8_t i2c_begin_read(uint8_t addr);
uint8_t i2c_begin_write(uint8_t addr);

uint8_t i2c_read();
void i2c_write(uint8_t data);

void i2c_rx_stop();
void i2c_tx_stop();

void i2c_end_write(i2c_callback_t callback);
void i2c_end_read(i2c_callback_t callback);

uint8_t i2c_register(i2c_callback_t callback);
void i2c_unregister(i2c_callback_t callback);
void i2c_set_default(i2c_callback_t callback);

uint8_t i2c_registered();

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------


#endif /* I2C_H */
