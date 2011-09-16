/*
 * archos-lcd-BOE.c
 *
 *  Created on: mar 15, 2010
 *      Author: Matthias Welwarsky <welwarsky@archos.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/clk.h>

#include <plat/board.h>
#include <plat/display.h>

#include <mach/gpio.h>
#include <mach/archos-gpio.h>
#include <mach/board-archos.h>

static struct archos_disp_conf display_gpio;
static int panel_state;

static int panel_enable(struct omap_dss_device *disp)
{
	pr_info("panel_enable [%s]\n", disp->name);

	if ( panel_state == 1)
		return -1;

	if (GPIO_EXISTS(display_gpio.lcd_pwon)) {
		gpio_set_value( GPIO_PIN(display_gpio.lcd_pwon), 1 );
		msleep(50);
	}
	if (GPIO_EXISTS(display_gpio.lcd_rst)) {
		gpio_set_value( GPIO_PIN(display_gpio.lcd_rst), 1 );
		msleep(100);
	}
	if (GPIO_EXISTS(display_gpio.lcd_pci))
		gpio_set_value( GPIO_PIN(display_gpio.lcd_pci), 0 );

	panel_state = 1;
	return 0;
}

static void panel_disable(struct omap_dss_device *disp)
{
	pr_debug("panel_disable [%s]\n", disp->name);

	if (GPIO_EXISTS(display_gpio.lcd_pci))
		gpio_set_value( GPIO_PIN(display_gpio.lcd_pci), 1 );

	if (GPIO_EXISTS(display_gpio.lcd_rst))
		gpio_set_value( GPIO_PIN(display_gpio.lcd_rst), 0 );
	if (GPIO_EXISTS(display_gpio.lcd_pwon))
		gpio_set_value( GPIO_PIN(display_gpio.lcd_pwon), 0 );

	panel_state = 0;
}


static struct omap_dss_device boe_wsvga_10_panel = {
	.type = OMAP_DISPLAY_TYPE_DPI,
	.name = "lcd",
	.driver_name = "boe_wsvga_10",
	.phy.dpi.data_lines = 18,
	//.phy.dpi.dither = OMAP_DSS_DITHER_SPATIAL,
	.platform_enable = panel_enable,
	.platform_disable = panel_disable,
};

int __init panel_boe_wsvga_10_init(struct omap_dss_device *disp_data)
{
	const struct archos_display_config *disp_cfg;
	int ret = -ENODEV;

	disp_cfg = omap_get_config( ARCHOS_TAG_DISPLAY, struct archos_display_config );
	if (disp_cfg == NULL)
		return ret;

	if ( hardware_rev >= disp_cfg->nrev ) {
		printk(KERN_DEBUG "archos_display_init: hardware_rev (%i) >= nrev (%i)\n",
			hardware_rev, disp_cfg->nrev);
		return ret;
	}

	*disp_data = boe_wsvga_10_panel;
	display_gpio = disp_cfg->rev[hardware_rev];

	archos_gpio_init_output(&display_gpio.lcd_pwon, "LCD_PWON");
	archos_gpio_init_output(&display_gpio.lcd_rst, "LCD_RST");
	if (GPIO_EXISTS(display_gpio.lcd_pci))
		archos_gpio_init_output(&display_gpio.lcd_pci, "LCD_PCI");

#if !defined(CONFIG_FB_OMAP_BOOTLOADER_INIT)
	panel_disable(disp_data);
#endif

	return 0;
}
