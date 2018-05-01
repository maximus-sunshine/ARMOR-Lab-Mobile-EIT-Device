#include "gpiolib.h"

#define BANK            1
#define PINMASK         (1 << 12)
#define ITERATIONS      3000000 * 3
 
void main()
{
        gpio_info *gg;
        int i;
 
        gpio_init();
        gg = gpio_attach(BANK, PINMASK, GPIO_OUT);
 
        for (i = 0; i < ITERATIONS; i++) {
                gpio_set(gg);
                gpio_clear(gg);
        }
 
        gpio_detach(gg);
        gpio_finish();
}