/*
 * Taal DSI command mode panel
 *
 * Copyright (C) 2009 Nokia Corporation
 * Author: Tomi Valkeinen <tomi.valkeinen@nokia.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define DEBUG

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/mutex.h>

#include <plat/display.h>

/* DSI Virtual channel. Hardcoded for now. */
#define TCH 0

#define DCS_READ_NUM_ERRORS	0x05
#define DCS_READ_POWER_MODE	0x0a
#define DCS_READ_MADCTL		0x0b
#define DCS_READ_PIXEL_FORMAT	0x0c
#define DCS_RDDSDR		0x0f
#define DCS_SLEEP_IN		0x10
#define DCS_SLEEP_OUT		0x11
#define DCS_DISPLAY_OFF		0x28
#define DCS_DISPLAY_ON		0x29
#define DCS_COLUMN_ADDR		0x2a
#define DCS_PAGE_ADDR		0x2b
#define DCS_MEMORY_WRITE	0x2c
#define DCS_TEAR_OFF		0x34
#define DCS_TEAR_ON		0x35
#define DCS_MEM_ACC_CTRL	0x36
#define DCS_PIXEL_FORMAT	0x3a
#define DCS_BRIGHTNESS		0x51
#define DCS_CTRL_DISPLAY	0x53
#define DCS_WRITE_CABC		0x55
#define DCS_READ_CABC		0x56
#define DCS_GET_ID1		0xda
#define DCS_GET_ID2		0xdb
#define DCS_GET_ID3		0xdc

#define TAAL_ESD_CHECK_PERIOD	msecs_to_jiffies(5000)

#define pw(x...) dsi_vc_dcs_write_nosync(0, (u8[]){x}, sizeof((u8[]){x}))

#define GAMMA_ADJUST	1
#if GAMMA_ADJUST
struct gamma_val {
	unsigned char add;
	unsigned char par[9];
};

struct gamma_par {

	struct gamma_val red_p;
	struct gamma_val red_n;
	struct gamma_val green_p;
	struct gamma_val green_n;
	struct gamma_val blue_p;
	struct gamma_val blue_n;
};

#define gpw(x)	pw( x.add, x.par[0], x.par[1], x.par[2], x.par[3], \
		x.par[4], x.par[5], x.par[6], \
		x.par[7], x.par[8])

static struct gamma_par lg_gamma = {
	{ 0xd0,
	{ 0x00, 0x44, 0x74, 0x47, 0x22, 0x12, 0x61, 0x36, 0x05 }},
	{ 0xd1,
	{ 0x00, 0x44, 0x70, 0x47, 0x22, 0x02, 0x61, 0x36, 0x03 }},
	{ 0xd2,
	{ 0x00, 0x44, 0x74, 0x47, 0x22, 0x12, 0x61, 0x46, 0x05 }},
	{ 0xd3,
	{ 0x00, 0x44, 0x70, 0x47, 0x22, 0x02, 0x61, 0x46, 0x03 }},
	{ 0xd4,
	{ 0x00, 0x44, 0x74, 0x47, 0x22, 0x12, 0x61, 0x46, 0x05 }},
	{ 0xd5,
	{ 0x00, 0x44, 0x70, 0x47, 0x22, 0x02, 0x61, 0x46, 0x03 }},
};
#endif


// HACK: it's needed to enable video mode.
extern void dispc_enable_channel(enum omap_channel channel, bool enable);

/**
 * struct panel_config - panel configuration
 * @name: panel name
 * @type: panel type
 * @timings: panel resolution
 * @sleep: various panel specific delays, passed to msleep() if non-zero
 * @reset_sequence: reset sequence timings, passed to udelay() if non-zero
 * @regulators: array of panel regulators
 * @num_regulators: number of regulators in the array
 */
struct panel_config {
	struct {
		unsigned int sleep_in;
		unsigned int sleep_out;
		unsigned int hw_reset;
		unsigned int enable_te;
	} sleep;

