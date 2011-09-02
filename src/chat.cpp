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

#include "chat.h"
#include "debug.h"
#include <cassert>
#include <cctype>

ChatBuffer::ChatBuffer(u32 scrollback):
	m_scrollback(scrollback),
	m_unformatted(),
	m_rows(0),
	m_cols(0),
	m_scroll(0),
	m_formatted(),
	m_empty_formatted_line()
{
	if (m_scrollback == 0)
		m_scrollback = 1;
	m_empty_formatted_line.first = true;
}

ChatBuffer::~ChatBuffer()
{
}

void ChatBuffer::addLine(const std::wstring& name, const std::wstring& text)
{
	ChatLine line(name, text);
	m_unformatted.push_back(line);

	if (m_rows > 0)
	{
		// m_formatted is valid and must be kept valid
		bool scrolled_at_bottom = (m_scroll == getBottomScrollPos());
		u32 num_added = formatChatLine(line, m_cols, m_formatted);
		if (scrolled_at_bottom)
			m_scroll += num_added;
	}

	// Limit number of lines by m_scrollback
	if (m_unformatted.size() > m_scrollback)
	{
		deleteOldest(m_unformatted.size() - m_scrollback);
	}
}

u32 ChatBuffer::getLineCount() const
{
	return m_unformatted.size();
}

u32 ChatBuffer::getScrollback() const
{
	return m_scrollback;
}

const ChatLine& ChatBuffer::getLine(u32 index) const
{
	assert(index < getLineCount());
	return m_unformatted[index];
}

void ChatBuffer::step(f32 dtime)
{
	for (u32 i = 0; i < m_unformatted.size(); ++i)
	{
		m_unformatted[i].age += dtime;
	}
}

void ChatBuffer::deleteOldest(u32 count)
{
	u32 del_unformatted = 0;
	u32 del_formatted = 0;

	while (count > 0 && del_unformatted < m_unformatted.size())
	{
		++del_unformatted;

		// keep m_formatted in sync
		if (del_formatted < m_formatted.size())
		{
			assert(m_formatted[del_formatted].first);
			++del_formatted;
			while (del_formatted < m_formatted.size() &&
					!m_formatted[del_formatted].first)
				++del_formatted;
		}

		--count;
	}

	m_unformatted.erase(0, del_unformatted);
	m_formatted.erase(0, del_formatted);
}

void ChatBuffer::deleteByAge(f32 maxAge)
{
	u32 count = 0;
	while (count < m_unformatted.size() && m_unformatted[count].age > maxAge)
		++count;
	deleteOldest(count);
}

u32 ChatBuffer::getRows() const
{
	return m_rows;
}

u32 ChatBuffer::getColumns() const
{
	return m_cols;
}

void ChatBuffer::reformat(u32 rows, u32 cols)
{
	if (rows == 0 || cols == 0)
	{
		// Clear formatted buffer
		m_rows = 0;
		m_cols = 0;
		m_scroll = 0;
		m_formatted.clear();
	}
	else
	{
		// TODO: Avoid reformatting ALL lines (even inivisble ones)
		// each time the console size changes.
		// TODO
	}
}

const ChatFormattedLine& ChatBuffer::getFormattedLine(u32 row) const
{
	s32 index = m_scroll + (s32) row;
	if (index >= 0 && index < (s32) m_formatted.size())
		return m_formatted[index];
	else
		return m_empty_formatted_line;
}

void ChatBuffer::scroll(s32 rows)
{
	scrollAbsolute(m_scroll + rows);
}

void ChatBuffer::scrollAbsolute(s32 scroll)
{
	s32 top = getTopScrollPos();
	s32 bottom = getBottomScrollPos();

	m_scroll = scroll;
	if (m_scroll < top)
		m_scroll = top;
	if (m_scroll > bottom)
		m_scroll = bottom;
}

void ChatBuffer::scrollBottom()
{
	m_scroll = getBottomScrollPos();
}

void ChatBuffer::scrollTop()
{
	m_scroll = getTopScrollPos();
}

