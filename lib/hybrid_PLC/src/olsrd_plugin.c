/*
 * Copyright (c) 2005, Bruno Randolf <bruno.randolf@4g-systems.biz>
 * Copyright (c) 2004, Andreas Tonnesen(andreto-at-olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * Neither the name of the UniK olsr daemon nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * Example plugin for olsrd.org OLSR daemon
 * Only the bare minimum
 */

#include <stdio.h>
#include <string.h>

#include "../../../src/olsrd_plugin.h"

#include "olsrd_plugin.h"
#include "olsr.h"
#include "defs.h"
#include "scheduler.h"

#include <unistd.h>
#include <sys/un.h>
#include <errno.h>
#include <ctype.h>

#include "linklayer_plc_data.h"
//#include "sockraw.h"



#define PLUGIN_INTERFACE_VERSION 5

static void update_plc(void);

/****************************************************************************
 *                Functions that the plugin MUST provide                    *
 ****************************************************************************/

/**
 * Plugin interface version
 * Used by main olsrd to check plugin interface version
 */
int olsrd_plugin_interface_version(void) {
	return PLUGIN_INTERFACE_VERSION;
}

//static int set_plugin_plc_mac(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
//	int i, j;
//	unsigned int temp;
//	u_char ch;
//	i = 0;
//	j = 0;
//	printf("\n*** Hybrid PLC: parameter plc_mac: %s\n", value);
//	while (value[i] != '\0') {
//		if (value[i] == ':') {
//			i++;
//			continue;
//		}
//		//Convert letter into lower case.
//		ch = tolower(value[i]);
//		printf("\n*** Current character: %c\n", ch);
//		if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f')) {
//			return 1;
//		}
//		sscanf(value+i, "%02x", &temp);
//		printf("\n*** temp: %02x\n", temp);
//		plc_mac[j] = temp;
//		printf("\n*** plc_mac[2]: %02x\n", plc_mac[2]);
//		i = i+2;
//		j++;
//	}
//	printf("PLC MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", plc_mac[0], plc_mac[1], plc_mac[2], plc_mac[3], plc_mac[4], plc_mac[5]);
//	return 0;
//}

static int
set_plugin_test(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  olsr_printf(1, "\n*** hybrid_PLC: parameter test: %s\n", value);
  return 0;
}

/**
 * Register parameters from config file
 * Called for all plugin parameters
 */
static const struct olsrd_plugin_parameters plugin_parameters[] = {
  {.name = "test",.set_plugin_parameter = &set_plugin_test,.data = NULL},
};

void
olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size)
{
  *params = plugin_parameters;
  *size = sizeof(plugin_parameters) / sizeof(*plugin_parameters);
}

/**
 * Initialize plugin
 * Called after all parameters are passed
 */
int olsrd_plugin_init(void) {
	int r;
  printf("*** Hybrid PLC: plugin_init\n");
	printf("sono nel PLUGIN!!!\n");
	init_plc_peer_neighbors();
	r = init_plc_communication("eth0");
	if (r == 1)
	  olsr_printf(1, "olsrd: plugin: sockraw: pcap init success!");
	olsr_start_timer(5 * MSEC_PER_SEC, 0, OLSR_TIMER_PERIODIC, &update_plc, NULL, 0);
  return 1;
}



static void update_plc(void) {
  olsr_printf(3, "olsrd: plugin: timer fired up!");
  update_plc_data();
}





/****************************************************************************
 *       Optional private constructor and destructor functions              *
 ****************************************************************************/

/* attention: make static to avoid name clashes */

static void my_init(void) __attribute__ ((constructor));
static void my_fini(void) __attribute__ ((destructor));

/**
 * Optional Private Constructor
 */
static void my_init(void) {
	printf("*** Hybrid PLC: constructor\n");
}

/**
 * Optional Private Destructor
 */
static void my_fini(void) {
	printf("*** Hybrid PLC: destructor\n");
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

