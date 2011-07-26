/* linux/arch/arm/mach-omap2/archos-wifi.c
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/err.h>

#include <linux/skbuff.h>
#include <linux/ti_wilink_st.h>
#include <linux/wl12xx.h>

#include <asm/gpio.h>
#include <asm/io.h>

#include <plat/board.h>
#include <mach/board-archos.h>

static int archos_wifi_pa_type = 0;
static struct archos_gpio wifi_pmena;
static int archos_wifi_power_state;

static int archos_wifi_power(int on)
{
	printk("%s: %d\n", __func__, on);
	gpio_set_value(GPIO_PIN(wifi_pmena), on);
	archos_wifi_power_state = on;
	return 0;
}

static ssize_t store_wifi_reset(struct device *dev,
		struct device_attribute *addr, const char *buf, size_t count)
{
	int on_off = simple_strtol(buf, NULL, 10);
	
	if( on_off > 1 || on_off < -1 )
		return -EINVAL;

	archos_wifi_power(on_off);

	return count;
}
static DEVICE_ATTR(wifi_reset, S_IRUGO|S_IWUSR, NULL, store_wifi_reset);

static ssize_t show_wifi_pa_type(struct device *dev, 
		struct device_attribute *attr, char *buf)
{
	switch (archos_wifi_pa_type) {
		case PA_TYPE_TQM67002_NEW_BOM:
			return snprintf(buf, PAGE_SIZE, "%s\n", "TQM67002_NEW_BOM");
		case PA_TYPE_TQM67002A:
			return snprintf(buf, PAGE_SIZE, "%s\n", "TQM67002A");
		case PA_TYPE_RF3482:
			return snprintf(buf, PAGE_SIZE, "%s\n", "RF3482");
		default:
			return snprintf(buf, PAGE_SIZE, "%s\n", "TQM67002");
	}
};
static DEVICE_ATTR(wifi_pa_type, S_IRUGO|S_IWUSR, show_wifi_pa_type, NULL);


static struct platform_device archos_wifi_device = {
        .name           = "device_wifi",
        .id             = -1,
};

static struct wl12xx_platform_data archos_wlan_data __initdata = {
	.board_ref_clock = 2,
};

/* wl127x BT, FM, GPS connectivity chip */
static struct ti_st_plat_data wl1271_chip_plat_data = {
	// TODO: are these fields used?
	.dev_name = "kim_device",
	.flow_cntrl = 0,
	.baud_rate = 0,
};
static struct platform_device wl1271_chip_device = {
	.name = "kim",		/* named after init manager for ST */
	.id = -1,
	.dev.platform_data = &wl1271_chip_plat_data,
};

static struct platform_device bt_device = {
	.name = "btwilink",
	.id = -1,
};

int __init archos_wifi_init(void)
{
	int ret;
	const struct archos_wifi_bt_config *wifi_bt_cfg;
	struct archos_gpio wifi_irq;
	struct archos_gpio bt_nshutdown;
	
	wifi_bt_cfg = omap_get_config(ARCHOS_TAG_WIFI_BT, struct archos_wifi_bt_config);
	
	/* might be NULL */
	if (wifi_bt_cfg == NULL) {
		printk(KERN_DEBUG "archos_wifi_init: no board configuration found\n");
		return -ENODEV;
	}
	if ( hardware_rev >= wifi_bt_cfg->nrev ) {
		printk(KERN_DEBUG "archos_wifi_init: hardware_rev (%i) >= nrev (%i)\n",
			hardware_rev, wifi_bt_cfg->nrev);
		return -ENODEV;
	}

	wifi_irq = wifi_bt_cfg->rev[hardware_rev].wifi_irq;
	wifi_pmena = wifi_bt_cfg->rev[hardware_rev].wifi_power;
	archos_wifi_pa_type = wifi_bt_cfg->rev[hardware_rev].wifi_pa_type;

	printk("%s: start\n", __func__);
	ret = gpio_request(GPIO_PIN(wifi_irq), "wifi_irq");
	if (ret < 0) {
		printk(KERN_ERR "%s: can't reserve GPIO: %d\n", __func__,
			GPIO_PIN(wifi_irq));
		goto out;
	}
	gpio_direction_input(GPIO_PIN(wifi_irq));

	archos_wlan_data.irq = gpio_to_irq(GPIO_PIN(wifi_irq));
	if (wl12xx_set_platform_data(&archos_wlan_data)) {
		pr_err("archos_wifi_bt_init: error setting wl12xx data\n");
		return -1;
	}

	bt_nshutdown = wifi_bt_cfg->rev[hardware_rev].bt_power;
	if (GPIO_EXISTS(bt_nshutdown)) {
		wl1271_chip_plat_data.nshutdown_gpio = GPIO_PIN(bt_nshutdown);
		ret = platform_device_register(&wl1271_chip_device);
		if (ret < 0)
			goto out;
	}

	ret = platform_device_register(&bt_device);
	if (ret < 0) {
		pr_err("archos_wifi_bt_init: couldn't register bt device\n");
		return -1;
	}

	ret = platform_device_register(&archos_wifi_device);
	if (ret < 0)
		goto out;
	
	ret = device_create_file(&archos_wifi_device.dev, &dev_attr_wifi_reset);
	if (ret < 0)
		goto out;

	ret = device_create_file(&archos_wifi_device.dev, &dev_attr_wifi_pa_type);

out:
        return ret;
}

