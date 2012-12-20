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

#include <glibmm/miscutils.h>

#include "generate_operation.hh"

GenerateOperation::GenerateOperation(Glib::RefPtr<Gio::File> input_directory, std::shared_ptr<LogReader> reader, Glib::RefPtr<Gio::File> output_directory) :
	Operation(input_directory, reader),
	_output_directory(output_directory)
{ }

void GenerateOperation::_cleanup()
{
	Glib::RefPtr<Gio::File> output_file = Gio::File::create_for_path(Glib::build_filename(this->_output_directory->get_path(), "index.html"));
	Glib::RefPtr<Gio::DataOutputStream> output_stream = Gio::DataOutputStream::create(output_file->create_file());

	this->_output_html_header(output_stream);

	output_stream->put_string("<table>\n");
	output_stream->put_string("<tr><th>Rank</th><th>Nickname</th><th>Messages</th></tr>\n");

	std::set<std::pair<int, Glib::ustring>> nick_message_counts;

	for (auto pair : this->_nick_message_counts)
		nick_message_counts.insert(std::make_pair(-pair.second, pair.first));

	unsigned int count = 0;
	unsigned int rank = 0;
	unsigned int last_score = 0;

	for (auto pair : nick_message_counts)
	{
		unsigned int score = -pair.first;

		if (score < last_score || last_score == 0)
			rank = count + 1;

		if (rank > 1000)
			break;

		last_score = score;

		output_stream->put_string(Glib::ustring::compose("<tr><td>%1</td><td>%2</td><td>%3</td></tr>\n", rank, pair.second, score));
		count++;
	}

	output_stream->put_string("</table>\n");

	if (count < nick_message_counts.size())
		output_stream->put_string(Glib::ustring::compose("<p>Plus %1 others who obviously weren't important enough for the table</p>\n", nick_message_counts.size() - count));

	this->_output_html_footer(output_stream);
}

void GenerateOperation::_output_html_header(Glib::RefPtr<Gio::DataOutputStream> output_stream)
{
	output_stream->put_string(Glib::ustring::compose("<!DOCTYPE html>\n<html>\n<head>\n<title>%1 Statistics</title>\n", this->_target));
	output_stream->put_string("</head>\n<body>\n");
}

void GenerateOperation::_output_html_footer(Glib::RefPtr<Gio::DataOutputStream> output_stream)
{
	output_stream->put_string("</body>\n</html>\n");
}

void GenerateOperation::_handle_sessions(const std::vector<std::shared_ptr<Session>> & sessions)
{
	for (auto session : sessions)
	{
		if (this->_target.empty())
			this->_target = session->target;

		for (auto event : session->events)
		{
			if (event->type == EventType::MESSAGE)
				this->_nick_message_counts[event->subject.nick]++;
		}
	}
}
