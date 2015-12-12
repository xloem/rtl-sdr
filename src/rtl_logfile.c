/*
 * rtl-sdr, turns your Realtek RTL2832 based DVB dongle into a SDR receiver
 * Copyright (C) 2012 by Steve Markgraf <steve@steve-m.de>
 * Copyright (C) 2012 by Hoernchen <la@tfc-server.de>
 * Copyright (C) 2012 by Kyle Keen <keenerd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/*
 * rtl_logfile: read a logfile
 * outputs CSV
 * timestart, timestop, Hz, rate, i1, q1, i2, q2, i3, q3, ...
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include "getopt/getopt.h"
#define usleep(x) Sleep(x/1000)
#ifdef _MSC_VER
#define round(x) (x > 0.0 ? floor(x + 0.5): ceil(x - 0.5))
#endif
#define _USE_MATH_DEFINES
#endif

#include "rtl-sdr.h"

void usage(void)
{
	fprintf(stderr,
		"rtl_logfile, read rtl-sdr logfiles\n"
		"\n"
		"Use:\trtl_logfile [filename]\n"
		"\n"
		"CSV output columns:\n"
		"\ttime start, time end, Hz, rate, i, q, i, q, ...\n"
		"\n"
		"Generate files by setting RTL_LOGFILE in environment\n");
	exit(1);
}

void csv(rtlsdr_dev_t *dev, uint8_t * buf, size_t len)
{
	size_t i;
	struct timespec pre_ts, post_ts;
	char *sep = ", ";

	rtlsdr_get_timestamp(dev, &pre_ts, &post_ts);
	printf("%lu.%09lu, ", pre_ts.tv_sec, pre_ts.tv_nsec);
	printf("%lu.%09lu, ", post_ts.tv_sec, post_ts.tv_nsec);

	printf("%u, ", rtlsdr_get_center_freq(dev));

	printf("%u, ", rtlsdr_get_sample_rate(dev));

	for (i=0; i < len; ++ i) {
		if (i == len-1)
			sep = "\n";
		printf("%i%s", buf[i] - 127, sep);
	}
}

int main(int argc, char **argv)
{
	char *filename = NULL;
	int r, opt;
	rtlsdr_dev_t *dev;
	uint8_t buf[0x100000];
	uint32_t read_len;

	while ((opt = getopt(argc, argv, "h")) != -1) {
		switch (opt) {
		case 'h':
		default:
			usage();
			break;
		}
	}

	if (argc <= optind) {
		filename = "-";
	} else {
		filename = argv[optind];
	}

	r = rtlsdr_open_logfile(&dev, filename);
	if (r < 0) {
		fprintf(stderr, "Failed to open logfile %s.\n", filename);
		exit(1);
	}

	while (1) {
		r = rtlsdr_read_logfile(dev, &buf, sizeof(buf), &read_len);
		if (r <= 0)
			break;
		csv(dev, buf, read_len);
	}

	r |= rtlsdr_close(dev);

	return r;
}
