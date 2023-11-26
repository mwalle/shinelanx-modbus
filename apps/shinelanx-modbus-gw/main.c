/*
 * Copyright (c) 2023, Michael Walle <michael@walle.cc>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <nuttx/config.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <fcntl.h>

#include "netutils/netinit.h"
#include "netutils/netlib.h"
#include "netutils/dhcpc.h"

#include <nuttx/leds/userled.h>

#define STM32_SYSMEM_UID	0x1ffff7e8

/*
 * Use the same MAC calculation as the original ShineLAN-X code.
 * Reverese engineered from the function located at 0x0800f8b6 of
 * firmware version 3.6.0.2, which uses the STM32 CRC module. This
 * module uses the CRC32-MPEG2 algo.
 *
 * As we shouldn't access the registers from a nuttx application
 * and there is no driver for the CRC32 peripheral, just calculate
 * the checksum in software. It is only used once during startup
 * for 12 bytes.
 */

uint32_t crc32mpeg2(FAR uint8_t *src, int len)
{
  uint32_t crc = 0xffffffff;
  bool msb;
  int i;

  while (len--) {
    crc ^= (uint32_t)(*(src++)) << 24;
    for (i = 8; i; i--) {
      msb = crc & (1 << 31);
      crc <<= 1;
      if (msb)
        crc ^= 0x04c11db7;
     }
  }

  return crc;
}

#ifndef CONFIG_NETINIT_NOMAC
static void set_mac_address(void)
{
	uint8_t mac[IFHWADDRLEN];
	uint32_t lower_mac;
	uint32_t uid[3];
	int i;

	/* copy UID and swap */
	for (i = 0; i < nitems(uid); i++)
		uid[i] = swap32(((uint32_t*)STM32_SYSMEM_UID)[i]);

	lower_mac = crc32mpeg2((uint8_t*)uid, sizeof(uid));

	mac[0] = 0x00;
	mac[1] = 0x47;
	mac[2] = (lower_mac >> 24) & 0xff;
	mac[3] = (lower_mac >> 16) & 0xff;
	mac[4] = (lower_mac >> 8) & 0xff;
	mac[5] = lower_mac & 0xff;

	printf("Using MAC %02X:%02X:%02X:%02X:%02X:%02X\n",
	       mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	netlib_setmacaddr("eth0", mac);
}
#endif

static void init_leds(void)
{
	int fd;

	fd = open("/dev/userleds", 0);
	if (fd == -1)
		return;

	ioctl(fd, ULEDIOC_SETALL, (userled_set_t)2);
	close(fd);
}

static void dhcpc_callback(FAR struct dhcpc_state *ds)
{
	netlib_set_ipv4addr("eth0", &ds->ipaddr);
	netlib_set_ipv4netmask("eth0", &ds->netmask);
	netlib_set_dripv4addr("eth0", &ds->default_router);
	netlib_set_ipv4dnsaddr(&ds->dnsaddr);
}

static void start_dhcp(void)
{
	uint8_t mac[IFHWADDRLEN];
	FAR void *handle;

	netlib_getmacaddr("eth0", mac);

	netlib_ifup("eth0");

	netlib_getmacaddr("eth0", mac);
	handle = dhcpc_open("eth0", &mac, IFHWADDRLEN);
	dhcpc_request_async(handle, dhcpc_callback);
}

static char *mbusd_main_args[] = { "mbusd", "-p", "/dev/ttyACM0", "-d", "-b", "-L-", "-R10" };
int mbusd_main(int argc, char **argv);

int main(int argc, FAR char *argv[])
{
	printf("ShineLAN-X Modbus/TCP gateway\n");
	init_leds();

#ifndef CONFIG_NETINIT_NOMAC
	set_mac_address();
#endif

#ifndef CONFIG_NSH_NETINIT
#ifdef CONFIG_NETUTILS_NETINIT
	netinit_bringup();
#elif CONFIG_NETUTILS_DHCPC
	start_dhcp();
#endif
#endif

	/* XXX wait until ttyACM is available */
	sleep(1);

	mbusd_main(nitems(mbusd_main_args), mbusd_main_args);

	return 0;
}
