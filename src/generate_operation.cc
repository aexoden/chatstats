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
#include "version.hh"

GenerateOperation::GenerateOperation(Glib::RefPtr<Gio::File> input_directory, std::shared_ptr<LogReader> reader, Glib::RefPtr<Gio::File> output_directory, Glib::RefPtr<Gio::File> users_file, bool debug_users) :
	Operation(input_directory, reader),
	_output_directory(output_directory),
	_users(users_file),
	_debug_users(debug_users)
{ }

void GenerateOperation::_cleanup()
{
	this->_output_css_default();

	Glib::RefPtr<Gio::File> output_file = Gio::File::create_for_path(Glib::build_filename(this->_output_directory->get_path(), "index.html"));
	Glib::RefPtr<Gio::DataOutputStream> output_stream = Gio::DataOutputStream::create(output_file->create_file());

	this->_output_html_header(output_stream);
	this->_output_section_overall_ranking(output_stream);
	this->_output_html_footer(output_stream);

	if (this->_debug_users)
		this->_users.print_debug_info();
}

void GenerateOperation::_handle_sessions(const std::vector<std::shared_ptr<Session>> & sessions)
{
	for (auto session : sessions)
	{
		if (this->_target.empty())
			this->_target = session->target;

		this->_userhost_cache.clear();

		for (auto event : session->events)
		{
			if (this->_userhost_cache.count(event->subject.nick) == 0)
				this->_userhost_cache[event->subject.nick] = "@";

			if (event->type == EventType::MESSAGE || event->type == EventType::ACTION)
			{
				std::string userhost = this->_userhost_cache[event->subject.nick];

				auto user = this->_users.get_user(event->subject.nick, userhost, event->timestamp);

				switch (event->type)
				{
					case EventType::MESSAGE:
						user->increment_message_count(event->subject.nick);
						break;

					case EventType::ACTION:
						user->increment_action_count(event->subject.nick);
						break;

					default:
						break;
				}
			}
			else if (event->type == EventType::JOIN)
			{
				this->_userhost_cache[event->subject.nick] = Glib::ustring::compose("%1@%2", event->subject.user, event->subject.host);
			}
			else if (event->type == EventType::NICK_CHANGE)
			{
				this->_userhost_cache[event->object.nick] = this->_userhost_cache[event->subject.nick];
			}
		}
	}
}

void _encode_char(Glib::ustring & string, char search, const Glib::ustring & replace)
{
	size_t pos = 0;

	while ((pos = string.find(search, pos)) != std::string::npos)
	{
		string.replace(pos, 1, replace);
		pos += replace.length();
	}
}

Glib::ustring _encode_html_chars(Glib::ustring string)
{
	_encode_char(string, '&', "&amp;");
	_encode_char(string, '<', "&lt;");
	_encode_char(string, '>', "&gt;");
	_encode_char(string, '"', "&quot;");

	return string;
}

void GenerateOperation::_output_css_default()
{
	Glib::RefPtr<Gio::File> css_directory = Gio::File::create_for_path(Glib::build_filename(this->_output_directory->get_path(), "css"));

	if (!css_directory->query_exists())
		css_directory->make_directory();

	Glib::RefPtr<Gio::File> output_file = Gio::File::create_for_path(Glib::build_filename(css_directory->get_path(), "default.css"));
	Glib::RefPtr<Gio::DataOutputStream> output_stream = Gio::DataOutputStream::create(output_file->create_file());

	output_stream->put_string(
R"EOF(/* chatstats default CSS */

body {
	margin: auto;
	width: 70em;
}
)EOF");
}

void GenerateOperation::_output_html_header(Glib::RefPtr<Gio::DataOutputStream> output_stream)
{
	output_stream->put_string("<!DOCTYPE html>\n");
	output_stream->put_string("<html>\n");
	output_stream->put_string("\t<head>\n");
	output_stream->put_string("\t\t<meta charset=\"utf-8\">\n");
	output_stream->put_string(Glib::ustring::compose("\t\t<title>%1 Statistics</title>\n", this->_target));
	output_stream->put_string("\t\t<link rel=\"stylesheet\" href=\"css/blueprint/screen.css\" media=\"screen, projection\">\n");
	output_stream->put_string("\t\t<link rel=\"stylesheet\" href=\"css/blueprint/print.css\" media=\"print\">\n");
	output_stream->put_string("\t\t<link rel=\"stylesheet\" href=\"css/default.css\">\n");
	output_stream->put_string("\t</head>\n");
	output_stream->put_string("\t<body>\n");
	output_stream->put_string("\t\t<div id=\"header\">\n");
	output_stream->put_string(Glib::ustring::compose("\t\t\t<h1>%1 Statistics</h1>\n", this->_target));
	output_stream->put_string("\t\t</div>\n");
	output_stream->put_string("\t\t<div id=\"content\">\n");
}

void GenerateOperation::_output_html_footer(Glib::RefPtr<Gio::DataOutputStream> output_stream)
{
	Glib::DateTime now = Glib::DateTime::create_now_utc();

	output_stream->put_string("\t\t</div>\n");
	output_stream->put_string("\t\t<div id=\"footer\">\n");
	output_stream->put_string(Glib::ustring::compose("\t\t\t<p>Generated by <a href=\"https://github.com/aexoden/chatstats\">chatstats</a> %1 on %2 at %3 UTC</p>\n", CHATSTATS_VERSION, now.format("%Y-%m-%d"), now.format("%H:%M:%S")));
	output_stream->put_string("\t\t</div>\n");
	output_stream->put_string("\t</body>\n");
	output_stream->put_string("</html>\n");
}

void GenerateOperation::_output_section_overall_ranking(Glib::RefPtr<Gio::DataOutputStream> output_stream)
{
	output_stream->put_string("\t\t\t<table>\n");
	output_stream->put_string("\t\t\t\t<tr><th>Rank</th><th>User</th><th>Lines</th><th>Nicknames</th></tr>\n");

	std::set<std::pair<int, std::shared_ptr<UserStats>>> sorted_user_line_counts;

	for (auto user : this->_users.get_users())
	{
		sorted_user_line_counts.insert(std::make_pair(-(user->get_line_count()), user));
	}

	unsigned int count = 0;
	unsigned int rank = 0;
	unsigned int last_score = 0;

	for (auto pair : sorted_user_line_counts)
	{
		unsigned int score = -pair.first;

		if (score < last_score || last_score == 0)
			rank = count + 1;

		if (rank > 1000)
			break;

		last_score = score;

		output_stream->put_string(Glib::ustring::compose("\t\t\t\t<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>\n", rank, _encode_html_chars(pair.second->get_display_name()), score, pair.second->get_nick_count()));
		count++;
	}

	output_stream->put_string("\t\t\t</table>\n");

	if (count < sorted_user_line_counts.size())
		output_stream->put_string(Glib::ustring::compose("\t\t\t<p>Plus %1 others who obviously weren't important enough for the table</p>\n", sorted_user_line_counts.size() - count));
}
