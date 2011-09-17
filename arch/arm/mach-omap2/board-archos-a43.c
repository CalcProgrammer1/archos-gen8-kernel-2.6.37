/*
 * board-archos-a43.c
 *
 *  Created on: Jan 15, 2010
 *      Author: Matthias Welwarsky <welwarsky@archos.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/workqueue.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/i2c/twl.h>
#include <linux/spi/spi.h>
#include <linux/spi/ads7846.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/machine.h>
#include <linux/mmc/host.h>
#include <linux/opp.h>

#include <plat/board.h>
#include <plat/common.h>
#include <plat/display.h>
#include <plat/omap_device.h>
#include <plat/usb.h>

#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <mach/gpio.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <mach/board-archos.h>

//#include <linux/mma7660fc.h>

#ifdef CONFIG_WL127X_RFKILL
#include <linux/wl127x-rfkill.h>
#endif

#include "hsmmc.h"
#include "sdram-elpida-edk2132c2pb.h"
#include "mux.h"
#include "mux_remove.h"
#include "control.h"
#include "pm.h"

#ifdef CONFIG_PM
// TODO:
//#include <../drivers/media/video/omap/omap_voutdef.h>
#endif

// TODO: remove
unsigned int hardware_rev = 5;
EXPORT_SYMBOL(hardware_rev);

// TODO:
//static struct mma7660fc_pdata board_mma7660fc_pdata;

extern int __init archos_audio_gpio_init(void);

static struct archos_display_config display_config __initdata = {
	.nrev = 6,
	.rev[0] = {
		.lcd_pwon = 	{ 157,	AA21_34XX_GPIO157_OUT },
		.lcd_rst = 	{ 25,	AF7_3430_GPIO25_OUT },
		.lcd_pci = 	UNUSED_GPIO,
		.hdmi_pwr = 	{ 181,	Y4_34XX_GPIO181 },
		.hdmi_int = 	{ 110,	C26_3430_GPIO110 },
		.vcom_pwm = 	{ .timer = -1, .mux_cfg = -1 },
		.spi = {
			.spi_clk  = UNUSED_GPIO,
			.spi_data = UNUSED_GPIO,
			.spi_cs   = UNUSED_GPIO,
		},
	},
	.rev[1] = {
		.lcd_pwon = 	{ 157,	AA21_34XX_GPIO157_OUT },
		.lcd_rst = 	{ 25,	AF7_3430_GPIO25_OUT },
		.lcd_pci = 	UNUSED_GPIO,
		.hdmi_pwr = 	{ 181,	Y4_34XX_GPIO181 },
		.hdmi_int = 	{ 110,	C26_3430_GPIO110 },
		.vcom_pwm = 	{ .timer = -1, .mux_cfg = -1 },
		.spi = {
			.spi_clk  = UNUSED_GPIO,
			.spi_data = UNUSED_GPIO,
			.spi_cs   = UNUSED_GPIO,
		},
        },
	.rev[2] = {
		.lcd_pwon = 	{ 157,	AA21_34XX_GPIO157_OUT },
		.lcd_rst = 	{ 25,	AF7_3430_GPIO25_OUT },
		.lcd_pci = 	UNUSED_GPIO,
		.hdmi_pwr = 	{ 181,	Y4_34XX_GPIO181 },
		.hdmi_int = 	{ 110,	C26_3430_GPIO110 },
		.vcom_pwm = 	{ .timer = -1, .mux_cfg = -1 },
		.spi = {
			.spi_clk  = UNUSED_GPIO,
			.spi_data = UNUSED_GPIO,
			.spi_cs   = UNUSED_GPIO,
		},
        },
	.rev[3] = {
		.lcd_pwon = 	{ 157,	AA21_34XX_GPIO157_OUT },
		.lcd_rst = 	{ 25,	AF7_3430_GPIO25_OUT },
		.lcd_pci = 	UNUSED_GPIO,
		.hdmi_pwr = 	{ 181,	Y4_34XX_GPIO181 },
		.hdmi_int = 	{ 110,	C26_3430_GPIO110 },
		.vcom_pwm = 	{ .timer = -1, .mux_cfg = -1 },
		.spi = {
			.spi_clk  = UNUSED_GPIO,
			.spi_data = UNUSED_GPIO,
			.spi_cs   = UNUSED_GPIO,
		},
        },
	.rev[4] = {
		.lcd_pwon = 	{ 157,	AA21_34XX_GPIO157_OUT },
		.lcd_rst = 	{ 25,	AF7_3430_GPIO25_OUT },
		.lcd_pci = 	UNUSED_GPIO,
		.hdmi_pwr = 	{ 181,	Y4_34XX_GPIO181 },
		.hdmi_int = 	{ 110,	C26_3430_GPIO110 },
		.vcom_pwm = 	{ .timer = -1, .mux_cfg = -1 },
		.spi = {
			.spi_clk  = UNUSED_GPIO,
			.spi_data = UNUSED_GPIO,
			.spi_cs   = UNUSED_GPIO,
		},
        },
	.rev[5] = {
		.lcd_pwon = 	{ 157,	AA21_34XX_GPIO157_OUT },
		.lcd_rst = 	{ 25,	AF7_3430_GPIO25_OUT },
		.lcd_pci = 	UNUSED_GPIO,
		.hdmi_pwr = 	{ 181,	Y4_34XX_GPIO181 },
		.hdmi_int = 	{ 110,	C26_3430_GPIO110 },
		.vcom_pwm = 	{ .timer = -1, .mux_cfg = -1 },
		.spi = {
			.spi_clk  = UNUSED_GPIO,
			.spi_data = UNUSED_GPIO,
			.spi_cs   = UNUSED_GPIO,
		},
        },
};

static int __init archos_lcd_panel_init(struct omap_dss_device *disp_data)
{
	switch (hardware_rev) {
	default:
		return panel_fwvga_43_init(disp_data);
	}
}


static struct archos_tsp_config tsp_config __initdata = {
	.nrev = 6,
	.rev[0] = {
		.irq_gpio = { .nb = 24, .mux_cfg = AE7_3430_GPIO24 },
		.pwr_gpio = { .nb = 179, .mux_cfg = Y2_3430_GPIO179 },
		.x_plate_ohms = 290,
		.pressure_max = 1500,
		.inversion_flags = X_INV | Y_INV,
	},
	.rev[1] = {
		.irq_gpio = { .nb = 24, .mux_cfg = AE7_3430_GPIO24 },
		.pwr_gpio = { .nb = 179, .mux_cfg = Y2_3430_GPIO179 },
		.x_plate_ohms = 290,
		.pressure_max = 1500,
		.inversion_flags = X_INV | Y_INV,
	},
	.rev[2] = {
		.irq_gpio = { .nb = 24, .mux_cfg = AE7_3430_GPIO24 },
		.pwr_gpio = { .nb = 179, .mux_cfg = Y2_3430_GPIO179 },
		.x_plate_ohms = 290,
		.pressure_max = 1500,
		.inversion_flags = X_INV | Y_INV,
	},
	.rev[3] = {
		.irq_gpio = { .nb = 24, .mux_cfg = AE7_3430_GPIO24 },
		.pwr_gpio = { .nb = 179, .mux_cfg = Y2_3430_GPIO179 },
		.x_plate_ohms = 290,
		.pressure_max = 1500,
		.inversion_flags = X_INV | Y_INV,
	},
	.rev[4] = {
		.irq_gpio = { .nb = 24, .mux_cfg = AE7_3430_GPIO24 },
		.pwr_gpio = { .nb = 179, .mux_cfg = Y2_3430_GPIO179 },
		.x_plate_ohms = 290,
		.pressure_max = 1500,
		.inversion_flags = X_INV | Y_INV,
	},
	.rev[5] = {
		.irq_gpio = { .nb = 24, .mux_cfg = AE7_3430_GPIO24 },
		.pwr_gpio = { .nb = 179, .mux_cfg = Y2_3430_GPIO179 },
		.x_plate_ohms = 290,
		.pressure_max = 1500,
		.inversion_flags = X_INV | Y_INV,
	},
};

static struct archos_audio_config audio_config __initdata = {
	.nrev = 6,
	.rev[0] = {
		.spdif = UNUSED_GPIO,
		.hp_on = { .nb = 170, .mux_cfg = J25_34XX_GPIO170},
		.headphone_plugged = { .nb = 109, .mux_cfg = B25_3430_GPIO109},
		.clk_mux = -1,
	},
	.rev[1] = {
		.spdif = UNUSED_GPIO,
		.hp_on = { .nb = 170, .mux_cfg = J25_34XX_GPIO170},
		.headphone_plugged = { .nb = 109, .mux_cfg = B25_3430_GPIO109},
		.clk_mux = AE22_3430_CLKOUT2,
	},
	.rev[2] = {
		.spdif = UNUSED_GPIO,
		.hp_on = { .nb = 170, .mux_cfg = J25_34XX_GPIO170},
		.headphone_plugged = { .nb = 109, .mux_cfg = B25_3430_GPIO109},
		.clk_mux = AE22_3430_CLKOUT2,
	},
	.rev[3] = {
		.spdif = UNUSED_GPIO,
		.hp_on = { .nb = 170, .mux_cfg = J25_34XX_GPIO170},
		.headphone_plugged = { .nb = 109, .mux_cfg = B25_3430_GPIO109},
		.clk_mux = AE22_3430_CLKOUT2,
	},
	.rev[4] = {
		.spdif = UNUSED_GPIO,
		.hp_on = { .nb = 170, .mux_cfg = J25_34XX_GPIO170},
		.headphone_plugged = { .nb = 109, .mux_cfg = B25_3430_GPIO109},
		.clk_mux = AE22_3430_CLKOUT2,
	},
	.rev[5] = {
		.spdif = UNUSED_GPIO,
		.hp_on = { .nb = 170, .mux_cfg = J25_34XX_GPIO170},
		.headphone_plugged = { .nb = 109, .mux_cfg = B25_3430_GPIO109},
		.clk_mux = AE22_3430_CLKOUT2,
	},
};

static struct archos_charge_config charge_config __initdata = {
	.nrev = 6,
	.rev[0] = {
		.charge_enable 	= { .nb = 27, .mux_cfg = AH7_3430_GPIO27 },
		.charge_high	= { .nb = 28, .mux_cfg = AG8_3430_GPIO28 },
		.charge_low 	= { .nb = 22, .mux_cfg = AF9_3430_GPIO22 },
		.charger_type	= CHARGER_LX2208,
	},
	.rev[1] = {
		.charge_enable 	= { .nb = 27, .mux_cfg = AH7_3430_GPIO27 },
		.charge_high	= { .nb = 28, .mux_cfg = AG8_3430_GPIO28 },
		.charge_low 	= { .nb = 22, .mux_cfg = AF9_3430_GPIO22 },
		.charger_type	= CHARGER_LX2208,
	},
	.rev[2] = {
		.charge_enable 	= { .nb = 27, .mux_cfg = AH7_3430_GPIO27 },
		.charge_high	= { .nb = 28, .mux_cfg = AG8_3430_GPIO28 },
		.charge_low 	= { .nb = 22, .mux_cfg = AF9_3430_GPIO22 },
		.charger_type	= CHARGER_LX2208,
	},
	.rev[3] = {
		.charge_enable 	= { .nb = 27, .mux_cfg = AH7_3430_GPIO27 },
		.charge_high	= { .nb = 28, .mux_cfg = AG8_3430_GPIO28 },
		.charge_low 	= { .nb = 22, .mux_cfg = AF9_3430_GPIO22 },
		.charger_type	= CHARGER_LX2208,
	},
	.rev[4] = {
		.charge_enable 	= { .nb = 27, .mux_cfg = AH7_3430_GPIO27 },
		.charge_high	= { .nb = 28, .mux_cfg = AG8_3430_GPIO28 },
		.charge_low 	= { .nb = 22, .mux_cfg = AF9_3430_GPIO22 },
		.charger_type	= CHARGER_LX2208,
	},
	.rev[5] = {
		.charge_enable 	= { .nb = 27, .mux_cfg = AH7_3430_GPIO27 },
		.charge_high	= { .nb = 28, .mux_cfg = AG8_3430_GPIO28 },
		.charge_low 	= { .nb = 22, .mux_cfg = AF9_3430_GPIO22 },
		.charger_type	= CHARGER_LX2208,
	},
};


static struct archos_wifi_bt_config wifi_bt_dev_conf __initdata = {
	.nrev = 6,
	.rev[0] = {
		.wifi_power 	= { .nb = 111, .mux_cfg = B26_3430_GPIO111 },
		.wifi_irq 	= { .nb = 114, .mux_cfg = AG18_3430_GPIO114_UP },
		.bt_power 	= { .nb = 162, .mux_cfg = W21_3430_GPIO162 },
	},
	.rev[1] = {
		.wifi_power 	= { .nb = 111, .mux_cfg = B26_3430_GPIO111 },
		.wifi_irq 	= { .nb = 114, .mux_cfg = AG18_3430_GPIO114_UP },
		.bt_power 	= { .nb = 162, .mux_cfg = W21_3430_GPIO162 },
	},
	.rev[2] = {
		.wifi_power 	= { .nb = 111, .mux_cfg = B26_3430_GPIO111 },
		.wifi_irq 	= { .nb = 114, .mux_cfg = AG18_3430_GPIO114_UP },
		.bt_power 	= { .nb = 162, .mux_cfg = W21_3430_GPIO162 },
	},
	.rev[3] = {
		.wifi_power 	= { .nb = 111, .mux_cfg = B26_3430_GPIO111 },
		.wifi_irq 	= { .nb = 114, .mux_cfg = AG18_3430_GPIO114_UP },
		.bt_power 	= { .nb = 162, .mux_cfg = W21_3430_GPIO162 },
	},
	.rev[4] = {
		.wifi_power 	= { .nb = 111, .mux_cfg = B26_3430_GPIO111 },
		.wifi_irq 	= { .nb = 114, .mux_cfg = AG18_3430_GPIO114_UP },
		.bt_power 	= { .nb = 162, .mux_cfg = W21_3430_GPIO162 },
		.wifi_pa_type	= PA_TYPE_RF3482,
	},
	.rev[5] = {
		.wifi_power 	= { .nb = 111, .mux_cfg = B26_3430_GPIO111 },
		.wifi_irq 	= { .nb = 114, .mux_cfg = AG18_3430_GPIO114_UP },
		.bt_power 	= { .nb = 162, .mux_cfg = W21_3430_GPIO162 },
		.wifi_pa_type	= PA_TYPE_TQM67002A,
	},
};


static struct archos_gps_config gps_config __initdata = {
	.nrev = 6,
	.rev[0] = {
		.gps_enable = { .nb = 26, .mux_cfg = AG7_3430_GPIO26 },
		.gps_int = { .nb = 177, .mux_cfg = AB2_34XX_GPIO177 },
		.gps_reset = UNUSED_GPIO,
	},
	.rev[1] = {
		.gps_enable = { .nb = 26, .mux_cfg = AG7_3430_GPIO26 },
		.gps_int = { .nb = 177, .mux_cfg = AB2_34XX_GPIO177 },
		.gps_reset = UNUSED_GPIO,
	},
	.rev[2] = {
		.gps_enable = { .nb = 26, .mux_cfg = AG7_3430_GPIO26 },
		.gps_int = { .nb = 177, .mux_cfg = AB2_34XX_GPIO177 },
		.gps_reset = UNUSED_GPIO,
	},
	.rev[3] = {
		.gps_enable = { .nb = 26, .mux_cfg = AG7_3430_GPIO26 },
		.gps_int = { .nb = 177, .mux_cfg = AB2_34XX_GPIO177 },
		.gps_reset = UNUSED_GPIO,
	},
	.rev[4] = {
		.gps_enable = { .nb = 26, .mux_cfg = AG7_3430_GPIO26 },
		.gps_int = { .nb = 177, .mux_cfg = AB2_34XX_GPIO177 },
		.gps_reset = UNUSED_GPIO,
	},
	.rev[5] = {
		.gps_enable = { .nb = 26, .mux_cfg = AG7_3430_GPIO26 },
		.gps_int = { .nb = 177, .mux_cfg = AB2_34XX_GPIO177 },
		.gps_reset = UNUSED_GPIO,
	},
};

static struct archos_accel_config accel_config __initdata = {
	.nrev = 6,
	.rev[0] = {
		.accel_int1 = { .nb = 115, .mux_cfg = AH18_3430_GPIO115},
		.accel_int2 = UNUSED_GPIO,
	},
	.rev[1] = {
		.accel_int1 = { .nb = 115, .mux_cfg = AH18_3430_GPIO115},
		.accel_int2 = UNUSED_GPIO,
	},
	.rev[2] = {
		.accel_int1 = { .nb = 115, .mux_cfg = AH18_3430_GPIO115},
		.accel_int2 = UNUSED_GPIO,
	},
	.rev[3] = {
		.accel_int1 = { .nb = 115, .mux_cfg = AH18_3430_GPIO115},
		.accel_int2 = UNUSED_GPIO,
	},
	.rev[4] = {
		.accel_int1 = { .nb = 115, .mux_cfg = AH18_3430_GPIO115},
		.accel_int2 = UNUSED_GPIO,
	},
	.rev[5] = {
		.accel_int1 = { .nb = 115, .mux_cfg = AH18_3430_GPIO115},
		.accel_int2 = UNUSED_GPIO,
	},
};

static struct archos_camera_config camera_config __initdata = {
	.nrev = 6,
	.rev[0] = {
		.pwr_down = { .nb = 144, .mux_cfg = AB26_34XX_GPIO144},
		.reset = { .nb = 178, .mux_cfg = AA3_3430_GPIO178},
	},
	.rev[1] = {
		.pwr_down = { .nb = 129, .mux_cfg = R25_3430_GPIO129},
		.reset = { .nb = 178, .mux_cfg = AA3_3430_GPIO178},
	},
	.rev[2] = {
		.pwr_down = { .nb = 144, .mux_cfg = AB26_34XX_GPIO144},
		.reset = { .nb = 178, .mux_cfg = AA3_3430_GPIO178},
	},
	.rev[3] = {
		.pwr_down = { .nb = 144, .mux_cfg = AB26_34XX_GPIO144},
		.reset = { .nb = 178, .mux_cfg = AA3_3430_GPIO178},
	},
	.rev[4] = {
		.pwr_down = { .nb = 144, .mux_cfg = AB26_34XX_GPIO144},
		.reset = { .nb = 178, .mux_cfg = AA3_3430_GPIO178},
	},
	.rev[5] = {
		.pwr_down = { .nb = 144, .mux_cfg = AB26_34XX_GPIO144},
		.reset = { .nb = 178, .mux_cfg = AA3_3430_GPIO178},
	},
};

static struct archos_leds_config leds_config __initdata = {
	.nrev = 6,
	.rev[0] = {
		.power_led = { .nb = 57, .mux_cfg = P8_34XX_GPIO57_OFF_LOW},
		.backlight_led = { .timer = 8, .mux_cfg = V3_3430_GPT08 },
		.backlight_power = UNUSED_GPIO,
	},
	.rev[1] = {
		.power_led = { .nb = 57, .mux_cfg = P8_34XX_GPIO57_OFF_LOW},
		.backlight_led = { .timer = 8, .mux_cfg = V3_3430_GPT08 },
		.bkl_max = 200,
		.backlight_power = UNUSED_GPIO,
	},
	.rev[2] = {
		.power_led = { .nb = 57, .mux_cfg = P8_34XX_GPIO57_OFF_LOW},
		.backlight_led = { .timer = 8, .mux_cfg = V3_3430_GPT08 },
		.bkl_max = 200,
		.backlight_power = UNUSED_GPIO,
	},
	.rev[3] = {
		.power_led = { .nb = 57, .mux_cfg = P8_34XX_GPIO57_OFF_LOW},
		.backlight_led = { .timer = 8, .mux_cfg = V3_3430_GPT08 },
		.bkl_max = 200,
		.backlight_power = UNUSED_GPIO,
	},
	.rev[4] = {
		.power_led = { .nb = 57, .mux_cfg = P8_34XX_GPIO57_OFF_LOW},
		.backlight_led = { .timer = 8, .mux_cfg = V3_3430_GPT08 },
		.bkl_max = 200,
		.backlight_power = UNUSED_GPIO,
	},
	.rev[5] = {
		.power_led = { .nb = 57, .mux_cfg = P8_34XX_GPIO57_OFF_LOW},
		.backlight_led = { .timer = 8, .mux_cfg = V3_3430_GPT08 },
		.bkl_max = 200,
		.backlight_power = UNUSED_GPIO,
	},
};

static struct archos_sd_config sd_config __initdata = {
	.nrev = 6,
	.rev[0] = {
		.sd_power = { .nb = 158, .mux_cfg = V21_3430_GPIO158 },
		.sd_detect = { .nb = 65, .mux_cfg = J8_3430_GPIO65 },
		.sd_prewarn = { .nb = 58, .mux_cfg = N8_3430_GPIO58_IN },
	},
	.rev[1] = {
		.sd_power = { .nb = 158, .mux_cfg = V21_3430_GPIO158 },
		.sd_detect = { .nb = 65, .mux_cfg = J8_3430_GPIO65 },
		.sd_prewarn = { .nb = 58, .mux_cfg = N8_3430_GPIO58_IN },
	},
	.rev[2] = {
		.sd_power = { .nb = 158, .mux_cfg = V21_3430_GPIO158 },
		.sd_detect = { .nb = 65, .mux_cfg = J8_3430_GPIO65 },
		.sd_prewarn = { .nb = 58, .mux_cfg = N8_3430_GPIO58_IN },
	},
	.rev[3] = {
		.sd_power = { .nb = 158, .mux_cfg = V21_3430_GPIO158 },
		.sd_detect = { .nb = 65, .mux_cfg = J8_3430_GPIO65 },
		.sd_prewarn = { .nb = 58, .mux_cfg = N8_3430_GPIO58_IN },
	},
	.rev[4] = {
		.sd_power = { .nb = 158, .mux_cfg = V21_3430_GPIO158 },
		.sd_detect = { .nb = 65, .mux_cfg = J8_3430_GPIO65 },
		.sd_prewarn = { .nb = 58, .mux_cfg = N8_3430_GPIO58_IN },
	},
	.rev[5] = {
		.sd_power = { .nb = 158, .mux_cfg = V21_3430_GPIO158 },
		.sd_detect = { .nb = 65, .mux_cfg = J8_3430_GPIO65 },
		.sd_prewarn = { .nb = 58, .mux_cfg = N8_3430_GPIO58_IN },
	},
};

static struct archos_compass_config compass_config __initdata = {
	.nrev = 6,
	.rev[0] = {
		.reset = { .nb = 128, .mux_cfg = R27_3430_GPIO128},
	},
	.rev[1] = {
		.reset = { .nb = 128, .mux_cfg = R27_3430_GPIO128},
	},
	.rev[2] = {
		.reset = { .nb = 128, .mux_cfg = R27_3430_GPIO128},
	},
	.rev[3] = {
		.reset = { .nb = 128, .mux_cfg = R27_3430_GPIO128},
	},
	.rev[4] = {
		.reset = { .nb = 128, .mux_cfg = R27_3430_GPIO128},
	},
	.rev[5] = {
		.reset = { .nb = 128, .mux_cfg = R27_3430_GPIO128},
	},
};

static struct archos_keys_config keys_config __initdata = {
	.nrev = 6,
	.rev[0] = {
		.power = { .nb = 64, .mux_cfg = K8_3430_GPIO64},
	},
	.rev[1] = {
		.power = { .nb = 64, .mux_cfg = K8_3430_GPIO64},
	},
	.rev[2] = {
		.power = { .nb = 64, .mux_cfg = K8_3430_GPIO64},
	},
	.rev[3] = {
		.power = { .nb = 64, .mux_cfg = K8_3430_GPIO64},
	},
	.rev[4] = {
		.power = { .nb = 64, .mux_cfg = K8_3430_GPIO64},
	},
	.rev[5] = {
		.power = { .nb = 64, .mux_cfg = K8_3430_GPIO64},
	},
};

static struct omap_board_config_kernel board_config[] __initdata = {
	{ ARCHOS_TAG_DISPLAY,	&display_config },
	{ ARCHOS_TAG_TSP,	&tsp_config },
	{ ARCHOS_TAG_CHARGE,	&charge_config},
	{ ARCHOS_TAG_AUDIO,     &audio_config},
	{ ARCHOS_TAG_WIFI_BT,	&wifi_bt_dev_conf},
	{ ARCHOS_TAG_ACCEL,	&accel_config},
	{ ARCHOS_TAG_CAMERA,	&camera_config},
	{ ARCHOS_TAG_LEDS,	&leds_config},
	{ ARCHOS_TAG_GPS,	&gps_config},
	{ ARCHOS_TAG_SD,    	&sd_config},
	{ ARCHOS_TAG_COMPASS,	&compass_config},
	{ ARCHOS_TAG_KEYS,	&keys_config},
};

#ifdef CONFIG_WL127X_RFKILL
static struct wl127x_rfkill_platform_data wl127x_plat_data = {
	.bt_nshutdown_gpio = 162, 	/* Bluetooth Enable GPIO */
	.fm_enable_gpio = 0,		/* FM Enable GPIO */
};

