#include "hal_adc_inputs.h"

unsigned long adc_tps_average = 0;
unsigned long adc_counter = 0;

void HAL_ANALOG_ADC_Init(void)
{
    initADC();
}

unsigned long TEST_ADC_Get_TBIPosition()
{
    unsigned long tps_average;
    if(adc_counter > 0)
    {
        tps_average = adc_tps_average/adc_counter;
    }
    else
    {
        tps_average = HAL_ADC_Get_V_TBIPosition();
    }
    adc_counter = 0;
    adc_tps_average = 0;

    return tps_average;
}
