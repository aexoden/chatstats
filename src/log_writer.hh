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

#ifndef CHATSTATS_LOG_WRITER_HH
#define CHATSTATS_LOG_WRITER_HH

#include <memory>

#include <giomm/file.h>
#include <glibmm/ustring.h>

#include "session.hh"

class LogWriter
{
	public:
		const static Glib::ustring TIMESTAMP_FORMAT;

		void write(Glib::RefPtr<Gio::File> file, std::vector<std::shared_ptr<Session>> sessions);

	protected:
		Glib::ustring _format_session_start(std::shared_ptr<const Glib::DateTime> timestamp);
		Glib::ustring _format_session_stop(std::shared_ptr<const Glib::DateTime> timestamp);
		Glib::ustring _format_session_target(const Glib::ustring & target);

		Glib::ustring _format_action(const std::shared_ptr<const Event> & event);
		Glib::ustring _format_message(const std::shared_ptr<const Event> & event);
};

#endif // CHATSTATS_LOG_WRITER_HH