static struct platform_device board_wl127x_device = {
	.name           = "wl127x-rfkill",
	.id             = -1,
	.dev.platform_data = &wl127x_plat_data,
};
#endif

#ifdef CONFIG_OMAP2_DSS
static struct omap_dss_device board_lcd_device;

static int board_panel_enable_hdmi(struct omap_dss_device *dssdev)
{
	return 0;
}

static void board_panel_disable_hdmi(struct omap_dss_device *dssdev)
{
}

#ifdef CONFIG_OMAP2_DSS_DUMMY
static struct omap_dss_device board_dummy_device = {
	.name = "dummy",
	.driver_name = "generic_panel",
	.type = OMAP_DISPLAY_TYPE_DUMMY,
	.phy.dpi.data_lines = 24,
};
#endif

// TODO:
#if 0
static struct omap_dss_device board_hdmi_device = {
	.name = "hdmi",
	.driver_name = "hdmi_panel",
	.type = OMAP_DISPLAY_TYPE_HDMI,
	.phy.dpi.data_lines = 24,
	.platform_enable = board_panel_enable_hdmi,
	.platform_disable = board_panel_disable_hdmi,
};
#endif

#ifdef CONFIG_OMAP2_DSS_DUMMY
static struct omap_dss_device *board_dss_devices[3];
#else
static struct omap_dss_device *board_dss_devices[2];
#endif

