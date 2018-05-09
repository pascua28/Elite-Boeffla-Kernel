/*
 *  Spider-MHL mouse driver
 *
 *  Copyright (c) 2012 Ray Moon
 *
 *  Based on the work of:
 *	Vojtech Pavlik
 *	Michael Rausch		James Banks
 *	Matther Dillon		David Giller
 *	Nathan Laredo		Linus Torvalds
 *	Johan Myreen		Jes Sorensen
 *	Russell King
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/spidermhl.h>

MODULE_AUTHOR("Ray Moon <pianist@helixtech.co.kr>");
MODULE_DESCRIPTION("Spider-MHL mouse driver");
MODULE_LICENSE("GPL");

#define SPIDERMHL_VERSION	"0.3"
#define SPIDERMHL_MOUSE_DEVCONFIG		2

typedef struct {
	char name[32];
	char phys[32];
} SPIDERMHL_MOUSE_DEVINFO;

static SPIDERMHL_MOUSE_DEVINFO spidermouse_dev_info[SPIDERMHL_MOUSE_DEVCONFIG];

static struct {
	struct input_dev *dev[SPIDERMHL_MOUSE_DEVCONFIG];
} spidermouse_driver;

static struct spider_event queued_event;
static struct work_struct spidermouse_work;
static struct workqueue_struct *spidermouse_wq;

static int spidermouse_to_index(int dev_type)
{
	int source;
	int shift = 0;

	source = (SM_DEV_TYPE_MOUSE & dev_type)
					? SM_DEV_TYPE_MOUSE : SM_DEV_EVENT_MOUSE;

	do {
		source >>= 1;
		shift++;
	} while (!(source & 1));

	return (dev_type >> shift) - 1;
}

static struct input_dev *spidermouse_get_drvdata(int index)
{
	if (index < 0 || index >= SPIDERMHL_MOUSE_DEVCONFIG)
	{
		pr_err("spidermouse: %s: index error [%d]\n", __func__, index);
		return NULL;
	}

	return spidermouse_driver.dev[index];
}

static int spidermouse_set_drvdata(const struct input_dev *dev, const int index)
{
	dbg_enter();

	if (index < 0 || index >= SPIDERMHL_MOUSE_DEVCONFIG)
	{
		pr_err("spidermouse: %s: index error [%d]\n", __func__, index);
		return 1;
	}

	spidermouse_driver.dev[index] = (struct input_dev *)dev;

	pr_info("spidermouse: %s: %s mouse %sregistered\n", __func__,
			!index ? "PS2" : "USB", !dev ? "un" : "");

	dbg_leave();

	return 0;
}

static int spidermouse_handle_event(const struct spider_event *event)
{
	struct input_dev *dev;
	int nx, ny, nz, dx, dy, dz, new_buttons;
	int dev_type;
	int index;
	char *value = NULL;

	dbg_enter();

	dev_type = SM_DEV_EVENT_MOUSE & event->event_state;
	index = spidermouse_to_index(dev_type);
	pr_debug("spidermouse: %s: index: %d\n", __func__, index);

	dev = spidermouse_get_drvdata(index);

	if (!dev || !dev->users) {
		pr_err("spidermouse: %s: no dev or no user!\n", __func__);
		return -ENODEV;
	}

	value = !index ?
		(char *)&event->pmouse_data[0] : (char *)&event->umouse_data[0];

	new_buttons = (int)value[0];

	nx = value[1] & 0xff;
	ny = value[2] & 0xff;
	nz = value[3] & 0xff;

	dx = nx - 0;
	dy = ny - 0;
	dz = nz - 0;

	if (-127 > dx)	dx = (256 + nx);
	if (127 < dx)	dx = (nx - 256);
	if (-127 > dy)	dy = (256 + ny);
	if (127 < dy)	dy = (ny - 256);
	if (-127 > dz)	dz = (256 + nx);
	if (127 < dz)	dz = (nz - 256);

	pr_debug("spidermouse: %s: %#02x %d %d %d\n", __func__,
							new_buttons, dx, dy, dz);

	input_report_rel(dev, REL_X, dx);
	input_report_rel(dev, REL_Y, dy);
	input_report_rel(dev, REL_WHEEL, dz);

	input_report_key(dev, BTN_LEFT,  new_buttons & 1);
	input_report_key(dev, BTN_RIGHT, new_buttons & 2);
	input_report_key(dev, BTN_BACK,  new_buttons & 4);

	input_sync(dev);

	dbg_leave();

	return 0;
}

static int spidermouse_open(struct input_dev *dev)
{
	dbg_enter();

	dbg_leave();

	return 0;
}

static void spidermouse_close(struct input_dev *dev)
{
	dbg_enter();

	dbg_leave();
}

static int spidermouse_connect(int index)
{
	struct input_dev *dev;
	int err;

	dbg_enter();

	if (index < 0 || index >= SPIDERMHL_MOUSE_DEVCONFIG)
	{
		pr_err("spidermouse: %s: index error [%d]\n", __func__, index);
		return 1;
	}

	if (spidermouse_driver.dev[index]) {
		pr_warn("spidermouse: %s: mouse-%d already registered\n",
							__func__, index);
		return -EBUSY;
	}

	dev = input_allocate_device();
	if (!dev)
		return -ENOMEM;
	else
		pr_info("spidermouse: %s: dev[%d] = %p\n",
							__func__, index, dev);

	memset(&spidermouse_dev_info[index], 0, sizeof(SPIDERMHL_MOUSE_DEVINFO));

	sprintf((char *)spidermouse_dev_info[index].name, "%s%d", "spidermouse-mhl", index);
	sprintf((char *)spidermouse_dev_info[index].phys, "%s%d", "spidermouse/input", index);

	dev->name = (char *)spidermouse_dev_info[index].name;
	dev->phys = (char *)spidermouse_dev_info[index].phys;
#if 0	/*
	 * wake-up by mouse movement 12-05-02 pianist
	 * Check http://source.android.com/tech/input/input-device-configuration-files.html
	 * External devices on the external bus can wake android up.
	 * External bus can be BUS_USB or BUS_BLUETOOTH.
	 */
	dev->id.bustype = BUS_HOST;
