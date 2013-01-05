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

#include <cmath>
#include <iomanip>
#include <iostream>

#include <glibmm/datetime.h>
#include <glibmm/miscutils.h>

#include "log_writer.hh"
#include "operation.hh"

Operation::Operation(Glib::RefPtr<Gio::File> input_directory, std::shared_ptr<LogReader> reader) :
	_input_directory(input_directory),
	_reader(reader)
{ }

void Operation::execute()
{
	this->_start_time = std::make_shared<const Glib::DateTime>(Glib::DateTime::create_now_utc());

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

CountOperation::CountOperation(Glib::RefPtr<Gio::File> input_directory, std::shared_ptr<LogReader> reader) :
	Operation(input_directory, reader)
{ }

void CountOperation::_handle_sessions(const std::vector<std::shared_ptr<Session>> & sessions)
{
	for (auto session : sessions)
	{
		for (auto event : session->events)
		{
			if (event->type == EventType::MESSAGE || event->type == EventType::ACTION)
			{
				Glib::ustring date = event->timestamp->format("%Y-%m-%d");

				if (date != this->_current_date)
				{
					if (this->_current_date != "")
						std::cout << this->_current_date << "\t" << this->_count << std::endl;

					this->_current_date = date;
					this->_count = 0;
				}

				this->_count++;
			}
		}
	}
}

void CountOperation::_cleanup()
{
	std::cout << this->_current_date << "\t" << this->_count << std::endl;
}

CoverageOperation::CoverageOperation(Glib::RefPtr<Gio::File> input_directory, std::shared_ptr<LogReader> reader) :
	Operation(input_directory, reader),
	_covered(0)
{ }

void CoverageOperation::_handle_sessions(const std::vector<std::shared_ptr<Session>> & sessions)
{
	for (auto session : sessions)
	{
		if (!this->_start || session->start->to_unix() < this->_start->to_unix())
			this->_start = session->start;

		if (!this->_stop || session->stop->to_unix() > this->_stop->to_unix())
			this->_stop = session->stop;

		if (this->_last_stop)
			this->_gaps.insert(std::make_pair(session->start->difference(*(this->_last_stop)), std::make_pair(this->_last_stop, session->start)));

		this->_last_stop = session->stop;
		this->_covered += session->stop->difference(*(session->start));
	}
}

Glib::ustring _format_timespan(Glib::TimeSpan timespan)
{
	timespan /= 1000000;

	int seconds = timespan % 60;
	int minutes = (timespan / 60) % 60;
	int hours = (timespan / 3600) % 24;
	int days = timespan / 86400;

	return Glib::ustring::compose("%1 days, %2 hours, %3 minutes and %4 seconds", days, hours, minutes, seconds);
}

void CoverageOperation::_cleanup()
{
	Glib::TimeSpan total = this->_stop->difference(*(this->_start));

	std::cout << "Log Coverage Report" << std::endl << std::endl;

	std::cout << "Logs span from " << this->_start->format("%Y-%m-%d %H:%M:%S") << " to " << this->_stop->format("%Y-%m-%d %H:%M:%S") << std::endl;
	std::cout << "  spanning a period of " << _format_timespan(total) << "." << std::endl << std::endl;

	std::cout << "Logged sessions cover a period of " << _format_timespan(this->_covered) << ", for a coverage rate of " << (this->_covered * 100.0 / total) << "%" << std::endl << std::endl;

	std::cout << "Longest Gaps in Coverage:" << std::endl;

	auto iter = this->_gaps.rbegin();

	for (int i = 0; i < 10; i++)
	{
		auto gap = *iter;

		std::cout << "  " << gap.second.first->format("%Y-%m-%d %H:%M:%S") << " to " << gap.second.second->format("%Y-%m-%d %H:%M:%S") << "\t" << _format_timespan(gap.first) << std::endl;

		iter++;
	}
}

FrequencyOperation::FrequencyOperation(Glib::RefPtr<Gio::File> input_directory, std::shared_ptr<LogReader> reader, double target) :
	Operation(input_directory, reader),
	_target(target)
{ }

std::deque<Glib::ustring> _tokenize(const Glib::ustring & message)
{
	return Glib::Regex::create("\\P{Ll}")->split(message.lowercase());
}

void FrequencyOperation::_handle_sessions(const std::vector<std::shared_ptr<Session>> & sessions)
{
	for (auto session : sessions)
	{
		if (!this->_start)
			this->_start = session->start;

		if (!this->_stop || session->stop->to_unix() > this->_stop->to_unix())
			this->_stop = session->stop;

		for (auto event : session->events)
		{
			if (event->type == EventType::MESSAGE || event->type == EventType::ACTION)
			{
				for (auto token : _tokenize(event->message))
				{
					if (token != "")
					{
						if (!this->_last[token])
							this->_last[token] = this->_start;

						int gap = event->timestamp->difference(*(this->_last[token])) / 1000000;

						if (gap > this->_max[token])
							this->_max[token] = gap;

						if (this->_min[token] == 0 || gap < this->_min[token])
							this->_min[token] = gap;

						this->_count[token]++;
						this->_last[token] = event->timestamp;
					}
				}
			}
		}
	}
}

void FrequencyOperation::_cleanup()
{
	double total_time = static_cast<double>(this->_stop->difference(*(this->_start))) / 1000000;

	std::set<std::pair<double, Glib::ustring>> scores;

	for (auto pair : this->_count)
	{
		Glib::ustring token = pair.first;
		double average = total_time / pair.second;

		double score = abs(average - this->_target);

		if (this->_max[token] < this->_target * 8.0)
			scores.insert(std::make_pair(score, token));
	}

	for (auto pair : scores)
	{
		std::cout << std::fixed << std::setprecision(5);
		std::cout << std::setw(30) << pair.second << "\t" << pair.first << "\t" << this->_count[pair.second] << "\t" << (total_time / this->_count[pair.second]) << "\t" << this->_min[pair.second] << "\t" << this->_max[pair.second] << std::endl;
	}
}