	struct {
		unsigned int high;
		unsigned int low;
	} reset_sequence;
};

static struct panel_config lg_config = {
	.sleep		= {
		.sleep_in	= 5,
		.sleep_out	= 5,
		.hw_reset	= 5,
		.enable_te	= 100, /* possible panel bug */
	},
	.reset_sequence	= {
		.high		= 10,
		.low		= 10,
	},
};

struct taal_data {
	struct mutex lock;

	unsigned long	hw_guard_end;	/* next value of jiffies when we can
					 * issue the next sleep in/out command
					 */
	unsigned long	hw_guard_wait;	/* max guard time in jiffies */

	struct omap_dss_device *dssdev;

	bool enabled;
	bool mirror;

	//bool cabc_broken;
	//unsigned cabc_mode;

	struct panel_config *panel_config;
};


// TODO: rework to taal-like code organization.
#if 0
static void hw_guard_start(struct taal_data *td, int guard_msec)
{
	td->hw_guard_wait = msecs_to_jiffies(guard_msec);
	td->hw_guard_end = jiffies + td->hw_guard_wait;
}

static void hw_guard_wait(struct taal_data *td)
{
	unsigned long wait = td->hw_guard_end - jiffies;

	if ((long)wait > 0 && wait <= td->hw_guard_wait) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(wait);
	}
}

static int taal_dcs_read_1(u8 dcs_cmd, u8 *data)
{
	int r;
	u8 buf[1];

	r = dsi_vc_dcs_read(TCH, dcs_cmd, buf, 1);

	if (r < 0)
		return r;

	*data = buf[0];

	return 0;
}

static int taal_dcs_write_0(u8 dcs_cmd)
{
	return dsi_vc_dcs_write(TCH, &dcs_cmd, 1);
}

static int taal_dcs_write_1(u8 dcs_cmd, u8 param)
{
	u8 buf[2];
	buf[0] = dcs_cmd;
	buf[1] = param;
	return dsi_vc_dcs_write(TCH, buf, 2);
}

static int taal_sleep_in(struct taal_data *td)

{
	u8 cmd;
	int r;

	hw_guard_wait(td);

	cmd = DCS_SLEEP_IN;
	r = dsi_vc_dcs_write_nosync(TCH, &cmd, 1);
	if (r)
		return r;

	hw_guard_start(td, 120);

	if (td->panel_config->sleep.sleep_in)
		msleep(td->panel_config->sleep.sleep_in);

	return 0;
}

static int taal_sleep_out(struct taal_data *td)
{
	int r;

	hw_guard_wait(td);

	r = taal_dcs_write_0(DCS_SLEEP_OUT);
	if (r)
		return r;

	hw_guard_start(td, 120);

	if (td->panel_config->sleep.sleep_out)
		msleep(td->panel_config->sleep.sleep_out);

	return 0;
}

static int taal_get_id(u8 *id1, u8 *id2, u8 *id3)
{
	int r;

	r = taal_dcs_read_1(DCS_GET_ID1, id1);
	if (r)
		return r;
	r = taal_dcs_read_1(DCS_GET_ID2, id2);
	if (r)
		return r;
	r = taal_dcs_read_1(DCS_GET_ID3, id3);
	if (r)
		return r;

	return 0;
}

static int taal_set_addr_mode(u8 rotate, bool mirror)
{
	return 0;
}
#endif

static void taal_get_timings(struct omap_dss_device *dssdev,
			struct omap_video_timings *timings)
{
	*timings = dssdev->panel.timings;
}

static void taal_get_resolution(struct omap_dss_device *dssdev,
		u16 *xres, u16 *yres)
{
	*xres = dssdev->panel.timings.x_res;
	*yres = dssdev->panel.timings.y_res;
}

// TODO: this display has CABC too.
#if 0
static const char *cabc_modes[] = {
	"off",		/* used also always when CABC is not supported */
	"ui",
	"still-image",
	"moving-image",
};

