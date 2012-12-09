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

#include <iomanip>
#include <iostream>

#include <glibmm/miscutils.h>

#include "log_writer.hh"
#include "operation.hh"

Operation::Operation(Glib::RefPtr<Gio::File> input_directory, std::shared_ptr<LogReader> reader) :
	_input_directory(input_directory),
	_reader(reader)
{ }

void Operation::execute()
{
	for (const std::string & filename : this->_get_input_filenames())
	{
		auto sessions = this->_reader->read(Gio::File::create_for_path(filename));

		for (auto warning : this->_reader->get_warnings())
		{
			auto filename_string = Glib::ustring::format(std::setw(30), Glib::ustring::compose("%1:%2", Glib::path_get_basename(filename), warning.first));
			std::cerr << Glib::ustring::compose("%1: %2", filename_string, warning.second) << std::endl;
		}

		this->_handle_sessions(sessions);
	}

	this->_cleanup();
}

std::set<std::string> Operation::_get_input_filenames()
{
	std::set<std::string> filenames;

	Glib::RefPtr<Gio::FileEnumerator> file_enumerator = this->_input_directory->enumerate_children("standard::name");

	while (Glib::RefPtr<Gio::FileInfo> file_info = file_enumerator->next_file())
	{
		filenames.insert(Glib::build_filename(this->_input_directory->get_path(), file_info->get_name()));
	}

	return filenames;
}

ConvertOperation::ConvertOperation(Glib::RefPtr<Gio::File> input_directory, std::shared_ptr<LogReader> reader, Glib::RefPtr<Gio::File> output_directory) :
	Operation(input_directory, reader),
	_output_directory(output_directory)
{ }

void ConvertOperation::_cleanup()
{
	this->_write_sessions();
}

void ConvertOperation::_handle_sessions(const std::vector<std::shared_ptr<Session>> & sessions)
{
	for (auto session : sessions)
	{
		while (session)
		{
			if (this->_sessions.size() > 0)
			{
				auto first_session = this->_sessions.front();

				if (session->start->get_year() != first_session->start->get_year() || session->start->get_month() != first_session->start->get_month())
					this->_write_sessions();
			}

			this->_sessions.push_back(session);

			int year = session->start->get_year();
			int month = session->start->get_month() + 1;

			if (month == 13)
			{
				year++;
				month = 1;
			}

			session = session->split(Glib::DateTime::create_utc(year, month, 1, 0, 0, 0));
		}
	}
}

void ConvertOperation::_write_sessions()
{
	LogWriter log_writer;

	if (!this->_sessions.empty())
	{
		std::shared_ptr<Session> first_session = this->_sessions.front();

		Glib::ustring output_filename(Glib::ustring::compose("%1-%2.log", first_session->target, first_session->start->format("%Y%m")));
		Glib::RefPtr<Gio::File> output_file = Gio::File::create_for_path(Glib::build_filename(this->_output_directory->get_path(), output_filename));

		log_writer.write(output_file, this->_sessions);

		this->_sessions.clear();
	}
}
