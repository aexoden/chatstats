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

#ifndef CHATSTATS_USERS_HH
#define CHATSTATS_USERS_HH

#include <deque>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <giomm/file.h>
#include <glibmm/regex.h>
#include <glibmm/ustring.h>

#include "time_range.hh"

class UserMatch
{
	public:
		UserMatch(const Glib::ustring & string, Glib::RefPtr<Glib::Regex> regex, std::shared_ptr<TimeRange> time_range);

		Glib::ustring string;
		Glib::RefPtr<Glib::Regex> regex;
		std::shared_ptr<TimeRange> time_range;
};

class UserStats
{
	public:
		UserStats(const Glib::ustring & alias = "");

		Glib::ustring get_display_name();

		int get_line_count();
		int get_nick_count();

		std::set<std::pair<int, Glib::ustring>> get_nicks();

		void add_nick(const Glib::ustring & nick);

		void increment_message_count(const Glib::ustring & nick);
		void increment_action_count(const Glib::ustring & nick);

	private:
		Glib::ustring _alias;

		std::unordered_set<std::string> _nicks;

		std::unordered_map<std::string, int> _message_count;
		std::unordered_map<std::string, int> _action_count;
};

class Users
{
	public:
		Users(Glib::RefPtr<Gio::File> users_file, bool separate_userhosts);

		void print_debug_info();

		std::unordered_set<std::shared_ptr<UserStats>> get_users();
		std::shared_ptr<UserStats> get_user(const Glib::ustring & nick, const Glib::ustring & userhost, std::shared_ptr<const Glib::DateTime> timestamp);

	private:
		void _insert_user_match(std::list<std::shared_ptr<UserMatch>> & user_matches, std::shared_ptr<UserMatch> user_match);

		bool _separate_userhosts;

		std::unordered_map<std::string, std::shared_ptr<UserStats>> _users;

		std::list<std::shared_ptr<UserMatch>> _time_restricted_nicks;
		std::list<std::shared_ptr<UserMatch>> _unrestricted_nicks;

		std::unordered_map<std::string, std::pair<std::shared_ptr<TimeRange>, std::string>> _user_cache;

		std::unordered_set<std::shared_ptr<UserStats>> _declared_users;
		std::unordered_set<std::shared_ptr<UserStats>> _undeclared_users;
};

#endif // CHATSTATS_USERS_HH
