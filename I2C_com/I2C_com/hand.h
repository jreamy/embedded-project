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

#define PNT_TIP_ADDR    ('b') //(0x1E)
#define MID_TIP_ADDR    ('c') //(0x1E)
#define RNG_TIP_ADDR    ('d') //(0x1E)
#define PNK_TIP_ADDR    ('e') //(0x1E)
#define THM_TIP_ADDR    ('f') //(0x1E)

#define PNT_MID_ADDR    ('h') //(0x1C)
#define MID_MID_ADDR    ('i') //(0x1C)
#define RNG_MID_ADDR    ('j') //(0x1C)
#define PNK_MID_ADDR    ('k') //(0x1C)
#define THM_MID_ADDR    ('l') //(0x1C)

#define PNT_BS_ADDR     ('n') //(0x1E)
#define MID_BS_ADDR     ('o') //(0x1C)
#define RNG_BS_ADDR     ('p') //(0x1E)
#define PNK_BS_ADDR     ('q') //(0x1C)
#define THM_BS_ADDR     ('r') //(0x1E)
#define HND_BS_ADDR     ('s') //(0x1C)

#define MUX_PNT_TOP     ('t') //(1<<7)
#define MUX_MID_TOP     ('u') //(1<<6)
#define MUX_PNT_MID     ('v') //(1<<5)
#define MUX_RNG_TOP     ('w') //(1<<4)
#define MUX_PNK_TOP     ('x') //(1<<3)
#define MUX_RNG_PNK     ('y') //(1<<2)
#define MUX_THM_TOP     ('z') //(1<<1)
#define MUX_THM_HND     ('z') //(1<<0)

//------------------------------------------------------------------------------
//     ___      __   ___  __   ___  ___  __
//      |  \ / |__) |__  |  \ |__  |__  /__`
//      |   |  |    |___ |__/ |___ |    .__/
//
//------------------------------------------------------------------------------

typedef union {
    struct {
        uint16_t x;
        uint16_t y;
        uint16_t z;
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
uint8_t hand_read(hand_pos_t* output);
uint8_t hand_read_complete();

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------


#endif /* HAND_H */
