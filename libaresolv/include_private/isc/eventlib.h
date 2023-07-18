/*	$NetBSD: ev_timers.c,v 1.11 2012/03/21 00:34:54 christos Exp $	*/
/*	$NetBSD: ev_streams.c,v 1.6 2009/04/12 17:07:17 christos Exp $	*/

/*
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1995-1999 by Internet Software Consortium
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#pragma once

#include <sys/uio.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

#define BILLION 1000000000

/* Time */

static struct timespec
evConsTime(time_t sec, long nsec) {
	struct timespec x;

	x.tv_sec = sec;
	x.tv_nsec = nsec;
	return (x);
}

static struct timespec
evAddTime(struct timespec addend1, struct timespec addend2) {
	struct timespec x;

	x.tv_sec = addend1.tv_sec + addend2.tv_sec;
	x.tv_nsec = addend1.tv_nsec + addend2.tv_nsec;
	if (x.tv_nsec >= BILLION) {
		x.tv_sec++;
		x.tv_nsec -= BILLION;
	}
	return (x);
}

static struct timespec
evSubTime(struct timespec minuend, struct timespec subtrahend) {
	struct timespec x;

	x.tv_sec = minuend.tv_sec - subtrahend.tv_sec;
	if (minuend.tv_nsec >= subtrahend.tv_nsec)
		x.tv_nsec = minuend.tv_nsec - subtrahend.tv_nsec;
	else {
		x.tv_nsec = BILLION - subtrahend.tv_nsec + minuend.tv_nsec;
		x.tv_sec--;
	}
	return (x);
}

static int
evCmpTime(struct timespec a, struct timespec b) {
#define SGN(x) ((x) < 0 ? (-1) : (x) > 0 ? (1) : (0));
	time_t s = a.tv_sec - b.tv_sec;
	long n;

	if (s != 0)
		return SGN(s);

	n = a.tv_nsec - b.tv_nsec;
	return SGN(n);
}

static struct timespec
evTimeSpec(struct timeval tv) {
	struct timespec ts;

	ts.tv_sec = tv.tv_sec;
	ts.tv_nsec = tv.tv_usec * 1000;
	return (ts);
}

static struct timespec
evNowTime(void)
{
	struct timeval now;
#ifdef CLOCK_REALTIME
	struct timespec tsnow;
	int m = CLOCK_REALTIME;

	if (clock_gettime(m, &tsnow) == 0)
		return (tsnow);
#endif
	if (gettimeofday(&now, NULL) < 0)
		return (evConsTime((time_t)0, 0L));
	return (evTimeSpec(now));
}

/* IOVEC */

static struct iovec
evConsIovec(void *buf, size_t cnt) {
	struct iovec ret;

	memset(&ret, 0xf5, sizeof ret);
	ret.iov_base = buf;
	ret.iov_len = cnt;
	return (ret);
}
