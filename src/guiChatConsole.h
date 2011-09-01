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

#ifndef GUICHATCONSOLE_HEADER
#define GUICHATCONSOLE_HEADER

#include "common_irrlicht.h"

class GUIChatConsole : public gui::IGUIElement
{
public:
	GUIChatConsole(gui::IGUIEnvironment* env,
			gui::IGUIElement* parent, s32 id);
	virtual ~GUIChatConsole();

	// Open the console (height = desired fraction of screen size)
	// This doesn't open immediately but initiates an animation.
	void openConsole(f32 height);
	// Close the console, equivalent to openConsole(0).
	// This doesn't close immediately but initiates an animation.
	void closeConsole();
	// Close the console immediately, without animation.
	void closeConsoleAtOnce();

	// Return the desired height (fraction of screen size)
	// Zero if the console is closed or getting closed
	f32 getDesiredHeight() const;

	// Change how the cursor looks
	void setCursor(
		bool visible,
		bool blinking = false,
		f32 blink_speed = 1.0,
		f32 relative_height = 1.0);

	// Irrlicht draw method
	virtual void draw();

	bool canTakeFocus(gui::IGUIElement* element) { return false; }

	virtual bool OnEvent(const SEvent& event);

private:
	// These methods are called by draw
	void animate(u32 msec);
	void drawBackground();
	void drawText();
	void drawPrompt();
	void drawCursor();

private:
	// current screen size
	v2u32 m_screensize;

	// used to compute how much time passed since last animate()
	u32 m_animate_time_old;

	// current console height [pixels]
	s32 m_height;
	// if true, force next animate() to resize the GUI element
	bool m_height_changed;
	// desired height [screen height fraction]
	f32 m_desired_height_fraction;
	// console open/close animation speed [screen height fraction / second]
	f32 m_height_speed;

	// cursor blink frame (16-bit value)
	// cursor is off during [0,32767] and on during [32768,65535]
	u32 m_cursor_blink;
	// cursor blink speed [on/off toggles / second]
	f32 m_cursor_blink_speed;
	// cursor height [line height]
	f32 m_cursor_height;

	// background texture
	video::ITexture* m_background;
};


#endif