#else
	dev->id.bustype = BUS_USB;
#endif
	dev->id.vendor = 0x0001;
	dev->id.product = 0x0002;
	dev->id.version = 0x0100;

	dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
	dev->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y) | BIT_MASK(REL_WHEEL);
	dev->keybit[BIT_WORD(BTN_LEFT)] = BIT_MASK(BTN_LEFT) |
		BIT_MASK(BTN_BACK) | BIT_MASK(BTN_RIGHT);
	dev->open = spidermouse_open;
	dev->close = spidermouse_close;

	err = input_register_device(dev);
	if (err) {
		input_free_device(dev);
		return err;
	}

	err = spidermouse_set_drvdata(dev, index);
	if (err) {
		input_unregister_device(dev);
		return err;
	}

	dbg_leave();

	return 0;
}

static void spidermouse_enable(int dev_type)
{
	int i;
	int index;

	dbg_enter();

	index = spidermouse_to_index(dev_type);

	if (SM_DEV_TYPE_MOUSE == dev_type) {
		pr_info("spidermouse: %s: MHL cable connected\n", __func__);

		for (i = 0; i < SPIDERMHL_MOUSE_DEVCONFIG; i++) {
			spidermouse_connect(i);
		}
	} else {
		spidermouse_connect(index);
	}

	dbg_leave();
}

static void spidermouse_disconnect(int index)
{
	struct input_dev *dev;

	dbg_enter();

	dev = spidermouse_get_drvdata(index);

	if (!dev || !dev->users) {
		pr_warn("spidermouse: %s: %s mouse not opened\n",
			__func__, (!index) ? "PS2" : "USB");
		return;
	}

	spidermouse_set_drvdata(NULL, index);

	input_unregister_device(dev);

/** need to check below
 * input_free_device - free memory occupied by input_dev structure
 * This function should only be used if input_register_device()
 * was not called yet or if it failed. Once device was registered
 * use input_unregister_device() and memory will be freed once last
 * reference to the device is dropped.
 *
 * NOTE: If there are references to the input device then memory
 * will not be freed until last reference is dropped.
 *
 * input_register_device - register device with input core
 * This function registers device with input core. The device must be
 * allocated with input_allocate_device() and all it's capabilities
 * set up before registering.
 * If function fails the device must be freed with input_free_device().
 * Once device has been successfully registered it can be unregistered
 * with input_unregister_device(); input_free_device() should not be
 * called in this case.
 */
#if 0
	input_free_device(dev);
#endif
	dbg_leave();
}

