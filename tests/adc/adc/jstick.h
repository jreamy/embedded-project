// Joystick driver code for the Batman

#ifndef JSTICK_H
#define JSTICK_H

//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include <stdint.h>

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

#define JX (0)
#define JY (1)

#define JX_MAX (4096)
#define JY_MAX (4096)

#define JX_AVG (1996)
#define JY_AVG (2056)

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

void emg_init();
uint8_t emg_read();
uint8_t emg_read_complete();

#endif /* JSTICK_H */
