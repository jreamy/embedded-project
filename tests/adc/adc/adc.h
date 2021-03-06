// Interrupt driven ADC that reads the SAMd21 ADC

#ifndef ADC_H
#define ADC_H

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//     ___      __   ___  __   ___  ___  __
//      |  \ / |__) |__  |  \ |__  |__  /__`
//      |   |  |    |___ |__/ |___ |    .__/
//
//------------------------------------------------------------------------------

typedef uint8_t (*adc_callback_t)(uint16_t);

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

void adc_init();
void adc_read(uint8_t pin);

uint8_t adc_register(adc_callback_t callback);
void adc_unregister(adc_callback_t callback);
void adc_set_default(adc_callback_t callback);

#endif /* ADC_H */
