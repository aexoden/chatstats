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

#include "SQLiteC++.h"

#include "operation.hh"
#include "user_specification.hh"

class GenerateOperation : public Operation
{
	public:
		GenerateOperation(const Glib::RefPtr<Gio::File> & input_directory, const std::shared_ptr<LogReader> & reader, const Glib::RefPtr<Gio::File> & output_directory, const Glib::RefPtr<Gio::File> & users_file, const bool debug, const bool separate_userhosts);

	protected:
		virtual void _cleanup();
		virtual void _handle_sessions(const std::vector<std::shared_ptr<Session>> & sessions);

	private:
		void _initialize_database();
		void _initialize_database_tables();
		void _initialize_database_queries();

		void _apply_users_file();
		std::vector<std::shared_ptr<UserSpecification>> _parse_users_file() const;

		Glib::RefPtr<Gio::File> _get_user_directory(const Glib::ustring & alias) const;

		void _assign_aliases();
		void _create_undeclared_users();

		void _output_css_default() const;

		void _output_html_index();
		void _output_html_header(const Glib::RefPtr<Gio::DataOutputStream> & output_stream, const Glib::ustring & title, const Glib::ustring & media_prefix = "") const;
		void _output_html_footer(const Glib::RefPtr<Gio::DataOutputStream> & output_stream) const;

		void _output_html_user_index(const Glib::RefPtr<Gio::File> & user_file, const int user_id);

		void _output_html_section_overall_ranking(const Glib::RefPtr<Gio::DataOutputStream> & output_stream);

		int _get_nickuserhost_id(const User & user);

		void _print_debug_info();

		const Glib::RefPtr<const Gio::File> _output_directory;
		const Glib::RefPtr<Gio::File> _users_directory;

		const Glib::RefPtr<Gio::File> _users_file;

		Glib::ustring _target;

		SQLite::Database _database;

		std::shared_ptr<SQLite::Statement> _nickuserhost_insert_query;

		std::unordered_map<std::string, std::string> _userhosts;
		std::unordered_map<std::string, int> _nickuserhost_ids;

		std::shared_ptr<const Glib::DateTime> _last_session_stop;

		const bool _separate_userhosts;
		const bool _debug;
};

#endif // CHATSTATS_GENERATE_OPERATION_HH

