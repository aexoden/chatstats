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
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <giomm/file.h>
#include <glibmm/ustring.h>

class UserStats
{
	public:
		UserStats(const Glib::ustring & alias = "");

		Glib::ustring get_display_name();

		int get_line_count();

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
		Users(Glib::RefPtr<Gio::File> users_file);

		std::set<std::shared_ptr<UserStats>> get_users();

		std::shared_ptr<UserStats> get_user(const Glib::ustring & nick);

	private:
		std::map<std::string, std::shared_ptr<UserStats>> _users;
};

#endif // CHATSTATS_USERS_HH

