#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/hrtimer.h>
#include <asm/mach-types.h>

#include <plat/board.h>
#include <plat/mcbsp.h>

#include <mach/gpio.h>
#include <mach/archos-audio.h>
#include <mach/archos-gpio.h>
#include <mach/board-archos.h>

#include "clock.h"

static struct archos_audio_conf audio_gpio;

static char *sysclock_name = NULL;
static char sys_clkout1_name[] = "sys_clkout1";
static char sys_clkout2_name[] = "sys_clkout2";

/* WORKAROUND: There is no way to keep CM_96M_FCLK (the sys_clkout2 source)
 * alive when every module which depends on it is idle, So we have to keep
 * the McBSP1 fclk ON.
 */
int use_mcbsp1_fclk = 0;

int use_vamp_usb = 0;
int vamp_enabled = 0;

struct hrtimer vamp_watchdog_timer;
int vamp_watchdog_state;

static enum hrtimer_restart vamp_watchdog_timer_func(struct hrtimer *timer)
{
	switch(vamp_watchdog_state) {
		case 1:
			// pulse down to allow usb de-plug detection...
			gpio_set_value( GPIO_PIN( audio_gpio.vamp_dc), 0);

			// and re-enable it during next iteration, 1ms later.
			hrtimer_start(&vamp_watchdog_timer,
					ktime_set( 0, 1 * 1E6L),
					HRTIMER_MODE_REL);
			vamp_watchdog_state = 0;
			break;

		default:
			if (use_vamp_usb) {
				// if using vamp_usb, schedule trigger pulse
				// down during next iteration.
				gpio_set_value( GPIO_PIN( audio_gpio.vamp_dc), 1);
				vamp_watchdog_state = 1;
			} else {
				// else, just secure disabling of vamp_dc
				gpio_set_value( GPIO_PIN( audio_gpio.vamp_dc), 0);
			}

			hrtimer_start(&vamp_watchdog_timer,
					ktime_set( 0, 1000 * 1E6L),
					HRTIMER_MODE_REL);
			break;
	}

	return HRTIMER_NORESTART;
}

static void power_watchdog_ctrl(int start) {
	if (start) {
		vamp_watchdog_state = 0;
		hrtimer_start(&vamp_watchdog_timer, ktime_set( 0, 1 * 1E6L), HRTIMER_MODE_REL);
	} else {
		hrtimer_cancel(&vamp_watchdog_timer);
	}
}

static void _set_ampli(int onoff)
{
	if (GPIO_PIN( audio_gpio.spdif ) < 0) {
		pr_debug("No SPDIF in this device !\n");
		return;
	}

	if (onoff)
		gpio_set_value( GPIO_PIN( audio_gpio.spdif), 0);
	else
		gpio_set_value( GPIO_PIN( audio_gpio.spdif), 1);
}

static void _set_vamp(int onoff)
{
	if (GPIO_PIN( audio_gpio.vamp_vbat ) < 0) {
		pr_debug("No Vamp config in this device !\n");
		return;
	}

	vamp_enabled = onoff;
	
	if (onoff){
		// we keep vamp_vbat always on : it's safe electrically
		// and allows us to disable vamp_dc from time to time.
		gpio_set_value( GPIO_PIN( audio_gpio.vamp_vbat), 1);
		gpio_set_value( GPIO_PIN( audio_gpio.vamp_dc), 0);

		if ( GPIO_PIN( audio_gpio.vamp_dc ) > 0)
			power_watchdog_ctrl(1);
	} else {
		gpio_set_value( GPIO_PIN( audio_gpio.vamp_vbat), 0);

		if ( GPIO_PIN( audio_gpio.vamp_dc ) > 0) {
			power_watchdog_ctrl(0);
			gpio_set_value( GPIO_PIN( audio_gpio.vamp_dc), 0);
		}
	}
}

