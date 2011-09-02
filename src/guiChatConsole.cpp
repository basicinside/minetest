/*
Minetest-c55
Copyright (C) 2011 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#include "guiChatConsole.h"
#include "chat.h"
#include "debug.h"
#include "gettime.h"
#include "keycode.h"
#include "tile.h"
#include <cmath>
#include <string>

#include "gettext.h"

// TODO text display
// TODO text input
// TODO test setCursor()

GUIChatConsole::GUIChatConsole(
		gui::IGUIEnvironment* env,
		gui::IGUIElement* parent,
		s32 id,
		ChatBackend* backend
):
	IGUIElement(gui::EGUIET_ELEMENT, env, parent, id,
			core::rect<s32>(0,0,100,100)),
	m_chat_backend(backend),
	m_screensize(v2u32(0,0)),
	m_animate_time_old(0),
	m_height(0),
	m_height_changed(true),
	m_desired_height(0),
	m_desired_height_fraction(0.0),
	m_height_speed(1.5),
	m_open_inhibited(0),
	m_cursor_blink(0.0),
	m_cursor_blink_speed(0.0),
	m_cursor_height(0.0),
	m_background(NULL),
	m_font(NULL),
	m_fontsize(0, 0)
{
	m_animate_time_old = getTimeMs();

	// load the background texture
	m_background = env->getVideoDriver()->getTexture(getTexturePath("background_chat.jpg").c_str());

	// load the font
	m_font = env->getSkin()->getFont();
	if (m_font != NULL)
	{
		core::dimension2d<u32> dim = m_font->getDimension(L"M");
		m_fontsize = v2u32(dim.Width, dim.Height);
		dstream << "Font size: " << m_fontsize.X << " " << m_fontsize.Y << std::endl;
	}
	if (m_fontsize.X <= 0)
		m_fontsize.X = 1;
	if (m_fontsize.Y <= 0)
		m_fontsize.Y = 1;

	// set default cursor options
	setCursor(true, true, 0.5, 0.1);

	// TODO remove the ChatBuffer test
	dstream << "*** ChatBuffer test ***" << std::endl;
	ChatBuffer buf(500);
	buf.reformat(3, 80);
	buf.addLine(std::wstring(L""), std::wstring(L"# Server version 0.2.20110731_3"));
	buf.addLine(std::wstring(L""), std::wstring(L"# MOTD"));
	buf.addLine(std::wstring(L"kahrl"), std::wstring(L"Hello world! Lorem ipsum dolor sit amet, consectetuer adipisicing elit. Lorem ipsum dolor sit amet, consectetuer adipisicing elit. Lorem ipsum dolor sit amet, consectetuer adipisicing elit. Lorem ipsum dolor sit amet, consectetuer adipisicing elit. LOLHAXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX!!!!!!!!!!!!!!!!!!!!!!!!!1111111111111111 ftw!"));
	buf.scrollTop();
	buf.scroll(1);

	dstream << "Scrollback = " << buf.getScrollback() << std::endl;
	dstream << "Line count = " << buf.getLineCount() << std::endl;


	for (u32 i = 0; i < buf.getRows(); ++i)
	{
		const ChatFormattedLine& fl = buf.getFormattedLine(i);
		dstream << "Formatted line " << i << ", first=" << (fl.first ? "yes" : "no") << std::endl;
		for (u32 j = 0; j < fl.fragments.size(); ++j)
		{
			ChatFormattedFragment frag = fl.fragments[j];
			dstream << "  Fragment " << j << ", len=" << frag.text.size() << ", text=\"";
			for (const wchar_t* ptr = frag.text.c_str(); *ptr; ++ptr)
				dstream << (char) (*ptr);
			dstream << "\", column=" << frag.column << ", bold=" << (bool) (frag.bold) << std::endl;
		}
	}
	dstream << "*** ChatBuffer test ended ***" << std::endl;
}

GUIChatConsole::~GUIChatConsole()
{
}

void GUIChatConsole::openConsole(f32 height)
{
	m_desired_height_fraction = height;
}

bool GUIChatConsole::isOpenInhibited() const
{
	return m_open_inhibited > 0;
}

void GUIChatConsole::closeConsole()
{
	m_desired_height_fraction = 0.0;
}

void GUIChatConsole::closeConsoleAtOnce()
{
	m_desired_height_fraction = 0.0;
	m_height = 0;
	m_height_changed = true;
}

f32 GUIChatConsole::getDesiredHeight() const
{
	return m_desired_height_fraction;
}

void GUIChatConsole::setCursor(
	bool visible, bool blinking, f32 blink_speed, f32 relative_height)
{
	if (visible)
	{
		if (blinking)
		{
			// leave m_cursor_blink unchanged
			m_cursor_blink_speed = blink_speed;
		}
		else
		{
			m_cursor_blink = 0x8000;  // on
			m_cursor_blink_speed = 0.0;
		}
	}
	else
	{
		m_cursor_blink = 0;  // off
		m_cursor_blink_speed = 0.0;
	}
	m_cursor_height = relative_height;
}

void GUIChatConsole::draw()
{
	if(!IsVisible)
		return;

	video::IVideoDriver* driver = Environment->getVideoDriver();

	// Check screen size
	v2u32 screensize = driver->getScreenSize();
	if (screensize != m_screensize)
	{
		// screen size has changed
		// scale current console height to new window size
		if (m_screensize.Y != 0)
			m_height = m_height * screensize.Y / m_screensize.Y;
		m_screensize = screensize;

		// recompute console size

	}

	// Animation
	u32 now = getTimeMs();
	animate(now - m_animate_time_old);
	m_animate_time_old = now;

	// Draw console elements if visible
	if (m_height > 0)
	{
		drawBackground();
		drawText();
		drawPrompt();
		drawCursor();
	}

	gui::IGUIElement::draw();
}

void GUIChatConsole::animate(u32 msec)
{
	// animate the console height
	s32 screenheight = m_screensize.Y;
	m_desired_height = m_desired_height_fraction * screenheight;
	if (m_height != m_desired_height)
	{
		s32 max_change = msec * screenheight * (m_height_speed / 1000.0);
		if (max_change == 0)
			max_change = 1;

		if (m_height < m_desired_height)
		{
			// increase height
			if (m_height + max_change < m_desired_height)
				m_height += max_change;
			else
				m_height = m_desired_height;
		}
		else
		{
			// decrease height
			if (m_height > m_desired_height + max_change)
				m_height -= max_change;
			else
				m_height = m_desired_height;
		}
		m_height_changed = true;
	}
	if (m_height_changed)
	{
		core::rect<s32> rect(0, 0, m_screensize.X, m_height);
		DesiredRect = rect;
		recalculateAbsolutePosition(false);
	}

	// blink the cursor
	if (m_cursor_blink_speed != 0.0)
	{
		u32 blink_increase = 0x10000 * msec * (m_cursor_blink_speed / 1000.0);
		if (blink_increase == 0)
			blink_increase = 1;
		m_cursor_blink = ((m_cursor_blink + blink_increase) & 0xffff);
	}

	// decrease open inhibit counter
	if (m_open_inhibited > msec)
		m_open_inhibited -= msec;
	else
		m_open_inhibited = 0;
}

void GUIChatConsole::drawBackground()
{
	if (m_background == NULL)
		return;

	video::IVideoDriver* driver = Environment->getVideoDriver();
	core::rect<s32> sourcerect(0, -m_height, m_screensize.X, 0);
	driver->draw2DImage(
		m_background,
		v2s32(0, 0),
		sourcerect,
		&AbsoluteClippingRect,
		video::SColor(240, 255, 255, 255),
		false);
}

void GUIChatConsole::drawText()
{
	if (m_font == NULL)
		return;

	core::rect<s32> destrect(20, 20, 100, 100);
	core::rect<s32> destrect2(20 + m_fontsize.X, 20 + m_fontsize.Y, 100 + m_fontsize.X, 100 + m_fontsize.Y);
	m_font->draw(L"Hello", destrect, video::SColor(255, 255, 255, 255), false, false, &AbsoluteClippingRect);
	m_font->draw(L"World", destrect2, video::SColor(255, 255, 255, 255), false, false, &AbsoluteClippingRect);
}

void GUIChatConsole::drawPrompt()
{
}

void GUIChatConsole::drawCursor()
{
	video::IVideoDriver* driver = Environment->getVideoDriver();
	video::SColor bgcolor(255,255,255,255);
	if ((m_cursor_blink & 0x8000) != 0)
	{
		driver->draw2DRectangle(bgcolor, core::rect<s32>(0, 0, 10, 10), &AbsoluteClippingRect);
	}
}

bool GUIChatConsole::OnEvent(const SEvent& event)
{
	if(event.EventType==EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown)
	{
		// Key input
		if(KeyPress(event.KeyInput) == getKeySetting("keymap_console"))
		{
			closeConsole();
			Environment->removeFocus(this);

			// inhibit open so the_game doesn't reopen immediately
			m_open_inhibited = 50;
			return true;
		}
		if(event.KeyInput.Key == KEY_ESCAPE)
		{
			closeConsoleAtOnce();
			Environment->removeFocus(this);
			// the_game will open the pause menu
			return true;
		}
		if(event.KeyInput.Key == KEY_RETURN)
		{
			dstream<<"GUIChatConsole: Enter pressed"<<std::endl;
			// TODO: submit message
			return true;
		}
		if(event.KeyInput.Key == KEY_BACK)
		{
			if(event.KeyInput.Control)
			{
				dstream<<"GUIChatConsole: Ctrl-Backspace pressed"<<std::endl;
				// TODO: backspace word
			}
			else
			{
				dstream<<"GUIChatConsole: Backspace pressed"<<std::endl;
				// TODO: backspace
			}
			return true;
		}
		if(event.KeyInput.Key == KEY_DELETE)
		{
			if(event.KeyInput.Control)
			{
				dstream<<"GUIChatConsole: Ctrl-Delete pressed"<<std::endl;
				// TODO: delete word
			}
			else
			{
				dstream<<"GUIChatConsole: Delete pressed"<<std::endl;
				// TODO: delete
			}
			return true;
		}
		if(event.KeyInput.Key == KEY_KEY_U && event.KeyInput.Control)
		{

			if(event.KeyInput.Control)
			{
				dstream<<"GUIChatConsole: Ctrl-Delete pressed"<<std::endl;
				// TODO: delete word
			}
			else
			{
				dstream<<"GUIChatConsole: Delete pressed"<<std::endl;
				// TODO: delete
			}
			return true;
		}
		if(event.KeyInput.Char != 0 && !event.KeyInput.Control)
		{
			// TODO: input text
			wchar_t ch = event.KeyInput.Char;
			dstream<<"GUIChatConsole: input "<<ch<<std::endl;
			return true;
		}
	}

	return Parent ? Parent->OnEvent(event) : false;
}

