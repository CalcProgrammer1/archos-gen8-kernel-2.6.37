/*
 * LG 4573 DSI panel
 *
 * Modified by Adam Honse (CalcProgrammer1), Feb 1 2012
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

#define GAMMA_ADJUST	1

// USER COMMAND SET

#define LG_UCS_NOP		0x00
#define LG_UCS_SWRESET		0x01
#define LG_UCS_RDDPM		0x0a
#define LG_UCS_RDDMADCTL	0x0b
#define LG_UCS_RDDCOLMOD	0x0c
#define LG_UCS_RDDIN		0x0d
#define LG_UCS_SLPIN		0x10
#define LG_UCS_SLPOUT		0x11
#define LG_UCS_INVOFF		0x20
#define LG_UCS_INVON		0x21
#define LG_UCS_DISPOFF		0x28
#define LG_UCS_DISPON		0x29
#define LG_UCS_MADCTL		0x36
#define LG_UCS_IDMOFF		0x38
#define LG_UCS_IDMON		0x39
#define LG_UCS_COLMOD		0x3a
#define LG_UCS_WRDISBV		0x51
#define LG_UCS_RDDISVB		0x52
#define LG_UCS_WRCTRLD		0x53
#define LG_UCS_RDCTRLD		0x54
#define LG_UCS_WRCABC		0x55
#define LG_UCS_RDCABC		0x56
#define LG_UCS_WRCABCMB		0x5e
#define LG_UCS_RDCABCMB		0x5f

// MANUFACTURER COMMAND SET

#define LG_MCS_RGBIF		0xb1
#define LG_MCS_PANELSET		0xb2
#define LG_MCS_PANELDRV		0xb3
#define LG_MCS_DISPMODE		0xb4
#define LG_MCS_DISPCTL1		0xb5
#define LG_MCS_DISPCTL2		0xb6
#define LG_MCS_OSCSET		0xc0
#define LG_MCS_PWRCTL1		0xc1
#define LG_MCS_PWRCTL2		0xc2
#define LG_MCS_PWRCTL3		0xc3
#define LG_MCS_PWRCTL4		0xc4
#define LG_MCS_PWRCTL5		0xc5
#define LG_MCS_PWRCTL6		0xc6
#define LG_MCS_OFCCTL		0xc7
#define LG_MCS_BLCTL		0xc8
#define LG_MCS_RGAMMAP		0xd0
#define LG_MCS_RGAMMAN		0xd1
#define LG_MCS_GGAMMAP		0xd2
#define LG_MCS_GGAMMAN		0xd3
#define LG_MCS_BGAMMAP		0xd4
#define LG_MCS_BGAMMAN		0xd5
#define LG_MCS_TEST1		0xf0
#define LG_MCS_OTP1		0xf8
#define LG_MCS_OTP2		0xf9
#define LG_MCS_OTP3		0xfa

#define DW(x...) dsi_vc_dcs_write_nosync(TCH, (u8[]){x}, sizeof((u8[]){x}))

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

struct panel_data {
	struct mutex lock;

	unsigned long	hw_guard_end;	/* next value of jiffies when we can
					 * issue the next sleep in/out command
					 */
	unsigned long	hw_guard_wait;	/* max guard time in jiffies */

	struct omap_dss_device *dssdev;

	bool 		enabled;
	u8 		rotate;
	bool		mirror;

	bool		cabc_broken;
	unsigned	cabc_mode;
	struct panel_config *panel_config;
};

static void panel_get_timings(struct omap_dss_device *dssdev, struct omap_video_timings *timings)
{
	*timings = dssdev->panel.timings;
}

static void panel_get_resolution(struct omap_dss_device *dssdev, u16 *xres, u16 *yres)
{
	*xres = dssdev->panel.timings.x_res;
	*yres = dssdev->panel.timings.y_res;
}
// TODO: this display has CABC too.

static const char *cabc_modes[] = {
	"off",		/* used also always when CABC is not supported */
	"ui",
	"still-image",
	"moving-image",
};