static void _set_hp(int onoff)
{
	if (GPIO_PIN( audio_gpio.hp_on ) < 0) {
		pr_debug("No Speaker in this device !\n");
		return;
	}

	if (onoff) {
		_set_vamp(onoff);
		msleep(100);
		gpio_set_value( GPIO_PIN( audio_gpio.hp_on), 1);
	} else {
		gpio_set_value( GPIO_PIN( audio_gpio.hp_on), 0);
		msleep(10);
		_set_vamp(onoff);
	}
}

static int _get_headphone_plugged(void)
{
	if (GPIO_PIN( audio_gpio.headphone_plugged ) < 0) {
		pr_debug("No Headphone detection in this device !\n");
		return -1;
	}

	return gpio_get_value( GPIO_PIN( audio_gpio.headphone_plugged) );
}

static int _get_headphone_irq(void)
{
	if (GPIO_PIN( audio_gpio.headphone_plugged ) < 0) {
		pr_debug("No Headphone detection in this device !\n");
		return -1;
	}

	return gpio_to_irq(GPIO_PIN( audio_gpio.headphone_plugged));
}

static void _sys_clkout_en(int en)
{
	struct clk *sys_clkout;
	struct omap_mcbsp *mcbsp;

	// TODO: port properly.
	mcbsp = id_to_mcbsp_ptr(0);

	sys_clkout = clk_get(NULL, sysclock_name);
	if (!IS_ERR(sys_clkout)) {
		if (en) {
			if (use_mcbsp1_fclk) {
				clk_enable(mcbsp->fclk);
			}
			if (clk_enable(sys_clkout) != 0) {
				printk(KERN_ERR "failed to enable %s\n", sysclock_name);
			}
		} else {
			clk_disable(sys_clkout);
			if (use_mcbsp1_fclk) {
				clk_disable(mcbsp->fclk);
			}
		}

		clk_put(sys_clkout);
	}
}

static int _get_clkout_rate(void)
{
	struct clk *sys_clkout;
	int rate;

	sys_clkout = clk_get(NULL, sysclock_name);
	if (IS_ERR(sys_clkout)) {
		printk(KERN_ERR "failed to get %s\n", sysclock_name);
	}
	rate = clk_get_rate(sys_clkout);
	clk_put(sys_clkout);

	return rate;
}

static void _suspend(void)
{
	if (GPIO_PIN(audio_gpio.headphone_plugged) != -1)
		gpio_set_debounce(GPIO_PIN(audio_gpio.headphone_plugged), 0);
}

static void _resume(void)
{
	if (GPIO_PIN(audio_gpio.headphone_plugged) != -1)
		gpio_set_debounce(GPIO_PIN(audio_gpio.headphone_plugged), (1 + 1) * 0x1f);
}

static struct audio_device_config audio_device_io = {
	.set_spdif = &_set_ampli,
	.get_headphone_plugged =&_get_headphone_plugged,
	.get_headphone_irq =&_get_headphone_irq,
	.set_speaker_state = &_set_hp,
	.set_codec_master_clk_state = &_sys_clkout_en,
	.get_master_clock_rate = &_get_clkout_rate,
	.suspend = &_suspend,
	.resume = &_resume,
};

struct audio_device_config *archos_audio_get_io(void) {
		return &audio_device_io;
}

static ssize_t show_audio_vamp_vusb_ctrl(struct device *dev, 
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", use_vamp_usb);
};

static ssize_t store_audio_vamp_vusb_ctrl(struct device *dev, 
		struct device_attribute *addr, const char *buf, size_t count)
{
	int on_off = simple_strtol(buf, NULL, 10);

	if( on_off > 1 || on_off < -1 )
		return -EINVAL;

	// update status
	use_vamp_usb = on_off;
	// and apply config.
	_set_vamp(vamp_enabled);
	
	return count;
}
static DEVICE_ATTR(vamp_vusb_ctrl, S_IRUGO|S_IWUSR, show_audio_vamp_vusb_ctrl, store_audio_vamp_vusb_ctrl);

static struct platform_driver archos_audio_vamp_driver = {
	.driver = {
		.name = "archos_audio_vamp",
	},
};

static struct platform_device archos_audio_vamp_device = {
	.name = "archos_audio_vamp",
	.id = -1,
};

