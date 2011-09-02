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

#ifndef CHAT_HEADER
#define CHAT_HEADER

#include "common_irrlicht.h"
#include <string>

// Chat console related classes, only used by the client

struct ChatLine
{
	// age in seconds
	f32 age;
	// name of sending player, or empty if sent by server
	std::wstring name;
	// message text
	std::wstring text;

	ChatLine(const std::wstring& a_name, const std::wstring& a_text):
		age(0.0),
		name(a_name),
		text(a_text)
	{
	}
};

struct ChatFormattedFragment
{
	// text string
	std::wstring text;
	// starting column
	u32 column;
	// formatting
	u8 bold:1;
};

struct ChatFormattedLine
{
	// Array of text fragments
	core::array<ChatFormattedFragment> fragments;
	// true if first line of one formatted ChatLine
	bool first;
};

class ChatBuffer
{
public:
	ChatBuffer(u32 scrollback);
	~ChatBuffer();

	// Append chat line
	// Removed oldest chat line if scrollback size is reached
	void addLine(const std::wstring& name, const std::wstring& text);

	// Get number of lines currently in buffer.
	u32 getLineCount() const;
	// Get scrollback size, maximum number of lines in buffer.
	u32 getScrollback() const;
	// Get reference to i-th chat line.
	const ChatLine& getLine(u32 index) const;

	// Increase each chat line's age by dtime.
	void step(f32 dtime);
	// Delete oldest N chat lines.
	void deleteOldest(u32 count);
	// Delete lines older than maxAge.
	void deleteByAge(f32 maxAge);

	// Get number of rows, 0 if reformat has not been called yet.
	u32 getRows() const;
	// Get number of columns, 0 if reformat has not been called yet.
	u32 getColumns() const;
	// Update console size and reformat all formatted lines.
	void reformat(u32 rows, u32 cols);
	// Get formatted line for a given row (0 is top of screen).
	// Only valid after reformat has been called at least once
	const ChatFormattedLine& getFormattedLine(u32 row) const;
	// Scrolling in formatted buffer (relative)
	// positive rows == scroll up, negative rows == scroll down
	void scroll(s32 rows);
	// Scrolling in formatted buffer (absolute)
	void scrollAbsolute(s32 scroll);
	// Scroll to bottom of buffer (newest)
	void scrollBottom();
	// Scroll to top of buffer (oldest)
	void scrollTop();

	// Format a chat line for the given number of columns.
	// Appends the formatted lines to the destination array and
	// returns the number of formatted lines.
	u32 formatChatLine(const ChatLine& line, u32 cols,
			core::array<ChatFormattedLine>& destination) const;

protected:
	s32 getTopScrollPos() const;
	s32 getBottomScrollPos() const;

private:
	// Scrollback size
	u32 m_scrollback;
	// Array of unformatted chat lines
	core::array<ChatLine> m_unformatted;
	
	// Number of character rows in console
	u32 m_rows;
	// Number of character columns in console
	u32 m_cols;
	// Scroll position (console's top line index into m_formatted)
	s32 m_scroll;
	// Array of formatted lines
	core::array<ChatFormattedLine> m_formatted;
	// Empty formatted line, for error returns
	ChatFormattedLine m_empty_formatted_line;
};


class ChatBackend
{
public:
	ChatBackend();
	~ChatBackend();

	// Add chat message
	void addMessage(const std::wstring& name, const std::wstring& text);

	// Parse and add legacy preformatted chat message
	void addLegacyMessage(const std::wstring& line);

	// Get the console buffer
	ChatBuffer& getConsoleBuffer();

	// Get the recent messages buffer
	ChatBuffer& getRecentBuffer();

	// Concatenate all recent messages
	std::wstring getRecentChat();

	// Reformat all buffers
	void reformat(u32 rows, u32 cols);

	// Age recent messages
	void step(float dtime);

private:
	ChatBuffer m_console_buffer;
	ChatBuffer m_recent_buffer;
};

#endif

