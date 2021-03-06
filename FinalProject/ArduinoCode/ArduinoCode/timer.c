//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "timer.h"

//------------------------------------------------------------------------------
//                __          __        ___  __
//     \  /  /\  |__) |  /\  |__) |    |__  /__`
//      \/  /~~\ |  \ | /~~\ |__) |___ |___ .__/
//
//------------------------------------------------------------------------------

volatile uint64_t millis;

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//==============================================================================
void timer_init()
{
    SysTick_Config(48000);
    millis = 0;
}

//==============================================================================
uint64_t timer_get()
{
    return millis;
}

//==============================================================================
void timer_delay(uint32_t delay)
{
    uint64_t start = millis;
    while (millis - start < delay);
}

//------------------------------------------------------------------------------
//        __   __  , __
//     | /__` |__)  /__`
//     | .__/ |  \  .__/
//
//------------------------------------------------------------------------------

//==============================================================================
void SysTick_Handler()
{
    millis++;
}
