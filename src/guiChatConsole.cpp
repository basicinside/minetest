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
#include "main.h"  // for g_settings
#include "tile.h"
#include <cmath>
#include <string>

#include "gettext.h"

// TODO text display
// TODO text input
// TODO test setCursor()

inline u32 clamp_u8(s32 value)
{
	return (u32) MYMIN(MYMAX(value, 0), 255);
}


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
	m_open(false),
	m_height(0),
	m_desired_height(0),
	m_desired_height_fraction(0.0),
	m_height_speed(1.5),
	m_open_inhibited(0),
	m_cursor_blink(0.0),
	m_cursor_blink_speed(0.0),
	m_cursor_height(0.0),
	m_background(NULL),
	m_background_color(255, 0, 0, 0),
	m_background_alpha(255),
	m_font(NULL),
	m_fontsize(0, 0)
{
	m_animate_time_old = getTimeMs();

	// load background settings
	bool console_color_set = !g_settings.get("console_color").empty();
	v3f console_color = g_settings.getV3F("console_color");
	s32 console_alpha = g_settings.getS32("console_alpha");

	// load the background texture depending on settings
	m_background_alpha = clamp_u8(console_alpha);
	if (console_color_set)
	{
		m_background_color.setAlpha(m_background_alpha);
		m_background_color.setRed(clamp_u8(round(console_color.X)));
		m_background_color.setGreen(clamp_u8(round(console_color.Y)));
		m_background_color.setBlue(clamp_u8(round(console_color.Z)));
	}
	else
	{
		m_background = env->getVideoDriver()->getTexture(getTexturePath("background_chat.jpg").c_str());
	}

	// load the font
	m_font = env->getSkin()->getFont();
	if (m_font != NULL)
	{
		core::dimension2d<u32> dim = m_font->getDimension(L"M");
		m_fontsize = v2u32(dim.Width, dim.Height);
		dstream << "Font size: " << m_fontsize.X << " " << m_fontsize.Y << std::endl;
	}
	m_fontsize.X = MYMAX(m_fontsize.X, 1);
	m_fontsize.Y = MYMAX(m_fontsize.Y, 1);

	// set default cursor options
	setCursor(true, true, 0.5, 0.1);
}

GUIChatConsole::~GUIChatConsole()
{
}

void GUIChatConsole::openConsole(f32 height)
{
	m_open = true;
	m_desired_height_fraction = height;
	m_desired_height = height * m_screensize.Y;
	reformatConsole();
}

bool GUIChatConsole::isOpenInhibited() const
{
	return m_open_inhibited > 0;
}

void GUIChatConsole::closeConsole()
{
	m_open = false;
}

