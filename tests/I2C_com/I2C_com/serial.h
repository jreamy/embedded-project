#ifndef SERIAL_H
#define SERIAL_H

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

#define SERIAL_BUSY (0)
#define SERIAL_FREE (1)

#define SERIAL_TX_READY (0)
#define SERIAL_TX_BUSY (1)
#define SERIAL_TX_COMPLETE (2)

//------------------------------------------------------------------------------
//     ___      __   ___  __   ___  ___  __
//      |  \ / |__) |__  |  \ |__  |__  /__`
//      |   |  |    |___ |__/ |___ |    .__/
//
//------------------------------------------------------------------------------

typedef uint8_t (*serial_callback_t)(void);

typedef struct {
    serial_callback_t rx;
    serial_callback_t tx;
} serial_t;

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

void serial_init(uint32_t baudrate);

uint8_t serial_tx_flag();
uint8_t serial_rx_flag();

uint8_t serial_read();
void serial_write(uint8_t data);

uint8_t serial_register(serial_t callback);
void serial_unregister(serial_t callback);
void serial_set_default(serial_t callback);

uint8_t serial_register_rx(serial_callback_t callback);
void serial_unregister_rx(serial_callback_t callback);
void serial_set_default_rx(serial_callback_t callback);

uint8_t serial_register_tx(serial_callback_t callback);
void serial_unregister_tx(serial_callback_t callback);
void serial_set_default_tx(serial_callback_t callback);

#endif /* SERIAL_H */
