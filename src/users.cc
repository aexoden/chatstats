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
#include <set>
#include <vector>

#include <giomm/datainputstream.h>
#include <glibmm/regex.h>

#include "users.hh"

UserStats::UserStats(const Glib::ustring & alias) :
	_alias(alias)
{ }

Glib::ustring UserStats::get_display_name()
{
	if (!this->_alias.empty())
		return this->_alias;

	int max_line_count = -1;
	Glib::ustring best_nick = "";

	for (auto nick : this->_nicks)
	{
		int line_count = this->_message_count[nick] + this->_action_count[nick];

		if (line_count > max_line_count)
		{
			max_line_count = line_count;
			best_nick = nick;
		}
	}

	return best_nick;
}

int UserStats::get_line_count()
{
	int line_count = 0;

	for (auto nick : this->_nicks)
		line_count += this->_message_count[nick] + this->_action_count[nick];

	return line_count;
}

void UserStats::add_nick(const Glib::ustring & nick)
{
	this->_nicks.insert(nick);
}

void UserStats::increment_message_count(const Glib::ustring & nick)
{
	this->add_nick(nick);
	this->_message_count[nick]++;
}

void UserStats::increment_action_count(const Glib::ustring & nick)
{
	this->add_nick(nick);
	this->_action_count[nick]++;
}

Users::Users(Glib::RefPtr<Gio::File> users_file)
{
	if (!users_file)
		return;

	auto users_stream = Gio::DataInputStream::create(users_file->read());
	std::string line;

	auto user = std::make_shared<UserStats>();

	while (users_stream->read_line(line))
	{
		if (!line.empty())
		{
			std::vector<Glib::ustring> tokens = Glib::Regex::create("[\t ]+")->split(line);

			if (tokens[0] == "USER")
			{
				if (line.length() > 5)
					user = std::make_shared<UserStats>(line.substr(5));
				else
					user = std::make_shared<UserStats>();

				this->_declared_users.insert(user);
			}
			else if (tokens[0] == "NICK")
			{
				for (size_t i = 1; i < tokens.size(); i++)
				{
					if (!tokens[i].empty())
						this->_users[tokens[i]] = user;
				}
			}
		}
	}
}

void Users::print_debug_info()
{
	std::set<std::pair<int, std::shared_ptr<UserStats>>> users;

	for (auto user : this->_declared_users)
		users.insert(std::make_pair(-(user->get_line_count()), user));

	std::cout << "Declared Users:" << std::endl;

	for (auto pair : users)
		std::cout << "  " << std::setiosflags(std::ios_base::left) << std::setw(30) << pair.second->get_display_name().raw() << " " << -pair.first << std::endl;

	users.clear();

	for (auto user : this->_undeclared_users)
		users.insert(std::make_pair(-(user->get_line_count()), user));

	std::cout << std::endl << "Unassigned Nicks:" << std::endl;

	for (auto pair : users)
		std::cout << "  " << std::setiosflags(std::ios_base::left) << std::setw(30) << pair.second->get_display_name().raw() << " " << -pair.first << std::endl;
}

std::unordered_set<std::shared_ptr<UserStats>> Users::get_users()
{
	std::unordered_set<std::shared_ptr<UserStats>> users;

	for (auto pair : this->_users)
		users.insert(pair.second);

	return users;
}

std::shared_ptr<UserStats> Users::get_user(const Glib::ustring & nick)
{
	if (this->_users.count(nick) == 0)
	{
		this->_users[nick] = std::make_shared<UserStats>();
		this->_undeclared_users.insert(this->_users[nick]);
	}

	return this->_users[nick];
}
