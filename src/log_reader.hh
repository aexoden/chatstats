/*
 * Copyright (c) 2012 Jason Lynch <jason@calindora.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef CHATSTATS_LOG_READER_HH
#define CHATSTATS_LOG_READER_HH

#include <map>
#include <memory>
#include <vector>

#include <glibmm/regex.h>
#include <glibmm/ustring.h>
#include <giomm/file.h>

#include "event.hh"
#include "session.hh"

class LogReader
{
	public:
		LogReader();

		std::vector<std::shared_ptr<Session>> read(const Glib::RefPtr<Gio::File> & file);

		const std::multimap<int, Glib::ustring> & get_warnings() const;

	protected:
		std::vector<std::pair<EventType, Glib::RefPtr<Glib::Regex>>> _regex_event;
		std::vector<Glib::RefPtr<Glib::Regex>> _regex_timestamp;

	private:
		void _load_file_contents(const Glib::RefPtr<Gio::File> & file);

		std::shared_ptr<const Event> _parse_line(const Glib::ustring & line);
		std::shared_ptr<const Glib::DateTime> _parse_timestamp(const Glib::ustring & data);
		int _parse_timestamp_int(const Glib::ustring & data, int default_value);

		void _add_warning(const Glib::ustring & warning);

		void _add_regex_event(EventType type, const Glib::ustring & regex_string);

		void _parse_next_session(const std::shared_ptr<Session> & session);

		std::vector<Glib::ustring> _lines;
		std::vector<Glib::ustring>::const_iterator _iter;

		std::shared_ptr<const Glib::DateTime> _current_timestamp;

		std::multimap<int, Glib::ustring> _warnings;
};

#endif // CHATSTATS_LOG_READER_HH

