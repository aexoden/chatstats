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
#include <string>

#include <glibmm/convert.h>
#include <glibmm/datetime.h>
#include <glibmm/timezone.h>
#include <giomm/datainputstream.h>

#include "log_reader.hh"

LogReader::LogReader()
{
	this->_regex_timestamp.push_back(Glib::Regex::create("^(?P<year>[0-9]{4})-(?P<month>[0-9]{2})-(?P<day>[0-9]{2}) (?P<hour>[0-9]{2}):(?P<minute>[0-9]{2}):(?P<second>[0-9]{2})(?P<offset>[0-9+-]{5})$"));

	this->_regex_event.push_back(std::make_pair(EventType::MESSAGE, Glib::Regex::create("^\\[(?P<timestamp>[^\\]]*)\\] <(?P<subject_nick>[^ ]*)> (?P<message>.*)$")));
}

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

std::shared_ptr<const Event> LogReader::_parse_line(const Glib::ustring & line)
{
	Glib::MatchInfo match_info;

	for (auto regex : this->_regex_event)
	{
		if (regex.second->match(line, match_info))
		{
			std::shared_ptr<const Glib::DateTime> timestamp = this->_parse_timestamp(match_info.fetch_named("timestamp"));

			if (!timestamp)
				return nullptr;

			User subject(match_info.fetch_named("subject_nick"), match_info.fetch_named("subject_user"), match_info.fetch_named("subject_host"));
			User object(match_info.fetch_named("object_nick"), match_info.fetch_named("object_user"), match_info.fetch_named("object_host"));

			return std::make_shared<const Event>(regex.first, *timestamp, subject, object, match_info.fetch_named("message"));
		}
	}

	return nullptr;
}

std::shared_ptr<const Glib::DateTime> LogReader::_parse_timestamp(const Glib::ustring & data)
{
	Glib::MatchInfo match_info;

	for (auto regex : this->_regex_timestamp)
	{
		if (regex->match(data, match_info))
		{
			Glib::ustring offset = match_info.fetch_named("offset");
			Glib::TimeZone timezone = offset.empty() ? Glib::TimeZone::create_local() : Glib::TimeZone::create(offset);

			Glib::DateTime previous_timestamp = Glib::DateTime::create(timezone, 1970, 1, 1, 0, 0, 0);

			if (this->_current_timestamp)
			{
				previous_timestamp = this->_current_timestamp->to_timezone(timezone);
			}
			else if (match_info.fetch_named("year").empty() || match_info.fetch_named("month").empty() || match_info.fetch_named("day").empty())
			{
				this->_add_warning("Partial timestamp used before complete timestamp");
				return nullptr;
			}

			int year = this->_parse_timestamp_int(match_info.fetch_named("year"), previous_timestamp.get_year());
			int month = this->_parse_timestamp_int(match_info.fetch_named("month"), previous_timestamp.get_month());
			int day = this->_parse_timestamp_int(match_info.fetch_named("day"), previous_timestamp.get_day_of_month());
			int hour = this->_parse_timestamp_int(match_info.fetch_named("hour"), previous_timestamp.get_hour());
			int minute = this->_parse_timestamp_int(match_info.fetch_named("minute"), previous_timestamp.get_minute());
			int second = this->_parse_timestamp_int(match_info.fetch_named("second"), previous_timestamp.get_second());

			auto timestamp = std::make_shared<const Glib::DateTime>(Glib::DateTime::create(timezone, year, month, day, hour, minute, second).to_utc());

			if (timestamp->to_unix() < previous_timestamp.to_unix())
				this->_add_warning("Timestamp is earlier than the previous timestamp");

			this->_current_timestamp = timestamp;

			return timestamp;
		}
	}

	this->_add_warning("Invalid or missing timestamp");
	return nullptr;
}

int LogReader::_parse_timestamp_int(const Glib::ustring & data, int default_value)
{
	if (data.empty())
		return default_value;
	else
		return stol(data.raw());
}

void LogReader::_add_warning(const Glib::ustring & warning)
{
	this->_warnings.insert(std::make_pair(this->_iter - this->_lines.begin() + 1, warning));
}

void LogReader::_parse_next_session(const std::shared_ptr<Session> & session)
{
	// TODO: Warn about multiple targets.
	for (; this->_iter != this->_lines.end(); this->_iter++)
	{
		auto line = *(this->_iter);

		if (!line.empty())
		{
			auto event = this->_parse_line(line);

			if (event)
			{
				session->events.push_back(event);
			}
			else
			{
				this->_add_warning(Glib::ustring::compose("Unrecognized line: %1", line));
			}
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