int __init archos_audio_gpio_init(void)
{
	const struct archos_audio_config *audio_cfg;
	struct clk *clkout2_src_ck;
	struct clk *sys_clkout2;
	struct clk *core_ck;

	/* audio  */
	audio_cfg = omap_get_config( ARCHOS_TAG_AUDIO, struct archos_audio_config );
	if (audio_cfg == NULL) {
		pr_err("archos_audio_gpio_init: no board configuration found\n");
		return -ENODEV;
	}
	if ( hardware_rev >= audio_cfg->nrev ) {
		pr_err("archos_audio_gpio_init: hardware_rev (%i) >= nrev (%i)\n",
			hardware_rev, audio_cfg->nrev);
		return -ENODEV;
	}

	audio_gpio = audio_cfg->rev[hardware_rev];

	// a32 & a43 protos where using clkout1.
	if (hardware_rev >= 1 || !(machine_is_archos_a32() || machine_is_archos_a43())) {
		core_ck = clk_get(NULL, "cm_96m_fck");
		if (IS_ERR(core_ck)) {
			printk(KERN_ERR "failed to get core_ck\n");
		}
	
		clkout2_src_ck = clk_get(NULL, "clkout2_src_ck");
		if (IS_ERR(clkout2_src_ck)) {
			printk(KERN_ERR "failed to get clkout2_src_ck\n");
		}
	
		sys_clkout2 = clk_get(NULL, "sys_clkout2");
		if (IS_ERR(sys_clkout2)) {
			printk(KERN_ERR "failed to get sys_clkout2\n");
		}
	
		if ( clk_set_parent(clkout2_src_ck, core_ck) != 0) {
			printk(KERN_ERR "failed to set sys_clkout2 parent to clkout2\n");
		}
	
		/* Set the clock to 12 Mhz */
		omap2_clksel_set_rate(sys_clkout2, 12000000);

		clk_put(sys_clkout2);
		clk_put(clkout2_src_ck);
		clk_put(core_ck);

		sysclock_name = sys_clkout2_name;
		use_mcbsp1_fclk = 1;
	} else {
		sysclock_name = sys_clkout1_name;
	}

	if (GPIO_PIN( audio_gpio.spdif ) != -1)
		archos_gpio_init_output( &audio_gpio.spdif, "spdif" );

	if (GPIO_PIN( audio_gpio.hp_on ) != -1)
		archos_gpio_init_output( &audio_gpio.hp_on, "hp_on" );

	if (GPIO_PIN( audio_gpio.headphone_plugged ) != -1)
		archos_gpio_init_input( &audio_gpio.headphone_plugged, "hp_detect" );

	if (GPIO_PIN( audio_gpio.vamp_vbat ) != -1)
		archos_gpio_init_output( &audio_gpio.vamp_vbat, "vamp_vbat" );
	if (GPIO_PIN( audio_gpio.vamp_dc ) != -1)
		archos_gpio_init_output( &audio_gpio.vamp_dc, "vamp_dc" );

	// XXX maybe prevents OFF mode?
	if (GPIO_PIN( audio_gpio.headphone_plugged ) != -1)
		gpio_set_debounce(GPIO_PIN(audio_gpio.headphone_plugged), (1 + 1) * 0x1f);

	if (GPIO_PIN(audio_gpio.vamp_dc) != -1) {
		int ret = platform_device_register(&archos_audio_vamp_device);
		if (ret < 0)
			return ret;

		ret = platform_driver_register(&archos_audio_vamp_driver);
		if (ret < 0)
			return ret;

		ret = device_create_file(&archos_audio_vamp_device.dev, &dev_attr_vamp_vusb_ctrl);
		if (ret < 0)
			return ret;

		hrtimer_init(&vamp_watchdog_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		vamp_watchdog_timer.function = vamp_watchdog_timer_func;
	}

	pr_debug("%s init done\n", __FUNCTION__);
	return 0;
}

EXPORT_SYMBOL(archos_audio_get_io);