static ssize_t show_cabc_mode(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct omap_dss_device *dssdev = to_dss_device(dev);
	struct taal_data *td = dev_get_drvdata(&dssdev->dev);
	const char *mode_str;
	int mode;
	int len;

	mode = td->cabc_mode;

	mode_str = "unknown";
	if (mode >= 0 && mode < ARRAY_SIZE(cabc_modes))
		mode_str = cabc_modes[mode];
	len = snprintf(buf, PAGE_SIZE, "%s\n", mode_str);

	return len < PAGE_SIZE - 1 ? len : PAGE_SIZE - 1;
}

static ssize_t store_cabc_mode(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct omap_dss_device *dssdev = to_dss_device(dev);
	struct taal_data *td = dev_get_drvdata(&dssdev->dev);
	int i;

	for (i = 0; i < ARRAY_SIZE(cabc_modes); i++) {
		if (sysfs_streq(cabc_modes[i], buf))
			break;
	}

	if (i == ARRAY_SIZE(cabc_modes))
		return -EINVAL;

	mutex_lock(&td->lock);

	if (td->enabled) {
		dsi_bus_lock();
		if (!td->cabc_broken)
			taal_dcs_write_1(DCS_WRITE_CABC, i);
		dsi_bus_unlock();
	}

	td->cabc_mode = i;

	mutex_unlock(&td->lock);

	return count;
}

static ssize_t show_cabc_available_modes(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	int len;
	int i;

	for (i = 0, len = 0;
	     len < PAGE_SIZE && i < ARRAY_SIZE(cabc_modes); i++)
		len += snprintf(&buf[len], PAGE_SIZE - len, "%s%s%s",
			i ? " " : "", cabc_modes[i],
			i == ARRAY_SIZE(cabc_modes) - 1 ? "\n" : "");

	return len < PAGE_SIZE ? len : PAGE_SIZE - 1;
}

static DEVICE_ATTR(cabc_mode, S_IRUGO | S_IWUSR,
		show_cabc_mode, store_cabc_mode);
static DEVICE_ATTR(cabc_available_modes, S_IRUGO,
		show_cabc_available_modes, NULL);

static struct attribute *taal_attrs[] = {
	&dev_attr_cabc_mode.attr,
	&dev_attr_cabc_available_modes.attr,
	NULL,
};

static struct attribute_group taal_attr_group = {
	.attrs = taal_attrs,
};

static void taal_hw_reset(struct omap_dss_device *dssdev)
{
	struct taal_data *td = dev_get_drvdata(&dssdev->dev);
	struct nokia_dsi_panel_data *panel_data = get_panel_data(dssdev);

	if (panel_data->reset_gpio == -1)
		return;

	gpio_set_value(panel_data->reset_gpio, 1);
	if (td->panel_config->reset_sequence.high)
		udelay(td->panel_config->reset_sequence.high);
	/* reset the panel */
	gpio_set_value(panel_data->reset_gpio, 0);
	/* assert reset */
	if (td->panel_config->reset_sequence.low)
		udelay(td->panel_config->reset_sequence.low);
	gpio_set_value(panel_data->reset_gpio, 1);
	/* wait after releasing reset */
	if (td->panel_config->sleep.hw_reset)
		msleep(td->panel_config->sleep.hw_reset);
}
#endif

static int taal_probe(struct omap_dss_device *dssdev)
{
	struct taal_data *td;
	int r, i;

	dev_dbg(&dssdev->dev, "probe\n");

	td = kzalloc(sizeof(*td), GFP_KERNEL);
	if (!td) {
		r = -ENOMEM;
		goto err;
	}
	td->dssdev = dssdev;
	td->panel_config = &lg_config;

	mutex_init(&td->lock);

	dev_set_drvdata(&dssdev->dev, td);

	dssdev->caps = OMAP_DSS_DISPLAY_CAP_VIDEO_MODE;

	//taal_hw_reset(dssdev);

	/*
	r = sysfs_create_group(&dssdev->dev.kobj, &taal_attr_group);
	if (r) {
		dev_err(&dssdev->dev, "failed to create sysfs files\n");
		goto err_sysfs;
	}
	*/

	return 0;
//err_sysfs:
	kfree(td);
err:
	return r;
}

