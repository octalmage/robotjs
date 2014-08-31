#pragma once
#ifndef MICROSLEEP_H
#define MICROSLEEP_H

#include "os.h"
#include "inline_keywords.h"

#if !defined(IS_WINDOWS)
	/* Make sure nanosleep gets defined even when using C89. */
	#if !defined(__USE_POSIX199309) || !__USE_POSIX199309
		#define __USE_POSIX199309 1
	#endif

	#include <time.h> /* For nanosleep() */
#endif

/*
 * A more widely supported alternative to usleep(), based on Sleep() in Windows
 * and nanosleep() everywhere else.
 *
 * Pauses execution for the given amount of milliseconds.
 */
H_INLINE void microsleep(double milliseconds)
{
#if defined(IS_WINDOWS)
	Sleep((DWORD)milliseconds); /* (Unfortunately truncated to a 32-bit integer.) */
#else
	/* Technically, nanosleep() is not an ANSI function, but it is the most
	 * supported precise sleeping function I can find.
	 *
	 * If it is really necessary, it may be possible to emulate this with some
	 * hack using select() in the future if we really have to. */
	struct timespec sleepytime;
	sleepytime.tv_sec = milliseconds / 1000;
	sleepytime.tv_nsec = (milliseconds - (sleepytime.tv_sec * 1000)) * 1000000;
	nanosleep(&sleepytime, NULL);
#endif
}

#endif /* MICROSLEEP_H */
