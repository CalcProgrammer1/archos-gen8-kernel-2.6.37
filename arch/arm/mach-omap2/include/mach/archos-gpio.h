#ifndef _ARCH_ARCHOS_GPIO_H_
#define _ARCH_ARCHOS_GPIO_H_

#include <linux/gpio.h>

struct archos_gpio {
	int nb;
};

#define GPIO_EXISTS(x) ( x.nb >= 0 )
#define GPIO_PIN(x) ( x.nb )
#define UNUSED_GPIO (struct archos_gpio){ .nb = -1 }
#define INITIALIZE_GPIO(pin, cfg) (struct archos_gpio){ .nb = pin }

static inline void archos_gpio_init_output(const struct archos_gpio *x, const char *label)
{
	int pin = x->nb;

	WARN_ON(!gpio_is_valid(pin));

	if (pin < 0)
		return;

	if (gpio_request(pin, label) < 0) {
		pr_debug("archos_gpio_init_output: cannot acquire GPIO%d \n", pin);
		return;
	}
	if (gpio_direction_output(pin, 0) < 0)
		pr_debug("archos_gpio_init_output: cannot set output direction "
			"for GPIO%d \n", pin);
}

static inline void archos_gpio_init_input(const struct archos_gpio *x, const char *label)
{
	int pin = x->nb;

	WARN_ON(!gpio_is_valid(pin));

	if (pin < 0)
		return;

	if (gpio_request(pin, label) < 0) {
		pr_debug("archos_gpio_init_input: cannot acquire GPIO%d \n", pin);
		return;
	}
	if (gpio_direction_input(pin) < 0)
		pr_debug("archos_gpio_init_input: cannot set input direction "
			"for GPIO%d \n", pin);
}

#define omap_mux_write_mode0(val, mode0) \
	omap_mux_write(val, OMAP3_CONTROL_PADCONF_##mode0##_OFFSET)

#endif
