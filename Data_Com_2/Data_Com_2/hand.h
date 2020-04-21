#ifndef HAND_H
#define HAND_H

//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include <sam.h>
#include <stddef.h>
#include <stdint.h>

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

#define PNT_TIP_ADDR    (0x1E)
#define MID_TIP_ADDR    (0x1E)
#define RNG_TIP_ADDR    (0x1E)
#define PNK_TIP_ADDR    (0x1E)
#define THM_TIP_ADDR    (0x1E)

#define PNT_MID_ADDR    (0x1C)
#define MID_MID_ADDR    (0x1C)
#define RNG_MID_ADDR    (0x1C)
#define PNK_MID_ADDR    (0x1C)
#define THM_MID_ADDR    (0x1C)

#define PNT_BS_ADDR     (0x1E)
#define MID_BS_ADDR     (0x1C)
#define RNG_BS_ADDR     (0x1E)
#define PNK_BS_ADDR     (0x1C)
#define THM_BS_ADDR     (0x1E)
#define HND_BS_ADDR     (0x1C)

#define MUX_PNT_TOP     (1<<7)
#define MUX_MID_TOP     (1<<6)
#define MUX_PNT_MID     (1<<5)
#define MUX_RNG_TOP     (1<<4)
#define MUX_PNK_TOP     (1<<3)
#define MUX_RNG_PNK     (1<<2)
#define MUX_THM_TOP     (1<<1)
#define MUX_THM_HND     (1<<0)

//------------------------------------------------------------------------------
//     ___      __   ___  __   ___  ___  __
//      |  \ / |__) |__  |  \ |__  |__  /__`
//      |   |  |    |___ |__/ |___ |    .__/
//
//------------------------------------------------------------------------------

typedef union {
    struct {
        int16_t x;
        int16_t y;
        int16_t z;
    };
    uint8_t bytes[6];
} joint_pos_t;

typedef union {
    struct {
        joint_pos_t tip;
        joint_pos_t mid;
        joint_pos_t bs;
    };
    joint_pos_t joints[3];
    uint8_t bytes[18];
} finger_pos_t;

typedef union {
    struct {
        finger_pos_t pnt;
        finger_pos_t mid;
        finger_pos_t rng;
        finger_pos_t pnk;
        finger_pos_t thm;
        joint_pos_t hnd;
    };
    joint_pos_t joints[16];
    uint8_t bytes[16*6];
} hand_pos_t;

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

void hand_init();
void hand_stop();
uint8_t hand_read(hand_pos_t* output);
uint8_t hand_read_complete();

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------


#endif /* HAND_H */
