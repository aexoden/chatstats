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

#ifndef CHATSTATS_GENERATE_OPERATION_HH
#define CHATSTATS_GENERATE_OPERATION_HH

#include <giomm/dataoutputstream.h>

#include "operation.hh"
#include "users.hh"

class GenerateOperation : public Operation
{
	public:
		GenerateOperation(Glib::RefPtr<Gio::File> input_directory, std::shared_ptr<LogReader> reader, Glib::RefPtr<Gio::File> output_directory, Glib::RefPtr<Gio::File> users_file, bool debug_users);

	protected:
		virtual void _cleanup();
		virtual void _handle_sessions(const std::vector<std::shared_ptr<Session>> & sessions);

	private:
		void _output_css_default();

		void _output_html_header(Glib::RefPtr<Gio::DataOutputStream> output_stream);
		void _output_html_footer(Glib::RefPtr<Gio::DataOutputStream> output_stream);

		void _output_section_overall_ranking(Glib::RefPtr<Gio::DataOutputStream> output_stream);

		Glib::RefPtr<Gio::File> _output_directory;
		Glib::ustring _target;

		Users _users;

		bool _debug_users;

		std::unordered_map<std::string, std::string> _userhost_cache;
};

#endif // CHATSTATS_GENERATE_OPERATION_HH