static ssize_t show_cabc_mode(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct omap_dss_device *dssdev = to_dss_device(dev);
	struct panel_data *td = dev_get_drvdata(&dssdev->dev);
	const char *mode_str;
	int mode;
	int len;
	pr_debug("show_cabc_mode\n");
	mode = td->cabc_mode;

	mode_str = "unknown";
	if (mode >= 0 && mode < ARRAY_SIZE(cabc_modes)) mode_str = cabc_modes[mode];
	len = snprintf(buf, PAGE_SIZE, "%s\n", mode_str);

	return len < PAGE_SIZE - 1 ? len : PAGE_SIZE - 1;
}

static ssize_t store_cabc_mode(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct omap_dss_device *dssdev = to_dss_device(dev);
	struct panel_data *td = dev_get_drvdata(&dssdev->dev);
	int i;
	pr_debug("store_cabc_mode\n");
	for (i = 0; i < ARRAY_SIZE(cabc_modes); i++)
	{
		if (sysfs_streq(cabc_modes[i], buf)) break;
	}

	if (i == ARRAY_SIZE(cabc_modes)) return -EINVAL;

	mutex_lock(&td->lock);

	if (td->enabled)
	{
		dsi_bus_lock();
		if (!td->cabc_broken) DW(LG_UCS_WRCABC, i);
		dsi_bus_unlock();
	}

	td->cabc_mode = i;

	mutex_unlock(&td->lock);

	return count;
}

static ssize_t show_cabc_available_modes(struct device *dev, struct device_attribute *attr, char *buf)
{
	int len;
	int i;
	pr_debug("show_cabc_available_modes\n");
	for (i = 0, len = 0; len < PAGE_SIZE && i < ARRAY_SIZE(cabc_modes); i++)
	{
		len += snprintf(&buf[len], PAGE_SIZE - len, "%s%s%s", i ? " " : "", cabc_modes[i], i == ARRAY_SIZE(cabc_modes) - 1 ? "\n" : "");
	}

	return len < PAGE_SIZE ? len : PAGE_SIZE - 1;
}

static DEVICE_ATTR(cabc_mode, S_IRUGO | S_IWUSR, show_cabc_mode, store_cabc_mode);
static DEVICE_ATTR(cabc_available_modes, S_IRUGO, show_cabc_available_modes, NULL);

static struct attribute *panel_attrs[] = {
	&dev_attr_cabc_mode.attr,
	&dev_attr_cabc_available_modes.attr,
	NULL,
};

static struct attribute_group panel_attr_group = {
	.attrs = panel_attrs,
};

static void panel_sw_reset(struct omap_dss_device *dssdev)
{
	u8 cmd;
	pr_debug("panel_sw_reset\n");
	cmd = LG_UCS_SWRESET;
	dsi_vc_dcs_write_nosync(TCH, &cmd, 1);
	msleep(10);	// min 5ms; in sleep-out min 120ms to next SLPOUT
}

static int panel_probe(struct omap_dss_device *dssdev)
{
	struct panel_data *td;
	int r, i;
	pr_debug("panel_probe\n");
	dev_dbg(&dssdev->dev, "probe\n");

	td = kzalloc(sizeof(*td), GFP_KERNEL);
	if (!td)
	{
		return -ENOMEM;
	}
	td->dssdev = dssdev;
	td->panel_config = &lg_config;

	mutex_init(&td->lock);

	dev_set_drvdata(&dssdev->dev, td);

	dssdev->caps = OMAP_DSS_DISPLAY_CAP_VIDEO_MODE;

	return 0;
	kfree(td);
}

static void panel_remove(struct omap_dss_device *dssdev)
{
	struct panel_data *td = dev_get_drvdata(&dssdev->dev);

	dev_dbg(&dssdev->dev, "remove\n");

	/* reset, to be sure that the panel is in a valid state */
	//panel_hw_reset(dssdev);

	kfree(td);
}



