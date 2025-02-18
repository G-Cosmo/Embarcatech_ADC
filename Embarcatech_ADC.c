#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define VRX 27
#define VRY 26
#define buttonJ 22
#define buttonA 5
#define WRAP 4095

uint led_rgb[3] = {13,11,12};
uint64_t volatile last_time = 0;
uint64_t volatile current_time = 0;
bool led_flag = true;

uint init_pwm(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    
    pwm_set_enabled(slice_num, true);  
    return slice_num;  
}

void init_rgb(uint *led)
{
    gpio_init(led[0]);
    gpio_init(led[1]);
    gpio_init(led[2]);

    gpio_set_dir(led[0], GPIO_OUT);
    gpio_set_dir(led[1], GPIO_OUT);
    gpio_set_dir(led[2], GPIO_OUT);

    gpio_put(led[0], false);
    gpio_put(led[1], false);
    gpio_put(led[2], false);
}

void init_buttons()
{
    gpio_init(buttonJ);
    gpio_init(buttonA);

    gpio_set_dir(buttonJ, GPIO_IN);
    gpio_set_dir(buttonA, GPIO_IN);

    gpio_pull_up(buttonJ);
    gpio_pull_up(buttonA);
}

void gpio_irq_handler(uint gpio, uint32_t events)
{
    current_time = to_ms_since_boot(get_absolute_time());

    if((current_time - last_time > 200))
    {
        last_time =  current_time;
        
        led_flag = !led_flag;
    }
}

int main()
{
    stdio_init_all();

    init_rgb(led_rgb);

    init_buttons();
    
    adc_init();
    adc_gpio_init(VRX);
    adc_gpio_init(VRY);

    uint pwm_red = init_pwm(led_rgb[0],WRAP);
    uint pwm_blue = init_pwm(led_rgb[2],WRAP);

    gpio_set_irq_enabled_with_callback(buttonA, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);


    while (true) {
        adc_select_input(1);
        uint16_t vrx_value = adc_read();
        printf("\n X value: %d", vrx_value);

        adc_select_input(0);
        uint16_t vry_value = adc_read();
        printf("\n Y value: %d", vry_value);

        if(led_flag)
        {
            if(vrx_value == 2047)
            {
                pwm_set_gpio_level(led_rgb[0], 0);
            }else if(vrx_value < 2047)
            {
                pwm_set_gpio_level(led_rgb[0], 2047 + (2047 - vrx_value));
            }else
            {
                pwm_set_gpio_level(led_rgb[0], vrx_value);
            }


            if(vry_value == 2047)
            {
                pwm_set_gpio_level(led_rgb[2], 0);
            }else if(vry_value < 2047)
            {
                pwm_set_gpio_level(led_rgb[2], 2047 + (2047 - vry_value));
            }else{
                pwm_set_gpio_level(led_rgb[2], vry_value);
            }
        }

        current_time = to_ms_since_boot(get_absolute_time());

        if(!gpio_get(buttonJ) && (current_time - last_time > 200))
        {
            last_time = current_time;
            gpio_put(led_rgb[1], !gpio_get(led_rgb[1]));
        }

        sleep_ms(10);
    }
}