static struct omap_dss_board_info board_dss_data = {
	.num_devices = 0,
	.devices = board_dss_devices,
	.default_device = NULL,
};

static struct platform_device board_dss_device = {
	.name		= "omapdss",
	.id		= -1,
	.dev            = {
		.platform_data = &board_dss_data,
	},
};

// TODO:
#if 0
static struct resource board_vout_resource[3 - CONFIG_FB_OMAP2_NUM_FBS] = {
};

#ifdef CONFIG_PM
struct vout_platform_data a43_vout_data = {
	.set_min_bus_tput = omap_pm_set_min_bus_tput,
	.set_max_mpu_wakeup_lat =  omap_pm_set_max_mpu_wakeup_lat,
	.set_vdd1_opp = omap_pm_set_min_mpu_freq,
	.set_cpu_freq = omap_pm_cpu_set_freq,
};
#endif

static struct platform_device board_vout_device = {
	.name		= "omap_vout",
	.num_resources	= ARRAY_SIZE(board_vout_resource),
	.resource 	= &board_vout_resource[0],
	.id		= -1,
#ifdef CONFIG_PM
	.dev		= {
		.platform_data = &a43_vout_data,
	}
#else
	.dev		= {
		.platform_data = NULL,
	}
#endif
};
#endif
#endif

static struct regulator_consumer_supply board_vdda_dac_supply = {
	.supply		= "vdda_dac",
#ifdef CONFIG_OMAP2_DSS
	.dev		= &board_dss_device.dev,
#endif
};

static struct regulator_consumer_supply board_avcc_supply = {
	.supply		= "avcc",
};

// TODO:
//extern struct platform_device a43_camera_device;

static struct regulator_consumer_supply board_2v8d_cam_supply = {
	.supply 	= "2v8d_cam",
	// TODO:
	//.dev		= &a43_camera_device.dev,
};

//static struct regulator_consumer_supply board_vdd1_supply = {
	//REGULATOR_SUPPLY("mpu", NULL);
//};

/* consumer for vdds_dsi which is permanently enabled */
static struct regulator_consumer_supply board_vdds_dsi_supply = {
	.supply		= "vdds_dsi",
#ifdef CONFIG_OMAP2_DSS
	.dev		= &board_dss_device.dev,
#endif
};

/* ------ TPS65921 init data ---------- */

#define TWL4030_MSECURE_GPIO	159

static int __init msecure_init(void)
{
	int ret = 0;

#ifdef CONFIG_RTC_DRV_TWL4030
	void __iomem *msecure_pad_config_reg =
		omap_ctrl_base_get() + 0x192;
	int mux_mask = 0x04;
	u16 tmp;

	ret = gpio_request(TWL4030_MSECURE_GPIO, "msecure");
	if (ret < 0) {
		printk(KERN_ERR "msecure_init: can't"
			"reserve GPIO:%d !\n", TWL4030_MSECURE_GPIO);
		goto out;
	}
	
	/*
	 * TWL4030 will be in secure mode if msecure line from OMAP
	 * is low. Make msecure line high in order to change the
	 * TWL4030 RTC time and calender registers.
	 */
	tmp = __raw_readw(msecure_pad_config_reg);
	tmp &= 0xF8;	/* To enable mux mode 03/04 = GPIO_RTC */
	tmp |= mux_mask;/* To enable mux mode 03/04 = GPIO_RTC */
	__raw_writew(tmp, msecure_pad_config_reg);

	gpio_direction_output(TWL4030_MSECURE_GPIO, 1);
out:
#endif
	return ret;
}

/* VOLUME UP/DOWN */
static uint32_t board_twl4030_keymap[] = {
	KEY(0, 0, KEY_VOLUMEUP),
	KEY(0, 1, KEY_VOLUMEDOWN),
};

static struct matrix_keymap_data board_twl4030_map_data = {
	.keymap		= board_twl4030_keymap,
	.keymap_size	= ARRAY_SIZE(board_twl4030_keymap),
};

static struct twl4030_keypad_data board_kp_twl4030_data = {
	.keymap_data	= &board_twl4030_map_data,
	.rows		= 1,
	.cols		= 2,
	.rep		= 1,
};


static struct regulator_init_data board_vdac = {
	.constraints = {
		.min_uV                 = 1800000,
		.max_uV                 = 1800000,
		.valid_modes_mask       = REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask         = REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &board_vdda_dac_supply,
};

static struct regulator_init_data board_vmmc1 = {
	.constraints = {
		.min_uV			= 3000000,
		.max_uV			= 3000000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
		.always_on		= 1,
		.apply_uV		= 1,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &board_avcc_supply,
};

static struct regulator_init_data board_vaux2 = {
	.constraints = {
		.min_uV			= 2800000,
		.max_uV			= 2800000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
		.always_on		= 1,
		.apply_uV		= 1,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &board_2v8d_cam_supply,
};

/*
static struct regulator_init_data board_vdd1 = {
	.constraints = {
		.min_uV			= 1012500,
		.max_uV			= 1375000,
		.valid_modes_mask	= REGULATOR_MODE_FAST,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &board_vdd1_supply,
};*/

static struct twl4030_usb_data board_usb_data = {
	.usb_mode	= T2_USB_MODE_ULPI,
	// TODO:
	//.enable_charge_detect = 1, 
};

/* TODO: might not be needed
static struct twl4030_gpio_platform_data board_gpio_data = {
	.gpio_base	= OMAP_MAX_GPIO_LINES,
	.irq_base	= TWL4030_GPIO_IRQ_BASE,
	.irq_end	= TWL4030_GPIO_IRQ_END,
	.setup		= zoom2_twl_gpio_setup,
};
*/
static struct twl4030_madc_platform_data board_madc_data = {
	.irq_line	= 1,
};

static struct twl4030_ins __initdata sleep_on_seq[] = {