#ifdef GAMMA
static void lg_gamma(void)
{
// Shared values.
# define LG_PKP 0x00, 0x44, 0x74
# define LG_PRP 0x57
# define LG_VRP 0x15, 0x03
# define LG_PFP 0x61, 0x46
# define LG_PMP 0x03
	DW(LG_MCS_RGAMMAP, LG_PKP, LG_PRP, LG_VRP, LG_PFP, LG_PMP);
	DW(LG_MCS_RGAMMAN, LG_PKP, LG_PRP, LG_VRP, LG_PFP, LG_PMP);
	DW(LG_MCS_GGAMMAP, LG_PKP, LG_PRP, LG_VRP, LG_PFP, LG_PMP);
	DW(LG_MCS_GGAMMAN, LG_PKP, LG_PRP, LG_VRP, LG_PFP, LG_PMP);
	DW(LG_MCS_BGAMMAP, LG_PKP, LG_PRP, LG_VRP, LG_PFP, LG_PMP);
	DW(LG_MCS_BGAMMAN, LG_PKP, LG_PRP, LG_VRP, LG_PFP, LG_PMP);
}
#else	// GAMMA
# ifdef GAMMA_ADJUST
static void lg_adjust_gamma(void)
{
// Shared values.
#  define LG_PKPP 0x00, 0x44, 0x74
#  define LG_PKPN 0x00, 0x44, 0x70
#  define LG_PRP 0x47
#  define LG_VRPP 0x22, 0x12
#  define LG_VRPN 0x22, 0x02
#  define LG_PFP 0x61, 0x46
#  define LG_PMPP 0x05
#  define LG_PMPN 0x03
	DW(LG_MCS_RGAMMAP, LG_PKPP, LG_PRP, LG_VRPP, 0x61, 0x36, LG_PMPP);
	DW(LG_MCS_RGAMMAN, LG_PKPN, LG_PRP, LG_VRPN, 0x61, 0x36, LG_PMPN);
	DW(LG_MCS_GGAMMAP, LG_PKPP, LG_PRP, LG_VRPP, LG_PFP, LG_PMPP);
	DW(LG_MCS_GGAMMAN, LG_PKPN, LG_PRP, LG_VRPN, LG_PFP, LG_PMPN);
	DW(LG_MCS_BGAMMAP, LG_PKPP, LG_PRP, LG_VRPP, LG_PFP, LG_PMPP);
	DW(LG_MCS_BGAMMAN, LG_PKPN, LG_PRP, LG_VRPN, LG_PFP, LG_PMPN);
}
# else	// GAMMA_ADJUST
static void lg_noadjust_gamma(void)
{
// Shared values.
#  define LG_PKP 0x00, 0x44, 0x44
#  define LG_PRP 0x07
#  define LG_VRPP 0x00, 0x12
#  define LG_VRPN 0x00, 0x02
#  define LG_PFP 0x61, 0x16
#  define LG_PMPP 0x05
#  define LG_PMPN 0x03
	DW(LG_MCS_RGAMMAP, LG_PKP, LG_PRP, LG_VRPP, 0x61, 0x12, LG_PMPP);
	DW(LG_MCS_RGAMMAN, LG_PKP, LG_PRP, LG_VRPN, LG_PFP, LG_PMPN);
	DW(LG_MCS_GGAMMAP, LG_PKP, LG_PRP, LG_VRPP, LG_PFP, LG_PMPP);
	DW(LG_MCS_GGAMMAN, LG_PKP, LG_PRP, LG_VRPN, LG_PFP, LG_PMPN);
	DW(LG_MCS_BGAMMAP, LG_PKP, LG_PRP, LG_VRPP, LG_PFP, LG_PMPP);
	DW(LG_MCS_BGAMMAN, LG_PKP, LG_PRP, LG_VRPN, LG_PFP, LG_PMPN);
}
# endif	// !GAMMA_ADJUST
#endif	// !GAMMA

#undef LG_PFP
#undef LG_PKP
#undef LG_PKPN
#undef LG_PKPP
#undef LG_PMP
#undef LG_PMPN
#undef LG_PMPP
#undef LG_PRP
#undef LG_VRP
#undef LG_VRPN
#undef LG_VRPP

static void gamma_settings(void)
{
#ifdef GAMMA
	lg_gamma();
#else	// GAMMA
# ifdef GAMMA_ADJUST
	lg_adjust_gamma();
# else	// GAMMA_ADJUST
	lg_noadjust_gamma();
# endif	// !GAMMA_ADJUST
#endif	// !GAMMA
}

