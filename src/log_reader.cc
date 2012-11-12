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

#include <list>

#include <glibmm/convert.h>
#include <glibmm/datetime.h>
#include <giomm/datainputstream.h>

#include "log_reader.hh"

std::vector<std::shared_ptr<Session>> LogReader::read(const Glib::RefPtr<Gio::File> & file)
{
	std::vector<std::shared_ptr<Session>> sessions;

	Glib::ustring target = "";

	this->_warnings.clear();
	this->_load_file_contents(file);
	this->_iter = this->_lines.begin();

	while (this->_iter != this->_lines.end())
	{
		auto session = std::make_shared<Session>();
		session->target = target;

		this->_parse_next_session(session);

		if (session->events.size() > 0)
		{
			if (target == "" && session->target != "")
				target = session->target;

			sessions.push_back(session);
		}
	}

	if (target == "")
		this->_warnings.insert(std::make_pair(0, "No session target in file"));

	return sessions;
}

const std::multimap<int, Glib::ustring> & LogReader::get_warnings() const
{
	return this->_warnings;
}

void LogReader::_load_file_contents(const Glib::RefPtr<Gio::File> & file)
{
	const std::string encodings[] = {"UTF-8", "CP1252", "ISO-8859-1"};

	this->_lines.clear();

	Glib::RefPtr<Gio::DataInputStream> file_stream = Gio::DataInputStream::create(file->read());
	std::string line;

	while (file_stream->read_line(line))
	{
		for (const std::string & encoding : encodings)
		{
			try
			{
				this->_lines.push_back(Glib::convert(line, "UTF-8", encoding));
				break;
			}
			catch (Glib::ConvertError e) {}
		}
	}
}

void LogReader::_parse_next_session(const std::shared_ptr<Session> & session)
{
	// TODO: Warn about multiple targets.

	for (; this->_iter != this->_lines.end(); this->_iter++)
	{
		auto line = *(this->_iter);

		if (!line.empty())
		{
			this->_warnings.insert(std::make_pair(this->_iter - this->_lines.begin() + 1, Glib::ustring::compose("Unrecognized line: %1", line)));
		}
	}

	if (session->events.size() > 0)
	{
		if (!session->start)
		{
			session->start = std::make_shared<Glib::DateTime>(session->events.front()->timestamp);
		}

		session->stop = std::make_shared<Glib::DateTime>(session->events.back()->timestamp);
	}
}