	/* Turn OFF VAUX2 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VAUX2, RES_STATE_OFF), 2},
	/* Turn off HFCLKOUT */
	{MSG_SINGULAR(DEV_GRP_P1, RES_HFCLKOUT, RES_STATE_OFF), 2},
	/* Turn OFF VDD1 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD1, RES_STATE_OFF), 2},
	/* Turn OFF VDD2 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD2, RES_STATE_OFF), 2},
	/* Turn OFF VPLL1 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VPLL1, RES_STATE_OFF), 2},
};

static struct twl4030_script sleep_on_script __initdata = {
	.script	= sleep_on_seq,
	.size	= ARRAY_SIZE(sleep_on_seq),
	.flags	= TWL4030_SLEEP_SCRIPT,
};

static struct twl4030_ins wakeup_p12_seq[] __initdata = {
	/* Turn on VAUX2 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VAUX2, RES_STATE_ACTIVE), 2},
	/* Turn on HFCLKOUT */
	{MSG_SINGULAR(DEV_GRP_P1, RES_HFCLKOUT, RES_STATE_ACTIVE), 2},
	/* Turn ON VDD1 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD1, RES_STATE_ACTIVE), 2},
	/* Turn ON VDD2 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD2, RES_STATE_ACTIVE), 2},
	/* Turn ON VPLL1 */
	{MSG_SINGULAR(DEV_GRP_P1, RES_VPLL1, RES_STATE_ACTIVE), 2},
};

static struct twl4030_script wakeup_p12_script __initdata = {
	.script = wakeup_p12_seq,
	.size   = ARRAY_SIZE(wakeup_p12_seq),
	.flags  = TWL4030_WAKEUP12_SCRIPT,
};

static struct twl4030_ins wakeup_p3_seq[] __initdata = {
	{MSG_SINGULAR(DEV_GRP_P1, 0x19, RES_STATE_ACTIVE), 2},
};

static struct twl4030_script wakeup_p3_script __initdata = {
	.script = wakeup_p3_seq,
	.size   = ARRAY_SIZE(wakeup_p3_seq),
	.flags  = TWL4030_WAKEUP3_SCRIPT,
};

static struct twl4030_ins wrst_seq[] __initdata = {
/*
 * Reset twl4030.
 * Reset VDD1 regulator.
 * Reset VDD2 regulator.
 * Reset VPLL1 regulator.
 * Enable sysclk output.
 * Reenable twl4030.
 */
	{MSG_SINGULAR(DEV_GRP_NULL, RES_RESET, RES_STATE_OFF), 2},
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD1, RES_STATE_WRST), 15},
	{MSG_SINGULAR(DEV_GRP_P1, RES_VDD2, RES_STATE_WRST), 15},
	{MSG_SINGULAR(DEV_GRP_P1, RES_VPLL1, RES_STATE_WRST), 0x60},
	{MSG_SINGULAR(DEV_GRP_P1, RES_HFCLKOUT, RES_STATE_ACTIVE), 2},
	{MSG_SINGULAR(DEV_GRP_P1, RES_VAUX2, RES_STATE_ACTIVE), 2},
	{MSG_SINGULAR(DEV_GRP_NULL, RES_RESET, RES_STATE_ACTIVE), 2},
};

static struct twl4030_script wrst_script __initdata = {
	.script = wrst_seq,
	.size   = ARRAY_SIZE(wrst_seq),
	.flags  = TWL4030_WRST_SCRIPT,
};

static struct twl4030_script *twl4030_scripts[] __initdata = {
	&sleep_on_script,
	&wakeup_p12_script,
	&wakeup_p3_script,
	&wrst_script,
};

static struct twl4030_resconfig twl4030_rconfig[] = {
	{ .resource = RES_HFCLKOUT, .devgroup = DEV_GRP_P3, .type = -1,
		.type2 = -1 },
/* XXX removed, breaks booting after power-off
	{ .resource = RES_VDD1, .devgroup = DEV_GRP_P1, .type = -1,
		.type2 = -1 },
	{ .resource = RES_VDD2, .devgroup = DEV_GRP_P1, .type = -1,
		.type2 = -1 },
*/
	{ 0, 0},
};

static struct twl4030_resconfig twl4030_rconfig_vdd_cam_off[] = {
	{ .resource = RES_HFCLKOUT, .devgroup = DEV_GRP_P3, .type = -1,
		.type2 = -1 },
 	{ .resource = RES_VAUX2, .devgroup = DEV_GRP_P1, .type = -1,
		.type2 = -1 },
	{ 0, 0},
};

static struct twl4030_power_data board_t2scripts_data __initdata = {
	.scripts	= twl4030_scripts,
	.num		= ARRAY_SIZE(twl4030_scripts),
	.resource_config = twl4030_rconfig,
};

static struct twl4030_platform_data board_tps65921_pdata = {
	.irq_base	= TWL4030_IRQ_BASE,
	.irq_end	= TWL4030_IRQ_END,

	/* platform_data for children goes here */
	.madc		= &board_madc_data,
	.usb		= &board_usb_data,
	.keypad		= &board_kp_twl4030_data,
	.power		= &board_t2scripts_data,
	.vmmc1          = &board_vmmc1, /* used for VACC :-( */
	.vdac		= &board_vdac,
	.vaux2          = &board_vaux2,
	//.vdd1		= &board_vdd1,
};

/* fixed dummy regulator for 1.8v vdds_dsi rail */
static struct regulator_init_data board_vdds_dsi = {
	.constraints = {
		.valid_ops_mask		= REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &board_vdds_dsi_supply,
};

static struct fixed_voltage_config board_vdds_dsi_fixed = {
	.supply_name = "vdds_dsi",
	.microvolts = 1800000,
	.gpio = -EINVAL,
	.startup_delay = 0,
	.enabled_at_boot = 1,
	.init_data = &board_vdds_dsi,
};

static struct platform_device board_vdds_dsi_device = {
	.name		= "reg-fixed-voltage",
	.id		= 0,
	.dev.platform_data = &board_vdds_dsi_fixed,
};

static struct regulator_consumer_supply board_vmmc_ext_supply = {
	.supply	= "vmmc",
};
static struct regulator_init_data board_vmmc_ext = {
	.constraints = {
		.valid_ops_mask		= REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &board_vmmc_ext_supply,
};

static struct fixed_voltage_config board_vmmc_ext_fixed = {
	.supply_name		= "vmmc",
	.microvolts		= 3000000,
	// Set in archos_mmc1_setup_gpios().
	//.gpio			= -EINVAL,
	// TODO: 100ms is ok?
	.startup_delay		= 100000, /* 100ms */
	.enable_high		= 1,
	.enabled_at_boot	= 0,
	.init_data		= &board_vmmc_ext,
};

static struct platform_device board_vmmc_ext_device = {
	.name		= "reg-fixed-voltage",
	.id		= 1,
	.dev.platform_data = &board_vmmc_ext_fixed,
};

static struct regulator_consumer_supply board_vmmc2_supply = {
	.supply	= "vmmc",
};
static struct regulator_init_data board_vmmc2 = {
	.constraints = {
		.valid_ops_mask		= REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &board_vmmc2_supply,
};

static struct fixed_voltage_config board_vmmc2_fixed = {
	.supply_name = "vmmc",
	.microvolts = 1800000,
	.gpio = -EINVAL,
	.startup_delay = 0,
	.enabled_at_boot = 1,
	.init_data = &board_vmmc2,
};

static struct platform_device board_vmmc2_device = {
	.name		= "reg-fixed-voltage",
	.id		= 2,
	.dev.platform_data = &board_vmmc2_fixed,
};

static struct regulator_consumer_supply board_vmmc3_supply = {
	.supply	= "vmmc",
};
static struct regulator_init_data board_vmmc3 = {
	.constraints = {
		.valid_ops_mask		= REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &board_vmmc3_supply,
};

static struct fixed_voltage_config board_vmmc3_fixed = {
	.supply_name		= "vmmc",
	.microvolts		= 3000000,
	// Set in board init.
	//.gpio			= -EINVAL,
	// TODO: 100ms is ok?
	.startup_delay		= 100000, /* 100ms */
	.enable_high		= 1,
	.enabled_at_boot	= 0,
	.init_data		= &board_vmmc3,
};

static struct platform_device board_vmmc3_device = {
	.name		= "reg-fixed-voltage",
	.id		= 3,
	.dev.platform_data = &board_vmmc3_fixed,
};

static struct i2c_board_info __initdata board_i2c_bus1_info[] = {
	{
		I2C_BOARD_INFO("tps65921", 0x48),	// id to fix
		.flags = I2C_CLIENT_WAKE,
		.irq = INT_34XX_SYS_NIRQ,
		.platform_data = &board_tps65921_pdata,
	},
};

static void archos_hdmi_power(struct i2c_client *cl, int on_off)
{
	struct device *dev = &cl->dev;
	struct archos_hdmi_platform_data *pdata = dev->platform_data;
	
	if (on_off)
		gpio_direction_output(pdata->pwr_gpio, 1);
	else
		gpio_direction_input(pdata->pwr_gpio);
}

static struct archos_hdmi_platform_data board_hdmi_pdata = {
	.hdmi_pwr = archos_hdmi_power,
};


#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {
#if 0
	/* 34xx I2C */
	OMAP3_MUX(/* I2C1_SCL */ I2C1_SCL,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* I2C1_SDA */ I2C1_SDA,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* I2C2_SCL */ I2C2_SCL,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* I2C2_SDA */ I2C2_SDA,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* I2C3_SCL */ I2C3_SCL,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* I2C3_SDA */ I2C3_SDA,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* I2C4_SCL */ I2C4_SCL,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* I2C4_SDA */ I2C4_SDA,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),