u32 ChatBuffer::formatChatLine(const ChatLine& line, u32 cols,
		core::array<ChatFormattedLine>& destination) const
{
	u32 num_added = 0;
	core::array<ChatFormattedFragment> next_frags;
	ChatFormattedLine next_line;
	ChatFormattedFragment temp_frag;
	u32 out_column = 0;
	u32 in_pos = 0;
	u32 hanging_indentation = 0;

	// Format the sender name
	if (!line.name.empty())
	{
		temp_frag.text = L"<";
		temp_frag.column = 0;
		temp_frag.bold = 0;
		next_frags.push_back(temp_frag);
		temp_frag.text = line.name;
		temp_frag.column = 0;
		temp_frag.bold = 1;
		next_frags.push_back(temp_frag);
		temp_frag.text = L"> ";
		temp_frag.column = 0;
		temp_frag.bold = 0;
		next_frags.push_back(temp_frag);
	}

	// Choose an indentation level
	if (line.name.empty())
	{
		// Server messages
		hanging_indentation = 0;
	}
	else if (line.name.size() + 3 <= cols/2)
	{
		// Names shorter than about half the console width
		hanging_indentation = line.name.size() + 3;
	}
	else
	{
		// Very long names
		hanging_indentation = 2;
	}

	bool text_processing = false;

	// Produce fragments and layout them into lines
	while (!next_frags.empty() || in_pos < line.text.size())
	{
		// Layout fragments into lines
		while (!next_frags.empty())
		{
			ChatFormattedFragment& frag = next_frags[0];
			if (frag.text.size() <= cols - out_column)
			{
				// Fragment fits into current line
				frag.column = out_column;
				next_line.fragments.push_back(frag);
				out_column += frag.text.size();
				next_frags.erase(0, 1);
			}
			else
			{
				// Fragment does not fit into current line
				// So split it up
				temp_frag.text = frag.text.substr(0, cols - out_column);
				temp_frag.column = out_column;
				temp_frag.bold = frag.bold;
				next_line.fragments.push_back(temp_frag);
				frag.text = frag.text.substr(cols - out_column);
				out_column = cols;
			}
			if (out_column == cols || text_processing)
			{
				// End the current line
				destination.push_back(next_line);
				num_added++;
				next_line.fragments.clear();
				next_line.first = false;

				out_column = text_processing ? hanging_indentation : 0;
			}
		}

		// Produce fragment
		if (in_pos < line.text.size())
		{
			u32 remaining_in_input = line.text.size() - in_pos;
			u32 remaining_in_output = cols - out_column;

			// Determine a fragment length <= the minimum of
			// remaining_in_{in,out}put. Try to end the fragment
			// on a word boundary.
			u32 frag_length = 1, space_pos = 0;
			while (frag_length < remaining_in_input &&
					frag_length < remaining_in_output)
			{
				if (isspace(line.text[in_pos + frag_length]))
					space_pos = frag_length;
				++frag_length;
			}
			if (space_pos != 0 && frag_length < remaining_in_input)
				frag_length = space_pos + 1;

			temp_frag.text = line.text.substr(in_pos, frag_length);
			temp_frag.column = 0;
			temp_frag.bold = false;
			next_frags.push_back(temp_frag);
			in_pos += frag_length;
			text_processing = true;
		}
	}

	// End the last line
	if (num_added == 0 || !next_line.fragments.empty())
	{
		destination.push_back(next_line);
		num_added++;
	}

	return num_added;
}

s32 ChatBuffer::getTopScrollPos() const
{
	s32 formatted_count = (s32) m_formatted.size();
	s32 rows = (s32) m_rows;
	if (rows == 0)
		return 0;
	else if (formatted_count <= rows)
		return 0;
	else
		return formatted_count - rows;
}

s32 ChatBuffer::getBottomScrollPos() const
{
	s32 formatted_count = (s32) m_formatted.size();
	s32 rows = (s32) m_rows;
	if (rows == 0)
		return 0;
	else
		return formatted_count - rows;
}