void GUIChatConsole::closeConsoleAtOnce()
{
	m_open = false;
	m_height = 0;
	recalculateConsolePosition();
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
		m_desired_height = m_desired_height_fraction * m_screensize.Y;
		m_screensize = screensize;
		reformatConsole();
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

void GUIChatConsole::reformatConsole()
{
	s32 cols = m_screensize.X / m_fontsize.X - 2; // make room for a margin (looks better)
	s32 rows = m_desired_height / m_fontsize.Y - 1; // make room for the input prompt
	dstream << "GUIChatConsole: reformat cols " << cols << " rows " << rows << std::endl;
	if (cols <= 0 || rows <= 0)
		cols = rows = 0;
	m_chat_backend->reformat(cols, rows);
}

void GUIChatConsole::recalculateConsolePosition()
{
	core::rect<s32> rect(0, 0, m_screensize.X, m_height);
	DesiredRect = rect;
	recalculateAbsolutePosition(false);
}

void GUIChatConsole::animate(u32 msec)
{
	// animate the console height
	s32 goal = m_open ? m_desired_height : 0;
	if (m_height != goal)
	{
		s32 max_change = msec * m_screensize.Y * (m_height_speed / 1000.0);
		if (max_change == 0)
			max_change = 1;

		if (m_height < goal)
		{
			// increase height
			if (m_height + max_change < goal)
				m_height += max_change;
			else
				m_height = goal;
		}
		else
		{
			// decrease height
			if (m_height > goal + max_change)
				m_height -= max_change;
			else
				m_height = goal;
		}

		recalculateConsolePosition();
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
	video::IVideoDriver* driver = Environment->getVideoDriver();
	if (m_background != NULL)
	{
		core::rect<s32> sourcerect(0, -m_height, m_screensize.X, 0);
		driver->draw2DImage(
			m_background,
			v2s32(0, 0),
			sourcerect,
			&AbsoluteClippingRect,
			video::SColor(m_background_alpha, 255, 255, 255),
			false);
	}
	else
	{
		driver->draw2DRectangle(
			m_background_color,
			core::rect<s32>(0, 0, m_screensize.X, m_height),
			&AbsoluteClippingRect);
	}
}

void GUIChatConsole::drawText()
{
	if (m_font == NULL)
		return;

	ChatBuffer& buf = m_chat_backend->getConsoleBuffer();
	for (u32 row = 0; row < buf.getRows(); ++row)
	{
		const ChatFormattedLine& line = buf.getFormattedLine(row);
		if (line.fragments.empty())
			continue;

		s32 line_height = m_fontsize.Y;
		s32 y = row * line_height + m_height - m_desired_height;
		if (y + line_height < 0)
			continue;

		for (u32 i = 0; i < line.fragments.size(); ++i)
		{
			const ChatFormattedFragment& fragment = line.fragments[i];
			s32 x = (fragment.column + 1) * m_fontsize.X;
			core::rect<s32> destrect(
				x, y, x + m_fontsize.X * fragment.text.size(), y + m_fontsize.Y);
			m_font->draw(
				fragment.text.c_str(),
				destrect,
				video::SColor(255, 255, 255, 255),
				false,
				false,
				&AbsoluteClippingRect);
		}
	}
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
	if(event.EventType == EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown)
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
		else if(event.KeyInput.Key == KEY_ESCAPE)
		{
			closeConsoleAtOnce();
			Environment->removeFocus(this);
			// the_game will open the pause menu
			return true;
		}
		else if(event.KeyInput.Key == KEY_PRIOR)
		{
			m_chat_backend->scrollPageUp();
			return true;
		}
		else if(event.KeyInput.Key == KEY_NEXT)
		{
			m_chat_backend->scrollPageDown();
			return true;
		}
		else if(event.KeyInput.Key == KEY_RETURN)
		{
			dstream<<"GUIChatConsole: Enter pressed"<<std::endl;
			// TODO: submit message
			return true;
		}
		else if(event.KeyInput.Key == KEY_BACK)
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
		else if(event.KeyInput.Key == KEY_DELETE)
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
		else if(event.KeyInput.Key == KEY_KEY_U && event.KeyInput.Control)
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
		else if(event.KeyInput.Char == L'd')
		{
			// DEBUG
			ChatBuffer& buf = m_chat_backend->getConsoleBuffer();
			for (u32 row = 0; row < buf.getRows(); ++row)
			{
				const ChatFormattedLine& line = buf.getFormattedLine(row);
				dstream << "row " << row << (line.first ? " [first] " : "") << std::endl;
				for (u32 i = 0; i < line.fragments.size(); ++i)
				{
					const ChatFormattedFragment& fragment = line.fragments[i];
					dstream << "fragment len=" << fragment.text.size() << " column=" << fragment.column << " bold=" << fragment.bold << " text=\"";
					for (u32 j = 0; j < fragment.text.size(); ++j)
						dstream << (char) fragment.text[j];
					dstream << "\"" << std::endl;
				}
			}
		}
		else if(event.KeyInput.Char != 0 && !event.KeyInput.Control)
		{
			// TODO: input text
			wchar_t ch = event.KeyInput.Char;
			dstream<<"GUIChatConsole: input "<<ch<<std::endl;
			return true;
		}
	}
	else if(event.EventType == EET_MOUSE_INPUT_EVENT)
	{
		if(event.MouseInput.Event == EMIE_MOUSE_WHEEL)
		{
			s32 rows = round(-3.0 * event.MouseInput.Wheel);
			m_chat_backend->scroll(rows);
		}
	}

	return Parent ? Parent->OnEvent(event) : false;
}