	/* PHY - HSUSB: 12-pin ULPI PHY: Port 1*/
	OMAP3_MUX(/* USB1HS_PHY_CLK */ ETK_CTL,
			OMAP_MUX_MODE3 /*| OMAP_PIN_OFF_OUTPUT_LOW*/),
	OMAP3_MUX(/* USB1HS_PHY_STP */ ETK_CLK,
			OMAP_MUX_MODE3 /*| OMAP_PIN_OFF_OUTPUT_LOW*/),
	OMAP3_MUX(/* USB1HS_PHY_DIR */ ETK_D8,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB1HS_PHY_NXT */ ETK_D9,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB1HS_PHY_DATA0 */ ETK_D0,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB1HS_PHY_DATA1 */ ETK_D1,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB1HS_PHY_DATA2 */ ETK_D2,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB1HS_PHY_DATA3 */ ETK_D7,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB1HS_PHY_DATA4 */ ETK_D4,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB1HS_PHY_DATA5 */ ETK_D5,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB1HS_PHY_DATA6 */ ETK_D6,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB1HS_PHY_DATA7 */ ETK_D3,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),

	/* PHY - HSUSB: 12-pin ULPI PHY: Port 2*/
	OMAP3_MUX(/* USB2HS_PHY_CLK */ ETK_D10,
			OMAP_MUX_MODE3 /*| OMAP_PIN_OFF_OUTPUT_LOW*/),
	OMAP3_MUX(/* USB2HS_PHY_STP */ ETK_D11,
			OMAP_MUX_MODE3 /*| OMAP_PIN_OFF_OUTPUT_LOW*/),
	OMAP3_MUX(/* USB2HS_PHY_DIR */ ETK_D12,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB2HS_PHY_NXT */ ETK_D13,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB2HS_PHY_DATA0 */ ETK_D14,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB2HS_PHY_DATA1 */ ETK_D15,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB2HS_PHY_DATA2 */ MCSPI1_CS3,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB2HS_PHY_DATA3 */ MCSPI2_CS1,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB2HS_PHY_DATA4 */ MCSPI2_SIMO,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB2HS_PHY_DATA5 */ MCSPI2_SOMI,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB2HS_PHY_DATA6 */ MCSPI2_CS0,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB2HS_PHY_DATA7 */ MCSPI2_CLK,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),

	/* PHY FSUSB: FS Serial for Port 1 (multiple PHY modes supported), */
	OMAP3_MUX(/* USB1FS_PHY_MM1_RXDP */ ETK_CLK,
			OMAP_MUX_MODE5 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB1FS_PHY_MM1_RXDM */ ETK_D9,
			OMAP_MUX_MODE5 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB1FS_PHY_MM1_RXRCV */ ETK_D0,
			OMAP_MUX_MODE5 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB1FS_PHY_MM1_TXSE0 */ ETK_D1,
			OMAP_MUX_MODE5 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB1FS_PHY_MM1_TXDAT */ ETK_D2,
			OMAP_MUX_MODE5 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB1FS_PHY_MM1_TXEN_N */ ETK_D7,
			OMAP_MUX_MODE5 | OMAP_PIN_OUTPUT),

	/* PHY FSUSB: FS Serial for Port 2 (multiple PHY modes supported), */
	OMAP3_MUX(/* USB2FS_PHY_MM2_RXDP */ ETK_D11,
			OMAP_MUX_MODE5 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB2FS_PHY_MM2_RXDM */ ETK_D13,
			OMAP_MUX_MODE5 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB2FS_PHY_MM2_RXRCV */ ETK_D14,
			OMAP_MUX_MODE5 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB2FS_PHY_MM2_TXSE0 */ ETK_D15,
			OMAP_MUX_MODE5 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB2FS_PHY_MM2_TXDAT */ MCSPI1_CS3,
			OMAP_MUX_MODE5 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB2FS_PHY_MM2_TXEN_N */ MCSPI2_CS1,
			OMAP_MUX_MODE5 | OMAP_PIN_OUTPUT),

	/* PHY FSUSB: FS Serial for Port 3 (multiple PHY modes supported), */
	OMAP3_MUX(/* USB3FS_PHY_MM3_RXDP */ SDMMC2_DAT5,
			OMAP_MUX_MODE6 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB3FS_PHY_MM3_RXDM */ SDMMC2_DAT7,
			OMAP_MUX_MODE6 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB3FS_PHY_MM3_RXRCV */ MCBSP4_DR,
			OMAP_MUX_MODE6 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB3FS_PHY_MM3_TXSE0 */ MCBSP4_CLKX,
			OMAP_MUX_MODE6 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB3FS_PHY_MM3_TXDAT */ MCBSP4_DX,
			OMAP_MUX_MODE6 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* USB3FS_PHY_MM3_TXEN_N */ MCBSP4_FSX,
			OMAP_MUX_MODE6 | OMAP_PIN_OUTPUT),

	/* 34XX GPIO - bidirectional, unless the name has an "_OUT" suffix.
	 * (Always specify PIN_INPUT, except for names suffixed by "_OUT".),
	 * No internal pullup/pulldown without "_UP" or "_DOWN" suffix.
	 */
	OMAP3_MUX(/* GPIO0 */ SYS_NIRQ,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO8 */ SYS_BOOT6,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO9 */ SYS_OFF_MODE,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO10 */ SYS_CLKOUT1,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO28_OUT */ ETK_D14,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO29 */ ETK_D15,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),

