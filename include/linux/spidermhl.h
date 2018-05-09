/*
 *  Spider-MHL driver
 *
 *  Copyright (c) 2012 Helixtech Inc.
 *
 *  Ray Moon <pianist@helixtech.co.kr>
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation
 */

#ifndef _SPIDERMHL_H_
#define _SPIDERMHL_H_

/* message types sent from SM */
#define SM_MSG_MHL_CONNECT		1
#define SM_MSG_MHL_DISCONNECT		2

/* device types sent from SM */
#define SM_DEV_TYPE_MHL			0x01
#define SM_DEV_TYPE_KEYBOARD_PS2	0x02
#define SM_DEV_TYPE_KEYBOARD_USB	0x04
#define SM_DEV_TYPE_KEYBOARD		(SM_DEV_TYPE_KEYBOARD_PS2 | \
						SM_DEV_TYPE_KEYBOARD_USB)
#define SM_DEV_TYPE_MOUSE_PS2		0x08
#define SM_DEV_TYPE_MOUSE_USB		0x10
#define SM_DEV_TYPE_MOUSE		(SM_DEV_TYPE_MOUSE_PS2 | \
						SM_DEV_TYPE_MOUSE_USB)
#define SM_DEV_TYPE_UNKNOWN		0x4000
#define SM_DEV_TYPE_HEARTBEAT		0x8000
#define SM_DEV_TYPE_MASK		(SM_DEV_TYPE_MHL | \
						SM_DEV_TYPE_KEYBOARD | \
						SM_DEV_TYPE_MOUSE | \
						SM_DEV_TYPE_UNKNOWN | \
						SM_DEV_TYPE_HEARTBEAT)
#define SM_DEV_TYPE_NOT_MOUSE		(0xFFFF & ~SM_DEV_TYPE_MOUSE)

/* device states/events sent from SM */
#define SM_DEV_STATE_NONE		0x00
#define SM_DEV_STATE_CONNECTED		0x01
#define SM_DEV_STATE_DISCONNECTED	0x02
#define SM_DEV_STATE_MASK		(SM_DEV_STATE_CONNECTED | \
						SM_DEV_STATE_DISCONNECTED)
#define SM_DEV_EVENT_KEYBOARD		0x10
#define SM_DEV_EVENT_MOUSE_PS2		0x20
#define SM_DEV_EVENT_MOUSE_USB		0x40
#define SM_DEV_EVENT_MOUSE		(SM_DEV_EVENT_MOUSE_PS2 | \
						SM_DEV_EVENT_MOUSE_USB)
#define SM_DEV_EVENT_MASK		(SM_DEV_EVENT_KEYBOARD | \
							SM_DEV_EVENT_MOUSE)

#define SPIDER_EDID_LAPTOP_OLD		0x0142942e
#define SPIDER_EDID_LAPTOP		0xF0FF00//0xFFF0EF35

struct spider_event {
	short dev_type;		/* device type */
	char reserved;		/* reserved */
	char event_state;	/* type of event */
	char kbd_data[4];	/* event data */
	char pmouse_data[4];	/* event data */
	char umouse_data[4];	/* event data */
};

void spider_register_notifier(struct notifier_block *nb);
void spider_unregister_notifier(struct notifier_block *nb);

#ifdef CONFIG_SPIDER_MHL_DEBUG
#define dbg_enter()	pr_info("spider: +%s\n", __func__);
#define dbg_leave()	pr_info("spider: -%s\n", __func__);
#else
#define dbg_enter()
#define dbg_leave()
#endif

#endif	/* _SPIDERMHL_H_ */
