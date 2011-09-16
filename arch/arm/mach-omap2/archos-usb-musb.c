/*
 * linux/arch/arm/mach-omap2/usb-musb.c
 *
 * This file will contain the board specific details for the
 * MENTOR USB OTG controller on OMAP3430
 *
 * Copyright (C) 2007-2008 Texas Instruments
 * Copyright (C) 2008 Nokia Corporation
 * Author: Vikram Pandita
 *
 * Generalization by:
 * Felipe Balbi <felipe.balbi@nokia.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/usb/musb.h>

#include <plat/usb.h>
#include <mach/archos-gpio.h>
#include <mach/board-archos.h>


static struct archos_gpio gio_usb_pwroff = UNUSED_GPIO;

static int set_vbus(struct device *dev, int is_on)
{
	if ( GPIO_EXISTS( gio_usb_pwroff ) ) {
		gpio_set_value( GPIO_PIN( gio_usb_pwroff ), !is_on);
	}
	return 0;
}

static struct omap_musb_board_data musb_board_data = {
        .interface_type         = MUSB_INTERFACE_ULPI,
        .mode                   = MUSB_OTG,
	.power			= 50,			/* up to 100 mA */
};

void __init archos_usb_musb_init(void)
{
	const struct archos_usb_config *usb_cfg;

	usb_cfg = omap_get_config( ARCHOS_TAG_USB, struct archos_usb_config );
	if ( usb_cfg ) {
		if ( hardware_rev >= usb_cfg->nrev ) {
			printk(KERN_DEBUG "archos_usb_init: hardware_rev (%i) >= nrev (%i)\n",
					hardware_rev, usb_cfg->nrev);
			return;
		}

		gio_usb_pwroff = usb_cfg->rev[hardware_rev].usb_pwroff;
		if ( GPIO_EXISTS( gio_usb_pwroff ) ) {
			archos_gpio_init_output(&gio_usb_pwroff, "USB_PWROFF");
			set_vbus(NULL, 0);

			musb_board_data.extvbus = 1;
			musb_board_data.set_vbus = set_vbus;
			
		}
		if ( usb_cfg->rev[hardware_rev].usb_max_power )
			musb_board_data.power = usb_cfg->rev[hardware_rev].usb_max_power/2;
	}
	
	usb_musb_init(&musb_board_data);
}