	OMAP3_MUX(/* GPIO34 */ GPMC_A1,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* GPIO35 */ GPMC_A2,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* GPIO36 */ GPMC_A3,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* GPIO37 */ GPMC_A4,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* GPIO38 */ GPMC_A5,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* GPIO39 */ GPMC_A6,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* GPIO40 */ GPMC_A7,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* GPIO41 */ GPMC_A8,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* GPIO43_OUT */ GPMC_A10,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO53_OUT */ GPMC_NCS2,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO54_OUT */ GPMC_NCS3,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO54_DOWN */ GPMC_NCS3,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* GPIO55 */ GPMC_NCS4,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO55_OUT */ GPMC_NCS4,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	// a101. USB power.
	OMAP3_MUX(/* GPIO56_OUT */ GPMC_NCS5,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO57_OFF_LOW */ GPMC_NCS6,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* GPIO57 */ GPMC_NCS6,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_HIGH),
	OMAP3_MUX(/* GPIO58_OUT */ GPMC_NCS7,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO59_DOWN */ GPMC_CLK,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* GPIO63 */ GPMC_WAIT1,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO63_OUT */ GPMC_WAIT1,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
#endif
	/* mmc1 card detect */
	// SD detect.
	OMAP3_MUX(/* GPIO65 */ GPMC_WAIT3,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
#if 0
	OMAP3_MUX(/* GPIO86_OUT */ DSS_DATA16,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO92 */ DSS_DATA22,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO94 */ CAM_HS,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO96 */ CAM_XCLKA,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO99 */ CAM_D0,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO100 */ CAM_D1,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO101 */ CAM_D2,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* GPIO101_OUT */ CAM_D2,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO109 */ CAM_D10,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO111 */ CAM_XCLKB,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO134_OUT */ SDMMC2_DAT2,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO136_OUT */ SDMMC2_DAT4,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO137_OUT */ SDMMC2_DAT5,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO138_OUT */ SDMMC2_DAT6,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO139_DOWN */ SDMMC2_DAT7,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* GPIO140_UP */ MCBSP3_DX,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* GPIO141_DOWN */ MCBSP3_DR,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* GPIO142 */ MCBSP3_CLKX,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO143 */ MCBSP3_FSX,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
#endif
	// Display power.
	OMAP3_MUX(/* GPIO157_OUT */ MCBSP1_FSR,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
#if 0
	OMAP3_MUX(/* GPIO49_OUT */ GPMC_D13,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO162 */ MCBSP1_CLKX,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO163 */ UART3_CTS_RCTX,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* GPIO164_OUT */ UART3_RTS_SD,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO167 */ CAM_WEN,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
#endif
	// a43. Audio HP power.
	OMAP3_MUX(/* GPIO170 */ HDQ_SIO,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
#if 0
	OMAP3_MUX(/* GPIO175 */ MCSPI1_CS1,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO176_OUT */ MCSPI1_CS2,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO177 */ MCSPI1_CS3,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO178_DOWN */ MCSPI2_CLK,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* GPIO_179 */ MCSPI2_SIMO,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO180 */ MCSPI2_SOMI,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
#endif
	// a43. HDMI power.
	OMAP3_MUX(/* GPIO181 */ MCSPI2_CS0,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
#if 0
	OMAP3_MUX(/* GPIO186 */ SYS_CLKOUT2,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* GPIO186_OUT */ SYS_CLKOUT2,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),

	/* MMC1 */
	OMAP3_MUX(/* MMC1_CLK */ SDMMC1_CLK,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC1_CMD */ SDMMC1_CMD,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC1_DAT0 */ SDMMC1_DAT0,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC1_DAT1 */ SDMMC1_DAT1,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC1_DAT2 */ SDMMC1_DAT2,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC1_DAT3 */ SDMMC1_DAT3,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC1_DAT4 */ SIM_IO,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC1_DAT5 */ SIM_CLK,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC1_DAT6 */ SIM_PWRCTRL,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC1_DAT7 */ SIM_RST,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),

	/* MMC2 */
	OMAP3_MUX(/* MMC2_CLK */ SDMMC2_CLK,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC2_CMD */ SDMMC2_CMD,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC2_DAT0 */ SDMMC2_DAT0,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC2_DAT1 */ SDMMC2_DAT1,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC2_DAT2 */ SDMMC2_DAT2,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC2_DAT3 */ SDMMC2_DAT3,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC2_DAT4 */ SDMMC2_DAT4,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC2_DAT5 */ SDMMC2_DAT5,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC2_DAT6 */ SDMMC2_DAT6,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MMC2_DAT7 */ SDMMC2_DAT7,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLDOWN),
#endif

	/* MMC3 */
	// WiFi.
	OMAP3_MUX(/* MMC3_CLK */ ETK_CLK,
			OMAP_MUX_MODE2 | OMAP_PIN_INPUT_PULLUP),
#if 0
	OMAP3_MUX(/* MMC3_CMD */ MCSPI1_CS1,
			OMAP_MUX_MODE3 | OMAP_PIN_INPUT_PULLUP),
#endif
	// WiFi.
	OMAP3_MUX(/* MMC3_DAT0 */ ETK_D4,
			OMAP_MUX_MODE2 | OMAP_PIN_INPUT_PULLUP),
	// WiFi.
	OMAP3_MUX(/* MMC3_DAT1 */ ETK_D5,
			OMAP_MUX_MODE2 | OMAP_PIN_INPUT_PULLUP),
	// WiFi.
	OMAP3_MUX(/* MMC3_DAT2 */ ETK_D6,
			OMAP_MUX_MODE2 | OMAP_PIN_INPUT_PULLUP),
	// WiFi.
	OMAP3_MUX(/* MMC3_DAT3 */ ETK_D3,
			OMAP_MUX_MODE2 | OMAP_PIN_INPUT_PULLUP),

#if 0
	/* UART1 */
	OMAP3_MUX(/* UART1_TX */ UART1_TX,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* UART1_RX */ UART1_RX,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* UART1_RTS */ UART1_RTS,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* UART1_CTS */ UART1_CTS,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT),

	/* McSPI */
	OMAP3_MUX(/* McSPI1_CS2 */ MCSPI1_CS2,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),
#endif

	/* DSI */
	// a43. Display DSI. 6 lines DSI_*
	// init dsi bus, should be ever made in boot
	OMAP3_MUX(/* DSI_DX0 */ DSS_DATA0,
			OMAP_MUX_MODE1 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* DSI_DY0 */ DSS_DATA1,
			OMAP_MUX_MODE1 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* DSI_DX1 */ DSS_DATA2,
			OMAP_MUX_MODE1 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* DSI_DY1 */ DSS_DATA3,
			OMAP_MUX_MODE1 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* DSI_DX2 */ DSS_DATA4,
			OMAP_MUX_MODE1 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* DSI_DY2 */ DSS_DATA5,
			OMAP_MUX_MODE1 | OMAP_PIN_INPUT),

#if 0
	OMAP3_MUX(/* SDRC_CKE0 */ SDRC_CKE0,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* SDRC_CKE1 */ SDRC_CKE1,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),

	/* UART2 */
	OMAP3_MUX(/* UART2_TX */ UART2_TX,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* UART2_RX */ UART2_RX,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* UART2_RTS */ UART2_RTS,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* UART2_CTS */ UART2_CTS,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT),

	// a101. Supply 5V.
	OMAP3_MUX(/* GPIO144 */ UART2_CTS,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLDOWN),
#endif

	/* McBSP */
	// Audio.
	OMAP3_MUX(/* MCBSP2_FSX */ MCBSP2_FSX,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN),
	// Audio.
	OMAP3_MUX(/* MCBSP2_CLKX */ MCBSP2_CLKX,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN),
	// Audio.
	OMAP3_MUX(/* MCBSP2_DR */ MCBSP2_DR,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLDOWN),
	// Audio.
	OMAP3_MUX(/* MCBSP2_DX */ MCBSP2_DX,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
#if 0
	OMAP3_MUX(/* MCBSP3_FSX */ MCBSP1_FSX,
			OMAP_MUX_MODE2 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* MCBSP3_CLKX */ MCBSP1_CLKX,
			OMAP_MUX_MODE2 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* MCBSP3_DR */ MCBSP1_DR,
			OMAP_MUX_MODE2 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* MCBSP3_DX */ MCBSP1_DX,
			OMAP_MUX_MODE2 | OMAP_PIN_OUTPUT),
	/* HDQ */
	OMAP3_MUX(/* HDQ_SIO */ HDQ_SIO,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT),

	/* Camera */
	OMAP3_MUX(/* CAM_HS */ CAM_HS,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* CAM_VS */ CAM_VS,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* CAM_XCLKA */ CAM_XCLKA,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* CAM_PCLK */ CAM_PCLK,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* CAM_FLD */ CAM_FLD,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* CAM_D0 */ CAM_D0,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* CAM_D1 */ CAM_D1,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* CAM_D2 */ CAM_D2,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* CAM_D3 */ CAM_D3,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* CAM_D4 */ CAM_D4,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* CAM_D5 */ CAM_D5,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* CAM_D6 */ CAM_D6,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* CAM_D7 */ CAM_D7,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* CAM_D8 */ CAM_D8,
			OMAP_MUX_MODE7),
	OMAP3_MUX(/* CAM_D9 */ CAM_D9,
			OMAP_MUX_MODE7),
	OMAP3_MUX(/* CAM_D10 */ CAM_D10,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* CAM_D11 */ CAM_D11,
			OMAP_MUX_MODE7),
	OMAP3_MUX(/* CAM_XCLKB */ CAM_XCLKB,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* CAM_WEN */ CAM_WEN,
			OMAP_MUX_MODE4),
	OMAP3_MUX(/* CAM_STROBE */ CAM_STROBE,
			OMAP_MUX_MODE7),
	OMAP3_MUX(/* GPMC_WAIT2 */ GPMC_WAIT2,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
#endif
	/* SYS_NIRQ T2 INT1 */
	// enable_board_wakeup_source(void)
	OMAP3_MUX(/* SYS_NIRQ */ SYS_NIRQ,
			OMAP_WAKEUP_EN | OMAP_PIN_INPUT_PULLUP |
			OMAP_MUX_MODE0),

	// a43. Touchscreen IRQ.
	OMAP3_MUX(/* GPIO24 */ ETK_D10,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT ),
	// a43. Display reset.
	// a101. USB Touchscreen reset.
	OMAP3_MUX(/* GPIO25_OUT */ ETK_D11,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),

#if 0
	OMAP3_MUX(/* GPT08 */ MCSPI2_CS1,
			OMAP_MUX_MODE1 ),
	OMAP3_MUX(/* GPIO178 */ MCSPI2_CLK,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
#endif

	/* TSC PWRON */
	// a43. Touchscreen power.
	OMAP3_MUX(/* GPIO179 */ MCSPI2_SIMO,
			OMAP_MUX_MODE4  | OMAP_PIN_INPUT),

#if 0
	// a101. USB OHCI suspend.
	OMAP3_MUX(/* GPIO27 */ ETK_D13,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO28 */ ETK_D14,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	// a101. Audio HP power.
	OMAP3_MUX(/* GPIO22 */ ETK_D8,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
#endif
	// Audio HP detect.
	OMAP3_MUX(/* GPIO109 */ CAM_D10,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),

	// WiFi power.
	OMAP3_MUX(/* GPIO111 */ CAM_XCLKB,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
#if 0
	OMAP3_MUX(/* SAFE_UP */ CAM_XCLKB,
			OMAP_MUX_MODE7 | OMAP_PULL_ENA | OMAP_PULL_UP),
	OMAP3_MUX(/* SAFE_DOWN */ CAM_XCLKB,
			OMAP_MUX_MODE7 | OMAP_PULL_ENA),
#endif


	// WiFi IRQ.
	OMAP3_MUX(/* GPIO114_UP */ CSI2_DX1,
			OMAP_WAKEUP_EN |
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),

	// BT power. "bt kill switch".
	OMAP3_MUX(/* GPIO162 */ MCBSP1_CLKX,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
#if 0
	OMAP3_MUX(/* GPIO115 */ CSI2_DY1,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),

	OMAP3_MUX(/* UART1_TX */ UART1_TX,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* UART1_RTS */ UART1_RTS,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* UART1_CTS */ UART1_CTS,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* UART1_RX */ UART1_RX,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),

	OMAP3_MUX(/* SAFE */ SDMMC2_CLK,
			OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* SAFE */ SDMMC2_CMD,
			OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* SAFE */ SDMMC2_DAT0,
			OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* SAFE */ SDMMC2_DAT1,
			OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* SAFE */ SDMMC2_DAT2,
			OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* SAFE */ SDMMC2_DAT3,
			OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN),

	/* HOST_BT_ENABLE */
	OMAP3_MUX(/* SAFE_PULLUP */ MCBSP1_CLKX,
			OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* SAFE_PULLDOWN */ MCBSP1_CLKX,
			OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN),
#endif

	// a43. Vibrator. Unused pin.
	OMAP3_MUX(/* GPIO23 */ ETK_D9,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLDOWN),
#if 0
	// a101. USB Touchscreen power.
	OMAP3_MUX(/* GPIO26 */ ETK_D12,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
#endif
	// Keys power.
	OMAP3_MUX(/* GPIO64 */ GPMC_WAIT2,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
#if 0
	// TODO: no constant for this address.
	{
		.reg_offset	= /* GPIO127 */ 0xa24,
		.value		= OMAP_MUX_MODE4,
	},
	OMAP3_MUX(/* GPIO161 */ MCBSP1_FSX,
			OMAP_MUX_MODE4 ),
	OMAP3_MUX(/* GPIO159 */ MCBSP1_DR,
			OMAP_MUX_MODE4 ),

	OMAP3_MUX(/* MCSPI2_CLK */ MCSPI2_CLK,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN),
	OMAP3_MUX(/* MCSPI2_SIMO */ MCSPI2_SIMO,
			OMAP_MUX_MODE0 | OMAP_PIN_OFF_INPUT_PULLDOWN ),
	OMAP3_MUX(/* MCSPI2_SOMI */ MCSPI2_SOMI,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_INPUT_PULLDOWN ),
	OMAP3_MUX(/* MCSPI2_CS0 */ MCSPI2_CS0,
			OMAP_MUX_MODE0 | OMAP_PIN_OFF_INPUT_PULLDOWN ),
#endif

	// HDMI interrupt.
	OMAP3_MUX(/* GPIO110 */ CAM_D11,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
#if 0
	// TODO: no constant for this address.
	{
		.reg_offset	= /* GPIO129 */ 0xa2a,
		.value		= OMAP_MUX_MODE4,
	},
	// TODO: no constant for this address.
	{
		.reg_offset	= /* GPIO128 */ 0xa28,
		.value		= OMAP_MUX_MODE4,
	},
#endif
	/* micro_sd_pwron */
	// SD power.
	OMAP3_MUX(/* GPIO158 */ MCBSP1_DX,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLDOWN),
#if 0
	/* mmc1 prewarn */
	OMAP3_MUX(/* GPIO58_IN */ GPMC_NCS7,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
#endif
	// WiFi.
	OMAP3_MUX(/* MMC3_CMD */ ETK_CTL,
			OMAP_MUX_MODE2 | OMAP_PIN_INPUT_PULLUP),
#if 0
	OMAP3_MUX(/* GPIO126 */ CAM_STROBE,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO56_TRISTATE */ GPMC_NCS5,
			OMAP_MUX_MODE7),
#endif

	/* DSS data lines on alternative pins */
	// archos_hdmi_gpio_init(). DSS_DATA18 to DSS_DATA23.
	OMAP3_MUX(/* DSS_DATA0 */ DSS_DATA18,
			OMAP_MUX_MODE3 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA1 */ DSS_DATA19,
			OMAP_MUX_MODE3 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA2 */ DSS_DATA20,
			OMAP_MUX_MODE3 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA3 */ DSS_DATA21,
			OMAP_MUX_MODE3 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA4 */ DSS_DATA22,
			OMAP_MUX_MODE3 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA5 */ DSS_DATA23,
			OMAP_MUX_MODE3 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),

#if 0
	/* Alternative DSS_DATA0 - DSS_DATA5 in safe mode -> pins not used for A28 */
	OMAP3_MUX(/* SAFE */ DSS_DATA18,
			OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* SAFE */ DSS_DATA19,
			OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* SAFE */ DSS_DATA20,
			OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* SAFE */ DSS_DATA21,
			OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* SAFE */ DSS_DATA22,
			OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* SAFE */ DSS_DATA23,
			OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN),

	/* dss data lines on default pins */
	OMAP3_MUX(/* DSS_DATA0 */ DSS_DATA0,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA1 */ DSS_DATA1,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA2 */ DSS_DATA2,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA3 */ DSS_DATA3,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA4 */ DSS_DATA4,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA5 */ DSS_DATA5,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
#endif

	/* DSS functions on default pins */
	// archos_hdmi_gpio_init(). DSS_HSYNC to DSS_DATA17.
	OMAP3_MUX(/* DSS_HSYNC */ DSS_HSYNC,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_VSYNC */ DSS_VSYNC,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_PCLK */ DSS_PCLK,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_ACBIAS */ DSS_ACBIAS,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA6 */ DSS_DATA6,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA7 */ DSS_DATA7,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA8 */ DSS_DATA8,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA9 */ DSS_DATA9,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA10 */ DSS_DATA10,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA11 */ DSS_DATA11,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA12 */ DSS_DATA12,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA13 */ DSS_DATA13,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA14 */ DSS_DATA14,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA15 */ DSS_DATA15,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA16 */ DSS_DATA16,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA17 */ DSS_DATA17,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),

	/* DSS functions on sysboot pins */
	// archos_hdmi_gpio_init(). SYS_BOOT0 to SYS_BOOT6.
	OMAP3_MUX(/* DSS_DATA18 */ SYS_BOOT0,
			OMAP_MUX_MODE3 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA19 */ SYS_BOOT1,
			OMAP_MUX_MODE3 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA20 */ SYS_BOOT3,
			OMAP_MUX_MODE3 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA21 */ SYS_BOOT4,
			OMAP_MUX_MODE3 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA22 */ SYS_BOOT5,
			OMAP_MUX_MODE3 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	OMAP3_MUX(/* DSS_DATA23 */ SYS_BOOT6,
			OMAP_MUX_MODE3 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),

#if 0
	OMAP3_MUX(/* GPT08 */ GPMC_NCS7,
			OMAP_MUX_MODE3),
	OMAP3_MUX(/* GPT08_OFF_HIGH */ GPMC_NCS7,
			OMAP_MUX_MODE3 | OMAP_PIN_OFF_OUTPUT_HIGH),
	OMAP3_MUX(/* GPT08 */ UART2_RX,
			OMAP_MUX_MODE2),
	// a101. Display reset.
	OMAP3_MUX(/* GPIO14 */ ETK_D0,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),

	// a101. HDMI power.
	OMAP3_MUX(/* GPIO21 */ ETK_D7,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	// a101. Touchscreen power.
	OMAP3_MUX(/* GPIO160 */ MCBSP_CLKS,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	// a101. Touchscreen IRQ.
	OMAP3_MUX(/* GPIO156 */ MCBSP1_CLKR,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO15 */ ETK_D1,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO16 */ ETK_D2,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),

	/* Sys_clkout2 */
	OMAP3_MUX(/* CLKOUT2 */ SYS_CLKOUT2,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
#endif
	// Audio clock.
	OMAP3_MUX(/* CLKOUT2 */ SYS_CLKOUT2,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),

#if 0
	/* USB to sata bridge */
	OMAP3_MUX(/* GPIO120 */ SDMMC1_CLK,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO121 */ SDMMC1_CMD,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO122 */ SDMMC1_DAT0,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO123 */ SDMMC1_DAT1,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO124 */ SDMMC1_DAT2,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO125 */ SDMMC1_DAT3,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
#endif

	/* Bluetooth digital audio interface */
	// Audio + BT?
	OMAP3_MUX(/* MCBSP3_DX */ MCBSP3_DX,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	// Audio + BT?
	OMAP3_MUX(/* MCBSP3_DR */ MCBSP3_DR,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),
	// Audio + BT?
	OMAP3_MUX(/* MCBSP3_CLX */ MCBSP3_CLKX,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),
	// Audio + BT?
	OMAP3_MUX(/* MCBSP3_FSX */ MCBSP3_FSX,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),

	/* HDMI digital audio interface */
	// Audio + HDMI?
	OMAP3_MUX(/* MCBSP4_DX */ MCBSP4_DX,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT | OMAP_PIN_OFF_OUTPUT_LOW),
	// Audio + HDMI?
	OMAP3_MUX(/* MCBSP4_DR */ MCBSP4_DR,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),
	// Audio + HDMI?
	OMAP3_MUX(/* MCBSP4_CLX */ MCBSP4_CLKX,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),
	// Audio + HDMI?
	OMAP3_MUX(/* MCBSP4_FSX */ MCBSP4_FSX,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),


#if 0
	/* 3611 GPIO */
	OMAP3_MUX(/* GPIO157 */ MCBSP1_FSR,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO49 */ GPMC_D13,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPT08 */ MCSPI2_CS1,
			OMAP_MUX_MODE1 ),
	OMAP3_MUX(/* GPIO24 */ ETK_D10,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO51 */ GPMC_D15,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO111 */ CAM_XCLKB,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO27 */ ETK_D13,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO28 */ ETK_D14,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO22 */ ETK_D8,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO96 */ CAM_XCLKA,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO114 */ CSI2_DX1,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO156 */ CAM_XCLKA,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* GPIO115 */ CSI2_DY1,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO50 */ GPMC_D14,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* UART2_TX */ MCBSP3_CLKX,
			OMAP_MUX_MODE1 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* UART2_RX */ MCBSP3_FSX,
			OMAP_MUX_MODE1 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLUP),
	OMAP3_MUX(/* GPIO47 */ GPMC_D11,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),

	OMAP3_MUX(/* GPIO25 */ ETK_D11,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT),
	OMAP3_MUX(/* GPIO26 */ ETK_D12,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),

	OMAP3_MUX(/* MCBSP3_DX */ MCBSP3_DX,
			OMAP_MUX_MODE0 | OMAP_PIN_OUTPUT),
	OMAP3_MUX(/* MCBSP3_DR */ MCBSP3_DR,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* MCBSP3_CLK */ MCBSP3_CLKX,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(/* MCBSP3_FSX */ MCBSP3_FSX,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),

	// a101. Display pci(?), blk_en(?).
	OMAP3_MUX(/* GPIO146 */ UART2_TX,
			OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),
#endif
	// compa_rst, compass? Unused pin.
	OMAP3_MUX(/* GPIO145 */ UART2_RTS,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLDOWN),

#if 0
	// OHCI muxes are set up in usb-ehci.c
	// a101. USB OHCI.
	OMAP3_MUX(/* USB2FS_RCV */ ETK_D14,
			OMAP_MUX_MODE5 | OMAP_PIN_INPUT),
	// a101. USB OHCI.
	OMAP3_MUX(/* USB2FS_VM */ ETK_D15,
			OMAP_MUX_MODE5 | OMAP_PIN_INPUT),
	// a101. USB OHCI.
	OMAP3_MUX(/* USB2FS_VP */ MCSPI1_CS3,
			OMAP_MUX_MODE5 | OMAP_PIN_INPUT),
	// a101. USB OHCI.
	OMAP3_MUX(/* USB2FS_EN */ MCSPI2_CS1,
			OMAP_MUX_MODE5 | OMAP_PIN_INPUT),
#endif

	// board_init().
	OMAP3_MUX(/* UART3_RX_IRRX */ UART3_RX_IRRX,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_INPUT_PULLUP),

	// TODO: ok to keep as default?
	// board_offmode_config(void) "PWREN2 test" for rev >= 3.
	OMAP3_MUX(/* GPIO161_OFF_IPU */ MCBSP1_FSX,
			OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_INPUT_PULLUP),

#if 0
	OMAP3_MUX(/* UART1_RX */ UART1_RX,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(/* UART1_CTS */ UART1_CTS,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),

	OMAP3_MUX(/* RFBI_TE */ DSS_DATA16,
			OMAP_MUX_MODE0 | OMAP_PIN_INPUT ),

	OMAP3_MUX(/* GPT09 */ UART2_CTS,
			OMAP_MUX_MODE2),
	// AB26_3630_GPT09_OFF_HIGH mux has been added to Archos kernel recently.
	OMAP3_MUX(/* GPT10 */ UART2_RTS,
			OMAP_MUX_MODE2),

	OMAP3_MUX(/* GPIO147 */ UART2_RX,
			OMAP_MUX_MODE4),
#endif

	{ .reg_offset = OMAP_MUX_TERMINATOR },
};
#else
#define board_mux	NULL
#endif


static void __init board_init_irq(void)
{
	omap_board_config = board_config;
	omap_board_config_size = ARRAY_SIZE(board_config);

	omap2_init_common_hw(edk2132c2pd_50_sdrc_params, 
			edk2132c2pd_50_sdrc_params);
	omap_init_irq();
	omap_gpio_init();
}


static struct i2c_board_info __initdata board_i2c_bus2_info[] = {
	[0] = {
		I2C_BOARD_INFO("wm8988", 0x1a),
		.flags = I2C_CLIENT_WAKE,
	},
	[1] = {
		I2C_BOARD_INFO("tda998X", 0x70),
		.flags = I2C_CLIENT_WAKE,
		.platform_data = &board_hdmi_pdata,
	},
	[2] = {
		I2C_BOARD_INFO("tda99Xcec", 0x34),
		.flags = I2C_CLIENT_WAKE,
		.platform_data = &board_hdmi_pdata,
	}
};

static struct i2c_board_info __initdata board_i2c_bus3_info[] = {
	// TODO:
	//{
		//I2C_BOARD_INFO("mma7660fc", 0x4c),	// to fix
		//.flags = I2C_CLIENT_WAKE,
		//.platform_data = &board_mma7660fc_pdata,
	//},
	{
		I2C_BOARD_INFO("akm8973", 0x1c),
		.flags = I2C_CLIENT_WAKE,
	},
};

static int __init omap_i2c_init(void)
{
	omap_register_i2c_bus(1, 400, board_i2c_bus1_info,
			ARRAY_SIZE(board_i2c_bus1_info));
	omap_register_i2c_bus(2, 100, board_i2c_bus2_info,
			ARRAY_SIZE(board_i2c_bus2_info));
	omap_register_i2c_bus(3, 100, board_i2c_bus3_info,
			ARRAY_SIZE(board_i2c_bus3_info));

	return 0;
}

static struct platform_device sgx_device = {
	.name		= "pvrsrvkm",
	.id		= -1,
};

static struct platform_device *board_devices[] __initdata = {
	&board_vdds_dsi_device,
	&board_vmmc_ext_device,
	&board_vmmc2_device,
	&board_vmmc3_device,
#ifdef CONFIG_OMAP2_DSS
	&board_dss_device,
	// TODO:
	//&board_vout_device,
#endif
#ifdef CONFIG_WL127X_RFKILL
	&board_wl127x_device,
#endif
	&sgx_device,
};

static void __init archos_opp_init(void)
{
	int r = 0;
	struct omap_hwmod *hwm;
	struct device *dev;

	r = omap3_opp_init();
	/*
	 * FIXME: We are now called with late_initcall() and
	 * thus omap3_opp_init() has already been run once,
	 * so we will get -EEXIST. This check should be removed
	 * when our backport of opp/dvfs code gets updated
	 * and late_initcall removed.
	 */
	if (r && r != -EEXIST) {
		pr_err("%s: failed to init default opp table: %d\n", __func__, r);
		return;
	}

	hwm = omap_hwmod_lookup("mpu");
	if (!hwm) {
		pr_err("%s: omap_hwmod_lookup failed\n", __func__);
		return;
	}
	dev = &hwm->od->pdev.dev;
	r = opp_enable(dev, 800000000);
	if (r) {
		pr_err("%s: failed to enable opp, err: %d\n", __func__, r);
	}
	r = opp_enable(dev, 1000000000);
	if (r) {
		pr_err("%s: failed to enable opp, err: %d\n", __func__, r);
	}

	hwm = omap_hwmod_lookup("iva");
	if (!hwm) {
		pr_err("%s: omap_hwmod_lookup failed\n", __func__);
		return;
	}
	dev = &hwm->od->pdev.dev;
	r = opp_enable(dev, 660000000);
	if (r) {
		pr_err("%s: failed to enable opp, err: %d\n", __func__, r);
	}
	r = opp_enable(dev, 800000000);
	if (r) {
		pr_err("%s: failed to enable opp, err: %d\n", __func__, r);
	}
}
/*
 * FIXME: using late_initcall to get this working with current backported
 * version of opp/dvfs support. Currently archos_opp_init will fail if run
 * from board_init due to some hwmod related inits that are not yet done.
 * Move to normal call in board_init when updating the backport.
 */
late_initcall(archos_opp_init);

static void __init archos_hdmi_gpio_init(
		const struct archos_disp_conf* disp_conf)
{
	archos_gpio_init_output(&disp_conf->hdmi_pwr, "hdmi pwr");
	/* FIXME: make userspace configurable */
	gpio_set_value(GPIO_PIN(disp_conf->hdmi_pwr), 1);

	/* patch power gpio into platform data */
	board_hdmi_pdata.pwr_gpio = GPIO_PIN(disp_conf->hdmi_pwr);
	
	/* patch IRQ into HDMI I2C bus info */
	board_i2c_bus2_info[1].irq = gpio_to_irq(GPIO_PIN(disp_conf->hdmi_int));
	board_i2c_bus2_info[2].irq = gpio_to_irq(GPIO_PIN(disp_conf->hdmi_int));
}

static int __init wl127x_vio_leakage_fix(void)
{
	int ret = 0;
	const struct archos_wifi_bt_config *conf = &wifi_bt_dev_conf;
	struct archos_gpio bten_gpio;
	
	if (hardware_rev >= conf->nrev)
		return -ENODEV;
	
	bten_gpio = conf->rev[hardware_rev].bt_power;
	
	ret = gpio_request(GPIO_PIN(bten_gpio), "wl127x_bten");
	if (ret < 0) {
		printk(KERN_ERR "wl127x_bten gpio_%d request fail",
						GPIO_PIN(bten_gpio));
		goto fail;
	}

	gpio_direction_output(GPIO_PIN(bten_gpio), 1);
	mdelay(10);
	gpio_direction_output(GPIO_PIN(bten_gpio), 0);
	udelay(64);

	gpio_free(GPIO_PIN(bten_gpio));

fail:
	return ret;
}

static struct omap2_hsmmc_info mmc[] __initdata = {
	{
		// SD slot.
		.mmc		= 1,
		.caps		= MMC_CAP_4_BIT_DATA,
		.gpio_wp	= -EINVAL,
	},
	{
		// eMMC.
		.mmc		= 2,
		.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA,
		.gpio_wp	= -EINVAL,
		.gpio_cd	= -EINVAL,
		.ocr_mask	= MMC_VDD_165_195,
		.nonremovable	= true,
	},
	{
		// WiFi.
		.mmc		= 3,
		.caps		= MMC_CAP_4_BIT_DATA | MMC_CAP_POWER_OFF_CARD,
		.gpio_wp	= -EINVAL,
		.gpio_cd	= -EINVAL,
		.nonremovable	= true,
	},
	{}      /* Terminator */
};

#ifdef CONFIG_CLOCKS_INIT
static struct archos_clocks board_clocks __initdata = {
    .dpll4 = {
        .rate = 1728000000,
        .m2_rate = 192000000,
        .m3_rate = 54000000,
        .m4_rate = 172800000,
        .m5_rate = 216000000,
        .m6_rate = 288000000,
    },
};
#endif

static int archos_mmc1_setup_gpios(struct omap2_hsmmc_info *c)
{
 	const struct archos_sd_config *hsmmc_cfg;
	struct archos_gpio gpio_hsmmc_power, gpio_hsmmc_cd,gpio_hsmmc_cpd;
	
	/* sd */
	hsmmc_cfg = omap_get_config( ARCHOS_TAG_SD, struct archos_sd_config );
	if (hsmmc_cfg == NULL) {
		printk(KERN_DEBUG "%s: no board configuration found\n", __FUNCTION__);
		return -ENODEV;
	}
	if ( hardware_rev >= hsmmc_cfg->nrev ) {
		printk(KERN_DEBUG "%s: hardware_rev (%i) >= nrev (%i)\n",
			__FUNCTION__, hardware_rev, hsmmc_cfg->nrev);
		return -ENODEV;
	}
	
	printk(KERN_DEBUG "%s\n", __FUNCTION__);

	gpio_hsmmc_power = hsmmc_cfg->rev[hardware_rev].sd_power;
	board_vmmc_ext_fixed.gpio = GPIO_PIN(gpio_hsmmc_power);
	
	gpio_hsmmc_cd = hsmmc_cfg->rev[hardware_rev].sd_detect;
	gpio_hsmmc_cpd = hsmmc_cfg->rev[hardware_rev].sd_prewarn;
	
	/* card detect */
	c->gpio_cd = GPIO_PIN(gpio_hsmmc_cd);	

	/* Need to be here, before omap2_init_mmc which will correctly set the IRQ stuff */
	archos_gpio_init_input(&gpio_hsmmc_cd, "SD_DETECT");
	gpio_free(c->gpio_cd);
	
	return 0;
}

static void board_offmode_config(void)
{
	/* mass production boards have a proper power supply for the 26MHz XO
	 * and we can turn off the +1V8 supply rail in OFF mode */
	if (hardware_rev >= 3) {
		gpio_request(161, "pwren2");
		gpio_direction_input(161); // we have a pull-down while active and a pull-up while in OFF mode

		/* if we can turn off the 1.8V, we need to turn off the VDD_CAM, too */
		board_tps65921_pdata.power->resource_config = twl4030_rconfig_vdd_cam_off;
		
	}
}

static void init_unused(void)
{
	gpio_request(23, "vibrate");
	gpio_direction_input(23); // we have a pull-down

	gpio_request(145, "compa_rst");
	gpio_direction_input(145); // we have a pull-down
}

#ifdef CONFIG_CLOCKS_INIT
extern int __init archos_clocks_init(struct archos_clocks *clocks);
#endif  

static void __init board_init(void)
{
	int num_displays = 0;

	// TODO: Is it CBC?
	omap3_mux_init(board_mux, OMAP_PACKAGE_CBC);

	/* initialize gpios reserved for unsoldered parts */
	init_unused();

#ifdef CONFIG_CLOCKS_INIT
	archos_clocks_init(&board_clocks);
#endif  
	
	/* offmode config, before I2C config! */
	board_offmode_config();
	
	/* before omap_i2c_init() or IRQ will not forwarded to driver */
	if (display_config.nrev > hardware_rev)
		archos_hdmi_gpio_init(&display_config.rev[hardware_rev]);
	
	omap_i2c_init();
	/* Fix to prevent VIO leakage on wl127x */
	wl127x_vio_leakage_fix();

#if defined CONFIG_OMAP2_DSS
	if (archos_lcd_panel_init(&board_lcd_device) == 0) {
		board_dss_devices[num_displays++] = &board_lcd_device;
		board_dss_data.default_device = &board_lcd_device;
	}
	// TODO:
	//board_dss_devices[num_displays++] = &board_hdmi_device;
#ifdef CONFIG_OMAP2_DSS_DUMMY
	board_dss_devices[num_displays++] = &board_dummy_device;
	board_dss_data.default_device = &board_dummy_device;
#endif /* CONFIG_OMAP2_DSS_DUMMY */
	board_dss_data.num_devices = num_displays;
#endif/* CONFIG_OMAP2_DSS */

	platform_add_devices(board_devices, ARRAY_SIZE(board_devices));

	msecure_init();
	omap_serial_init();
	// TODO: it can supply 500mA?
	archos_usb_musb_init();

	//archos_accel_init(&board_mma7660fc_pdata);

	ads7846_dev_init();

	archos_mmc1_setup_gpios(&mmc[0]);
	// TODO: get rid of magic number, this is WiFi power gpio.
	board_vmmc3_fixed.gpio = 111;
	omap2_hsmmc_init(mmc);
	board_vmmc_ext_supply.dev = mmc[0].dev;
	board_vmmc2_supply.dev = mmc[1].dev;
	board_vmmc3_supply.dev = mmc[2].dev;

	archos_wifi_init();
	archos_audio_gpio_init();

	//archos_camera_mt9d113_init();
	//archos_leds_init();
	//archos_compass_init();
}

MACHINE_START(ARCHOS_A43, "Archos A43 board")
	.boot_params	= 0x80000100,
	.map_io		= omap3_map_io,
	.reserve	= omap_reserve,
	.init_irq	= board_init_irq,
	.init_machine	= board_init,
	.timer		= &omap_timer,
MACHINE_END