static void taal_remove(struct omap_dss_device *dssdev)
{
	struct taal_data *td = dev_get_drvdata(&dssdev->dev);

	dev_dbg(&dssdev->dev, "remove\n");

	//sysfs_remove_group(&dssdev->dev.kobj, &taal_attr_group);

	/* reset, to be sure that the panel is in a valid state */
	//taal_hw_reset(dssdev);

	kfree(td);
}

static void gamma_settings(void)
{
#ifdef GAMMA
	pw( 0xd0, 0x00, 0x44, 0x74, 0x57, 0x15, 0x03, 0x61, 0x46, 0x03 );
	pw( 0xd1, 0x00, 0x44, 0x74, 0x57, 0x15, 0x03, 0x61, 0x46, 0x03 );
	pw( 0xd2, 0x00, 0x44, 0x74, 0x57, 0x15, 0x03, 0x61, 0x46, 0x03 );
	pw( 0xd3, 0x00, 0x44, 0x74, 0x57, 0x15, 0x03, 0x61, 0x46, 0x03 );
	pw( 0xd4, 0x00, 0x44, 0x74, 0x57, 0x15, 0x03, 0x61, 0x46, 0x03 );
	pw( 0xd5, 0x00, 0x44, 0x74, 0x57, 0x15, 0x03, 0x61, 0x46, 0x03 );
#else
#ifdef GAMMA_ADJUST
	gpw( lg_gamma.red_p );
	gpw( lg_gamma.red_n );
	gpw( lg_gamma.green_p );
	gpw( lg_gamma.green_n );
	gpw( lg_gamma.blue_p );
	gpw( lg_gamma.blue_n );
#else
	pw( 0xd0, 0x00, 0x44, 0x44, 0x07, 0x00, 0x12, 0x61, 0x12, 0x05 );
	pw( 0xd1, 0x00, 0x44, 0x44, 0x07, 0x00, 0x02, 0x61, 0x16, 0x03 );
	pw( 0xd2, 0x00, 0x44, 0x44, 0x07, 0x00, 0x12, 0x61, 0x16, 0x05 );
	pw( 0xd3, 0x00, 0x44, 0x44, 0x07, 0x00, 0x02, 0x61, 0x16, 0x03 );
	pw( 0xd4, 0x00, 0x44, 0x44, 0x07, 0x00, 0x12, 0x61, 0x16, 0x05 );
	pw( 0xd5, 0x00, 0x44, 0x44, 0x07, 0x00, 0x02, 0x61, 0x16, 0x03 );
#endif
#endif
}

static void panel_initial_settings(void)
{
	pr_debug("panel_initial_settings\n");

	pw( 0x3a, 0x77 );
	pw( 0x36, 0x00 ); 	/* RGB, no H/V flip */
	
	pw( 0xb2, 0x20, 0xd6 );
	pw( 0xb3, 0x02 );
	
	pw( 0xb4, 0x04 );	/* display mode control "dithering off" */
	pw( 0xb5, 0x10, 0x0f, 0x0f, 0x00, 0x01 );

	/* DISPCTL2: DISPLAY OK!! 
	 * param 1: ASG=1, SDM=1, FHN=1, GSWAP=0, FVST=0 
	 * param 2: CLW=0x15
	 * param 3: GTO=0x02
	 * param 4: GNO=0x0f
	 * param 5: FTI=0x0f
	 * param 6: GPM=0x1f
	 */
	pw( 0xb6, 0x03, 0x15, 0x02, 0x0f, 0x0f, 0x1f );
	
	/* power control */
	pw( 0xc0, 0x01, 0x18 );		/* enable internal osc. */
	pw( 0xc1, 0x00, 0x01 );
	pw( 0xc3, 0x07, 0x04, 0x04, 0x04, 0x07 );
	pw( 0xc4, 0x12, 0x33, 0x1a, 0x1a, 0x07, 0x49 );
	pw( 0xc5, 0x6d );
	pw( 0xc6, 0x44, 0x63, 0x00 );
	
	gamma_settings();
	
}

