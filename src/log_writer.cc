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

#include <iostream>

#include <giomm/dataoutputstream.h>

#include "log_writer.hh"

const Glib::ustring LogWriter::TIMESTAMP_FORMAT = "%Y-%m-%d %H:%M:%S+0000";

void LogWriter::write(Glib::RefPtr<Gio::File> file, std::vector<std::shared_ptr<Session>> sessions)
{
	Glib::RefPtr<Gio::DataOutputStream> file_stream = Gio::DataOutputStream::create(file->create_file());

	bool first_session = true;

	for (auto session : sessions)
	{
		if (!first_session)
			file_stream->put_string("\n");

		file_stream->put_string(Glib::ustring::compose("%1\n", this->_format_session_start(session->start)).raw());

		if (!session->target.empty())
			file_stream->put_string(Glib::ustring::compose("%1\n", this->_format_session_target(session->target)).raw());

		for (auto event : session->events)
		{
			Glib::ustring message;

			switch (event->type)
			{
				case EventType::ACTION:
					message = this->_format_action(event);
					break;
				case EventType::CTCP:
					message = this->_format_ctcp(event);
					break;
				case EventType::JOIN:
					message = this->_format_join(event);
					break;
				case EventType::KICK:
					message = this->_format_kick(event);
					break;
				case EventType::MESSAGE:
					message = this->_format_message(event);
					break;
				case EventType::MODE_CHANGE:
					message = this->_format_mode_change(event);
					break;
				case EventType::NICK_CHANGE:
					message = this->_format_nick_change(event);
					break;
				case EventType::NOTICE:
					message = this->_format_notice(event);
					break;
				case EventType::PART:
					message = this->_format_part(event);
					break;
				case EventType::QUIT:
					message = this->_format_quit(event);
					break;
				case EventType::TOPIC_CHANGE:
					message = this->_format_topic_change(event);
					break;
				default:
					continue;
			}

			file_stream->put_string(Glib::ustring::compose("%1\n", message).raw());
		}

		file_stream->put_string(Glib::ustring::compose("%1\n", this->_format_session_stop(session->stop)).raw());

		first_session = false;
	}
}

Glib::ustring LogWriter::_format_session_start(std::shared_ptr<const Glib::DateTime> timestamp)
{
	return Glib::ustring::compose("Session Start: %1", timestamp->format(LogWriter::TIMESTAMP_FORMAT));
}

Glib::ustring LogWriter::_format_session_stop(std::shared_ptr<const Glib::DateTime> timestamp)
{
	return Glib::ustring::compose("Session Stop: %1", timestamp->format(LogWriter::TIMESTAMP_FORMAT));
}

Glib::ustring LogWriter::_format_session_target(const Glib::ustring & target)
{
	return Glib::ustring::compose("Session Target: %1", target);
}

Glib::ustring LogWriter::_format_action(const std::shared_ptr<const Event> & event)
{
	return Glib::ustring::compose("[%1] * %2 %3", event->timestamp->format(LogWriter::TIMESTAMP_FORMAT), event->subject.to_string(), event->message);
}

Glib::ustring LogWriter::_format_ctcp(const std::shared_ptr<const Event> & event)
{
	return Glib::ustring::compose("[%1] [%2] %3", event->timestamp->format(LogWriter::TIMESTAMP_FORMAT), event->subject.to_string(), event->message);
}

Glib::ustring LogWriter::_format_join(const std::shared_ptr<const Event> & event)
{
	return Glib::ustring::compose("[%1] *** %2 joins", event->timestamp->format(LogWriter::TIMESTAMP_FORMAT), event->subject.to_string());
}

Glib::ustring LogWriter::_format_kick(const std::shared_ptr<const Event> & event)
{
	return Glib::ustring::compose("[%1] *** %2 kicks %3 (%4)", event->timestamp->format(LogWriter::TIMESTAMP_FORMAT), event->subject.to_string(), event->object.to_string(), event->message);
}

Glib::ustring LogWriter::_format_message(const std::shared_ptr<const Event> & event)
{
	return Glib::ustring::compose("[%1] <%2> %3", event->timestamp->format(LogWriter::TIMESTAMP_FORMAT), event->subject.to_string(), event->message);
}

Glib::ustring LogWriter::_format_mode_change(const std::shared_ptr<const Event> & event)
{
	return Glib::ustring::compose("[%1] *** %2 sets mode: %3", event->timestamp->format(LogWriter::TIMESTAMP_FORMAT), event->subject.to_string(), event->message);
}

Glib::ustring LogWriter::_format_nick_change(const std::shared_ptr<const Event> & event)
{
	return Glib::ustring::compose("[%1] *** %2 is now known as %3", event->timestamp->format(LogWriter::TIMESTAMP_FORMAT), event->subject.to_string(), event->object.to_string());
}

Glib::ustring LogWriter::_format_notice(const std::shared_ptr<const Event> & event)
{
	return Glib::ustring::compose("[%1] -%2- %3", event->timestamp->format(LogWriter::TIMESTAMP_FORMAT), event->subject.to_string(), event->message);
}

Glib::ustring LogWriter::_format_part(const std::shared_ptr<const Event> & event)
{
	if (event->message.empty())
		return Glib::ustring::compose("[%1] *** %2 parts", event->timestamp->format(LogWriter::TIMESTAMP_FORMAT), event->subject.to_string());
	else
		return Glib::ustring::compose("[%1] *** %2 parts (%3)", event->timestamp->format(LogWriter::TIMESTAMP_FORMAT), event->subject.to_string(), event->message);
}

Glib::ustring LogWriter::_format_quit(const std::shared_ptr<const Event> & event)
{
	if (event->message.empty())
		return Glib::ustring::compose("[%1] *** %2 quits", event->timestamp->format(LogWriter::TIMESTAMP_FORMAT), event->subject.to_string());
	else
		return Glib::ustring::compose("[%1] *** %2 quits (%3)", event->timestamp->format(LogWriter::TIMESTAMP_FORMAT), event->subject.to_string(), event->message);
}

Glib::ustring LogWriter::_format_topic_change(const std::shared_ptr<const Event> & event)
{
	return Glib::ustring::compose("[%1] *** %2 changes topic to '%3'", event->timestamp->format(LogWriter::TIMESTAMP_FORMAT), event->subject.to_string(), event->message);
}