static int spidermouse_disable(int dev_type)
{
	int i;
	int index;

	dbg_enter();

	index = spidermouse_to_index(dev_type);

	if (SM_DEV_TYPE_MOUSE == dev_type) {
		pr_info("spidermouse: %s: MHL cable disconnected\n", __func__);

		for (i = 0; i < SPIDERMHL_MOUSE_DEVCONFIG; i++) {
			spidermouse_disconnect(i);
		}
	} else {
		spidermouse_disconnect(index);
	}

	dbg_leave();

	return 0;
}

static void spidermouse_control(struct work_struct *work)
{
	int new_state;
	int dev_type;

	dbg_enter();

	new_state = SM_DEV_STATE_MASK & queued_event.event_state;
	dev_type = SM_DEV_TYPE_MOUSE & queued_event.dev_type;

	if (SM_DEV_STATE_CONNECTED & new_state)
		spidermouse_enable(dev_type);
	else
		spidermouse_disable(dev_type);

	dbg_leave();
}

static int spidermouse_get_event(struct notifier_block *this, unsigned long unused, void *data)
{
	struct spider_event *event = (struct spider_event *)data;

	int new_state;
	int new_event;

	if (NULL == event) {
		pr_err("spidermouse: %s: NULL event\n", __func__);
		return NOTIFY_DONE;
	}

	dbg_enter();

	new_state = SM_DEV_STATE_MASK & event->event_state;
	new_event = SM_DEV_EVENT_MOUSE & event->event_state;

	/* state changed */
	if (new_state) {
		if (!(SM_DEV_TYPE_MOUSE & event->dev_type)) {
			pr_debug("spidermouse: %s: not for mouse\n", __func__);
		} else {
			pr_info("spidermouse: %s: SM_DEV_STATE_CHANGED\n",
								__func__);

			memcpy(&queued_event, event, 16);
			queue_work(spidermouse_wq, &spidermouse_work);
		}
	}

	/* event received */
	if (new_event) {
		pr_debug("spidermouse: %s: SM_DEV_EVENT_RECEIVED\n", __func__);

		if (new_event & SM_DEV_EVENT_MOUSE_PS2) {
			event->event_state = SM_DEV_EVENT_MOUSE_PS2;
			spidermouse_handle_event(event);
		}

		if (new_event & SM_DEV_EVENT_MOUSE_USB) {
			event->event_state = SM_DEV_EVENT_MOUSE_USB;
			spidermouse_handle_event(event);
		}

	}

	dbg_leave();

	return NOTIFY_DONE;
}

static struct notifier_block spidermouse_notifier = {
	.notifier_call = spidermouse_get_event,
};

static int __init spidermouse_probe(void)
{
	int i;

	dbg_enter();

	for (i = 0; i < SPIDERMHL_MOUSE_DEVCONFIG; i++) {
		spidermouse_driver.dev[i] = NULL;
	}

	spidermouse_wq = create_singlethread_workqueue("spidermouse_wq");
	INIT_WORK(&spidermouse_work, spidermouse_control);

	spider_register_notifier(&spidermouse_notifier);

	dbg_leave();

	return 0;
}

static int __exit spidermouse_remove(void)
{
	dbg_enter();

	spider_unregister_notifier(&spidermouse_notifier);

	dbg_leave();

	return 0;
}

static int __init spidermouse_init(void)
{
	int ret;

	pr_info("spidermhl: version %s\n", SPIDERMHL_VERSION);

	dbg_enter();

	ret = spidermouse_probe();
	if (0 > ret)
		pr_err("spidermouse: %s: spidermouse_probe failed(%d)\n",
								__func__, ret);

	dbg_leave();

	return ret;
}
module_init(spidermouse_init);

static void __exit spidermouse_exit(void)
{
	dbg_enter();

	spidermouse_remove();

	dbg_leave();
}
module_exit(spidermouse_exit);

MODULE_ALIAS("spidermhl-mouse");
