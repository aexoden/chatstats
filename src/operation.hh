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

#ifndef CHATSTATS_OPERATION_HH
#define CHATSTATS_OPERATION_HH

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <giomm/file.h>
#include <glibmm/ustring.h>

#include "log_reader.hh"
#include "session.hh"

class Operation
{
	public:
		Operation(Glib::RefPtr<Gio::File> input_directory, std::shared_ptr<LogReader> reader);

		void execute();

	protected:
		Glib::RefPtr<Gio::File> _input_directory;
		std::shared_ptr<LogReader> _reader;

		std::set<std::string> _get_input_filenames();

		virtual void _cleanup() = 0;
		virtual void _handle_sessions(const std::vector<std::shared_ptr<Session>> & sessions) = 0;
};

class ConvertOperation : public Operation
{
	public:
		ConvertOperation(Glib::RefPtr<Gio::File> input_directory, std::shared_ptr<LogReader> reader, Glib::RefPtr<Gio::File> output_directory);

	protected:
		virtual void _cleanup();
		virtual void _handle_sessions(const std::vector<std::shared_ptr<Session>> & sessions);

	private:
		void _write_sessions();

		Glib::RefPtr<Gio::File> _output_directory;

		std::vector<std::shared_ptr<Session>> _sessions;
};

class CountOperation : public Operation
{
	public:
		CountOperation(Glib::RefPtr<Gio::File> input_directory, std::shared_ptr<LogReader> reader);

	protected:
		virtual void _cleanup();
		virtual void _handle_sessions(const std::vector<std::shared_ptr<Session>> & sessions);

	private:
		Glib::ustring _current_date;
		int _count;
};

#endif // CHATSTATS_OPERATION_HH