static void panel_sleep(int enable)
{
	printk("panel_sleep(%i)\n", enable);
	
	if (enable) {
		pw( 0x10 );
		msleep(500);
		pw( 0x28 );
		msleep(500);
	} else {
		pw( 0x11 );
		msleep(500);
		pw( 0x29 );
		msleep(500);
	}
}

static int taal_power_on(struct omap_dss_device *dssdev)
{
	struct taal_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	r = omapdss_dsi_display_enable(dssdev);
	if (r) {
		dev_err(&dssdev->dev, "failed to enable DSI\n");
		goto err0;
	}

	// TODO: move platform_enable/disable code to this driver.
	if (dssdev->platform_enable)
		r = dssdev->platform_enable(dssdev);
	if (r < 0)
		goto err;

	//taal_hw_reset(dssdev);

	omapdss_dsi_vc_enable_hs(TCH, false);

	panel_initial_settings();
	panel_sleep(0);

	td->enabled = 1;

	// Enable Virtual Channel 1.
	omapdss_dsi_vc_enable_hs(1, 1);
	dispc_enable_channel(OMAP_DSS_CHANNEL_LCD, 1);

	return 0;
err:
	//dev_err(&dssdev->dev, "error while enabling panel, issuing HW reset\n");
	//taal_hw_reset(dssdev);

	omapdss_dsi_display_disable(dssdev);
err0:
	return r;
}

static void taal_power_off(struct omap_dss_device *dssdev)
{
	struct taal_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	dispc_enable_channel(OMAP_DSS_CHANNEL_LCD, 0);
	// Virtual Channels will be disabled in omapdss_dsi_display_disable().

	if (dssdev->platform_disable)
		dssdev->platform_disable(dssdev);
	else
		// TODO: else sleep-in?
		panel_sleep(1);

	/*
	if (r) {
		dev_err(&dssdev->dev,
				"error disabling panel, issuing HW reset\n");
		taal_hw_reset(dssdev);
	}
	*/

	omapdss_dsi_display_disable(dssdev);

	td->enabled = 0;
}

static int taal_enable(struct omap_dss_device *dssdev)
{
	struct taal_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	dev_dbg(&dssdev->dev, "enable\n");

	mutex_lock(&td->lock);

	if (dssdev->state != OMAP_DSS_DISPLAY_DISABLED) {
		r = -EINVAL;
		goto err;
	}

	dsi_bus_lock();

	r = taal_power_on(dssdev);

	dsi_bus_unlock();

	if (r)
		goto err;

	dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;

	mutex_unlock(&td->lock);

	return 0;
err:
	dev_dbg(&dssdev->dev, "enable failed\n");
	mutex_unlock(&td->lock);
	return r;
}

static void taal_disable(struct omap_dss_device *dssdev)
{
	struct taal_data *td = dev_get_drvdata(&dssdev->dev);

	dev_dbg(&dssdev->dev, "disable\n");

	mutex_lock(&td->lock);

	dsi_bus_lock();

	if (dssdev->state == OMAP_DSS_DISPLAY_ACTIVE)
		taal_power_off(dssdev);

	dsi_bus_unlock();

	dssdev->state = OMAP_DSS_DISPLAY_DISABLED;

	mutex_unlock(&td->lock);
}

