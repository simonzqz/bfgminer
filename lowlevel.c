/*
 * Copyright 2012-2013 Luke Dashjr
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.  See COPYING for more details.
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include <utlist.h>

#include "lowlevel.h"

static struct lowlevel_device_info *devinfo_list;

void lowlevel_devinfo_free(struct lowlevel_device_info * const info)
{
	if (info->lowl->devinfo_free)
		info->lowl->devinfo_free(info);
	free(info->manufacturer);
	free(info->product);
	free(info->serial);
	free(info->path);
	free(info);
}

void lowlevel_scan_free()
{
	if (!devinfo_list)
		return;
	
	struct lowlevel_device_info *info, *tmp;
	
	LL_FOREACH_SAFE(devinfo_list, info, tmp)
	{
		LL_DELETE(devinfo_list, info);
		lowlevel_devinfo_free(info);
	}
}

void lowlevel_scan()
{
	struct lowlevel_device_info *devinfo_mid_list;
	
	lowlevel_scan_free();
	
#ifdef USE_X6500
	devinfo_mid_list = lowl_ft232r.devinfo_scan();
	LL_CONCAT(devinfo_list, devinfo_mid_list);
#endif
	
#ifdef USE_NANOFURY
	devinfo_mid_list = lowl_mcp2210.devinfo_scan();
	LL_CONCAT(devinfo_list, devinfo_mid_list);
#endif
}

int _lowlevel_detect(lowl_found_devinfo_func_t cb, const char *serial, const char **product_needles)
{
	struct lowlevel_device_info *info, *tmp;
	int found = 0, i;
	
	LL_FOREACH_SAFE(devinfo_list, info, tmp)
	{
		if (serial && strcmp(serial, info->serial))
			continue;
		for (i = 0; product_needles[i]; ++i)
			if (!strstr(info->product, product_needles[i]))
				goto next;
		if (!cb(info))
			continue;
		LL_DELETE(devinfo_list, info);
		++found;
next: ;
	}
	
	return found;
}
