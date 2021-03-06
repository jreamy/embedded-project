#ifndef KX003_H
#define KX003_H

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

#define KX003_XOUT_L    (0x06)
#define KX003_XOUT_H    (0x07)
#define KX003_YOUT_L    (0x08)
#define KX003_YOUT_H    (0x09)
#define KX003_ZOUT_L    (0x0A)
#define KX003_ZOUT_H    (0x0B)
#define KX003_STATUS    (0x18)
#define KX003_CTRL_REG  (0x1D)

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

void kx003_init(uint8_t addr);
void kx003_stop(uint8_t addr);
uint8_t kx003_read(uint8_t addr, uint8_t reg, uint8_t* dest, uint8_t bytes);
uint8_t kx003_read_complete();

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------


#endif /* KX003_H */
