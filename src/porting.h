/*
Minetest-c55
Copyright (C) 2010 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*
	Random portability stuff
*/

#ifndef PORTING_HEADER
#define PORTING_HEADER

// Included for u64 and such
#include "common_irrlicht.h"

#ifdef _WIN32
	#define SWPRINTF_CHARSTRING L"%S"
#else
	#define SWPRINTF_CHARSTRING L"%s"
#endif

#ifdef _WIN32
	#include <windows.h>
	#define sleep_ms(x) Sleep(x)
#else
	#include <unistd.h>
	#define sleep_ms(x) usleep(x*1000)
#endif

namespace porting
{

/*
	Resolution is 10-20ms.
	Remember to check for overflows.
	Overflow can occur at any value higher than 10000000.
*/
#ifdef _WIN32 // Windows
	#include <windows.h>
	inline u32 getTimeMs()
	{
		return GetTickCount();
	}
#else // Posix
	#include <sys/timeb.h>
	inline u32 getTimeMs()
	{
		struct timeb tb;
		ftime(&tb);
		return tb.time * 1000 + tb.millitm;
	}
#endif

} // namespace porting

#endif