static void panel_initial_settings(void)
{
	pr_debug("panel_initial_settings\n");


	DW(LG_UCS_COLMOD, 0x70);	/* 24 bpp */
	DW(LG_UCS_MADCTL, 0x00); 	/* RGB, no H/V flip */
	DW(LG_MCS_PANELSET, 0x20, 0xd6);
	DW(LG_MCS_PANELDRV, 0x02);	/* 2-dot inversion mode. */
	DW(LG_MCS_DISPMODE, 0x04);	/* display mode control "dithering off" */

	DW(LG_MCS_DISPCTL1, 0x10, 0x0f, 0x0f, 0x00, 0x01);
	DW(LG_MCS_DISPCTL2, 0x03, 0x15, 0x02, 0x0f, 0x0f, 0x1f );
	
	/* power control */
	DW(LG_MCS_OSCSET, 0x01, 0x18 );		/* enable internal osc. */
	DW(LG_MCS_PWRCTL1, 0x00);
	DW(LG_MCS_PWRCTL3, 0x07, 0x04, 0x04, 0x04, 0x07 );
	DW(LG_MCS_PWRCTL4, 0x12, 0x33, 0x1a, 0x1a, 0x07, 0x49 );
	DW(LG_MCS_PWRCTL5, 0x6d );
	DW(LG_MCS_PWRCTL6, 0x44, 0x63, 0x00 );

	// Prevent DSI TX FIFO overflow.
	msleep(1);

	gamma_settings();
}

static int panel_sleep(int sleep)
{
	u8 cmd;

	printk("panel_sleep(%i)\n", sleep);
	
	if (sleep) {
		cmd = LG_UCS_SLPIN;
		dsi_vc_dcs_write_nosync(TCH, &cmd, 1);
		msleep(500);

		cmd = LG_UCS_DISPOFF;
		dsi_vc_dcs_write_nosync(TCH, &cmd, 1);
		msleep(500);
	} else {
		cmd = LG_UCS_SLPOUT;
		dsi_vc_dcs_write_nosync(TCH, &cmd, 1);
		msleep(500);

		cmd = LG_UCS_DISPON;
		dsi_vc_dcs_write_nosync(TCH, &cmd, 1);
		msleep(500);
	}

	return 0;
}

static int panel_power_on(struct omap_dss_device *dssdev)
{
	struct panel_data *td = dev_get_drvdata(&dssdev->dev);
	int r;
	pr_debug("panel_power_on\n");
	r = omapdss_dsi_display_enable(dssdev);
	if (r) {
		dev_err(&dssdev->dev, "failed to enable DSI\n");
		goto err0;
	}

	if (dssdev->platform_enable)
		r = dssdev->platform_enable(dssdev);
	if (r < 0)
		goto err;

	panel_sw_reset(dssdev);

	omapdss_dsi_vc_enable_hs(TCH, false);

	panel_initial_settings();
	panel_sleep(0);

	td->enabled = 1;

	// Enable Virtual Channel 1.
	omapdss_dsi_vc_enable_hs(1, 1);
	dispc_enable_channel(OMAP_DSS_CHANNEL_LCD, 1);

	return 0;
err:
	omapdss_dsi_display_disable(dssdev);
err0:
	return r;
}

static void panel_power_off(struct omap_dss_device *dssdev)
{
	struct panel_data *td = dev_get_drvdata(&dssdev->dev);

	dispc_enable_channel(OMAP_DSS_CHANNEL_LCD, 0);
	// Virtual Channels will be disabled in omapdss_dsi_display_disable().

	if (dssdev->platform_disable)
	{
		dssdev->platform_disable(dssdev);
	}
	else
	{
		// TODO: else sleep-in?
		panel_sleep(1);
	}

	omapdss_dsi_display_disable(dssdev);

	td->enabled = 0;
}

static int panel_enable(struct omap_dss_device *dssdev)
{
	struct panel_data *td = dev_get_drvdata(&dssdev->dev);
	int r;
	pr_debug("panel_enable\n");
	dev_dbg(&dssdev->dev, "enable\n");

	mutex_lock(&td->lock);

	if (dssdev->state != OMAP_DSS_DISPLAY_DISABLED)
	{
		r = -EINVAL;
		goto err;
	}

	dsi_bus_lock();

	r = panel_power_on(dssdev);

	dsi_bus_unlock();

	if (r)
	{
		goto err;
	}

	dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;

	mutex_unlock(&td->lock);

	return 0;
	
err:
	dev_dbg(&dssdev->dev, "enable failed\n");
	mutex_unlock(&td->lock);
	return r;
}