static int taal_suspend(struct omap_dss_device *dssdev)
{
	struct taal_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	dev_dbg(&dssdev->dev, "suspend\n");

	mutex_lock(&td->lock);

	if (dssdev->state != OMAP_DSS_DISPLAY_ACTIVE) {
		r = -EINVAL;
		goto err;
	}

	dsi_bus_lock();

	taal_power_off(dssdev);

	dsi_bus_unlock();

	dssdev->state = OMAP_DSS_DISPLAY_SUSPENDED;

	mutex_unlock(&td->lock);

	return 0;
err:
	mutex_unlock(&td->lock);
	return r;
}

static int taal_resume(struct omap_dss_device *dssdev)
{
	struct taal_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	dev_dbg(&dssdev->dev, "resume\n");

	mutex_lock(&td->lock);

	if (dssdev->state != OMAP_DSS_DISPLAY_SUSPENDED) {
		r = -EINVAL;
		goto err;
	}

	dsi_bus_lock();

	r = taal_power_on(dssdev);

	dsi_bus_unlock();

	if (r) {
		dssdev->state = OMAP_DSS_DISPLAY_DISABLED;
	} else {
		dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;
	}

	mutex_unlock(&td->lock);

	return r;
err:
	mutex_unlock(&td->lock);
	return r;
}

static int taal_update(struct omap_dss_device *dssdev,
				    u16 x, u16 y, u16 w, u16 h)
{
	dev_dbg(&dssdev->dev, "update is ignored for video mode displays\n");
	return 0;
}

static int taal_sync(struct omap_dss_device *dssdev)
{
	/* We are in video mode so wait for the vsync */
	return dssdev->manager->wait_for_vsync(dssdev->manager);
}

#if 0
static int taal_mirror(struct omap_dss_device *dssdev, bool enable)
{
	struct taal_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	dev_dbg(&dssdev->dev, "mirror %d\n", enable);

	mutex_lock(&td->lock);

	if (td->mirror == enable)
		goto end;

	dsi_bus_lock();
	if (td->enabled) {
		r = taal_set_addr_mode(td->rotate, enable);
		if (r)
			goto err;
	}

	td->mirror = enable;

	dsi_bus_unlock();
end:
	mutex_unlock(&td->lock);
	return 0;
err:
	dsi_bus_unlock();
	mutex_unlock(&td->lock);
	return r;
}

static bool taal_get_mirror(struct omap_dss_device *dssdev)
{
	struct taal_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	mutex_lock(&td->lock);
	r = td->mirror;
	mutex_unlock(&td->lock);

	return r;
}
#endif

static int taal_set_update_mode(struct omap_dss_device *dssdev,
		enum omap_dss_update_mode mode)
{
	if (mode != OMAP_DSS_UPDATE_AUTO)
		return -EINVAL;
	return 0;
}

static enum omap_dss_update_mode taal_get_update_mode(
		struct omap_dss_device *dssdev)
{
	return OMAP_DSS_UPDATE_AUTO;
}

static struct omap_dss_driver taal_driver = {
	.probe		= taal_probe,
	.remove		= taal_remove,

	.enable		= taal_enable,
	.disable	= taal_disable,
	.suspend	= taal_suspend,
	.resume		= taal_resume,

	.set_update_mode = taal_set_update_mode,
	.get_update_mode = taal_get_update_mode,

	.update		= taal_update,
	.sync		= taal_sync,

	.get_resolution	= taal_get_resolution,
	.get_recommended_bpp = omapdss_default_get_recommended_bpp,

	//.set_mirror	= taal_mirror,
	//.get_mirror	= taal_get_mirror,

	.get_timings	= taal_get_timings,

	.driver         = {
		.name   = "lg_fwvga_43",
		.owner  = THIS_MODULE,
	},
};

static int __init taal_init(void)
{
	omap_dss_register_driver(&taal_driver);

	return 0;
}

static void __exit taal_exit(void)
{
	omap_dss_unregister_driver(&taal_driver);
}

module_init(taal_init);
module_exit(taal_exit);

MODULE_DESCRIPTION("LG 4573 Driver");
MODULE_LICENSE("GPL");