static void panel_disable(struct omap_dss_device *dssdev)
{
	struct panel_data *td = dev_get_drvdata(&dssdev->dev);
	pr_debug("panel_disable\n");
	dev_dbg(&dssdev->dev, "disable\n");

	mutex_lock(&td->lock);
	dsi_bus_lock();

	if (dssdev->state == OMAP_DSS_DISPLAY_ACTIVE)
		panel_power_off(dssdev);

	dsi_bus_unlock();

	dssdev->state = OMAP_DSS_DISPLAY_DISABLED;

	mutex_unlock(&td->lock);
}

static int panel_suspend(struct omap_dss_device *dssdev)
{
	struct panel_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	dev_dbg(&dssdev->dev, "suspend\n");

	mutex_lock(&td->lock);

	if (dssdev->state != OMAP_DSS_DISPLAY_ACTIVE)
	{
		mutex_unlock(&td->lock);
		return -EINVAL;
	}

	dsi_bus_lock();
	// TODO: fix suspend/resume, seems like sleep-in and DISPOFF don't work.
	if (dssdev->state == OMAP_DSS_DISPLAY_ACTIVE)
	{
		panel_power_off(dssdev);
	}
	r = 0;
	dsi_bus_unlock();

	dssdev->state = OMAP_DSS_DISPLAY_SUSPENDED;

	mutex_unlock(&td->lock);
	return r;
}

static int panel_resume(struct omap_dss_device *dssdev)
{
	struct panel_data *td = dev_get_drvdata(&dssdev->dev);
	int r;

	dev_dbg(&dssdev->dev, "resume\n");

	mutex_lock(&td->lock);

	if (dssdev->state != OMAP_DSS_DISPLAY_SUSPENDED)
	{
		mutex_unlock(&td->lock);
		return -EINVAL;
	}

	dsi_bus_lock();
	r = panel_power_on(dssdev);
	dsi_bus_unlock();

	if (r)
	{
		dssdev->state = OMAP_DSS_DISPLAY_SUSPENDED;
	}
	else
	{
		dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;
	}

	mutex_unlock(&td->lock);
	return r;
}

static int panel_update(struct omap_dss_device *dssdev, u16 x, u16 y, u16 w, u16 h)
{
	dev_dbg(&dssdev->dev, "update is ignored for video mode displays\n");
	return 0;
}

static int panel_sync(struct omap_dss_device *dssdev)
{
	/* We are in video mode so wait for the vsync */
	return dssdev->manager->wait_for_vsync(dssdev->manager);
}

static int panel_set_update_mode(struct omap_dss_device *dssdev, enum omap_dss_update_mode mode)
{
	if (mode != OMAP_DSS_UPDATE_AUTO)
	{
		return -EINVAL;
	}
	return 0;
}

static enum omap_dss_update_mode panel_get_update_mode(struct omap_dss_device *dssdev)
{
	return OMAP_DSS_UPDATE_AUTO;
}

static struct omap_dss_driver panel_driver = {
	.probe		= panel_probe,
	.remove		= panel_remove,

	.enable		= panel_enable,
	.disable	= panel_disable,
	.suspend	= panel_suspend,
	.resume		= panel_resume,

	.set_update_mode = panel_set_update_mode,
	.get_update_mode = panel_get_update_mode,

	.update		= panel_update,
	.sync		= panel_sync,

	.get_resolution	= panel_get_resolution,
	.get_recommended_bpp = omapdss_default_get_recommended_bpp,

	.get_timings	= panel_get_timings,

	.driver         = {
		.name   = "lg_fwvga_43",
		.owner  = THIS_MODULE,
	},
};

static int __init panel_init(void)
{
	omap_dss_register_driver(&panel_driver);

	return 0;
}

static void __exit panel_exit(void)
{
	omap_dss_unregister_driver(&panel_driver);
}

module_init(panel_init);
module_exit(panel_exit);

MODULE_DESCRIPTION("LG 4573 Driver");
MODULE_LICENSE("GPL");
#undef DW
