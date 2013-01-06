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

int UserStats::get_nick_count()
{
	return this->_nicks.size();
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

void _str_replace(Glib::ustring & string, const Glib::ustring & search, const Glib::ustring & replace)
{
	size_t pos = 0;

	while ((pos = string.find(search, pos)) != std::string::npos)
	{
		string.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}

Users::Users(Glib::RefPtr<Gio::File> users_file, bool separate_userhosts) :
	_separate_userhosts(separate_userhosts)
{
	if (!users_file)
		return;

	auto users_stream = Gio::DataInputStream::create(users_file->read());
	std::string line;

	auto user = std::make_shared<UserStats>();

	auto nick_regex = Glib::Regex::create("(?P<nick>[^!#]*)(!(?P<userhost>[^@]*@[^#]*))?(#(?P<start_date>[0-9]{4}-[0-9]{2}-[0-9]{2})?/(?P<end_date>[0-9]{4}-[0-9]{2}-[0-9]{2})?\\+(?P<start_time>[0-9]{2}:[0-9]{2}:[0-9]{2})?/(?P<end_time>[0-9]{2}:[0-9]{2}:[0-9]{2})?)?");

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
					{
						Glib::MatchInfo match_info;

						if (nick_regex->match(tokens[i], match_info))
						{
							auto userhost = match_info.fetch_named("userhost");

							if (userhost.empty())
								userhost = "*@*";

							auto regex = Glib::ustring::compose("^%1$", Glib::Regex::escape_string(Glib::ustring::compose("%1!%2", match_info.fetch_named("nick"), userhost)));
							_str_replace(regex, "\\*", ".*");

							if (match_info.fetch_named("start_date").empty() && match_info.fetch_named("end_date").empty() && match_info.fetch_named("start_time").empty() && match_info.fetch_named("end_time").empty())
							{
								this->_unrestricted_nicks[tokens[i]] = Glib::Regex::create(regex);
							}
							else
							{
								auto time_range = std::make_shared<TimeRange>(match_info.fetch_named("start_date"), match_info.fetch_named("end_date"), match_info.fetch_named("start_time"), match_info.fetch_named("end_time"));
								this->_time_restricted_nicks[tokens[i]] = std::make_pair(Glib::Regex::create(regex), time_range);
							}
						}

						this->_users[tokens[i]] = user;
					}
				}
			}
		}
	}
}

void Users::print_debug_info()
{
	std::set<std::pair<int, std::shared_ptr<UserStats>>> users;
	int total_lines = 0;
	int total_declared_lines = 0;

	for (auto user : this->_declared_users)
	{
		int line_count = user->get_line_count();

		total_declared_lines += line_count;
		total_lines += line_count;

		users.insert(std::make_pair(-line_count, user));
	}

	std::cout << this->_declared_users.size() << " Declared Users:" << std::endl;

	for (auto pair : users)
		std::cout << "  " << Glib::ustring::format(std::left, std::setw(30), pair.second->get_display_name()).raw() << " " << -pair.first << std::endl;

	users.clear();

	Glib::ustring::size_type max_length = 0;

	for (auto user : this->_undeclared_users)
	{
		int line_count = user->get_line_count();

		if (user->get_display_name().size() > max_length)
			max_length = user->get_display_name().size();

		total_lines += line_count;
		users.insert(std::make_pair(-line_count, user));
	}

	std::cout << std::endl << "Top 100 Unassigned Nicks:" << std::endl;

	int count = 0;

	for (auto pair : users)
	{
		if (count < 100)
		{
			std::cout << "  " << Glib::ustring::format(std::left, std::setw(max_length), pair.second->get_display_name()).raw() << " " << -pair.first << std::endl;
			count++;
		}
	}

	std::cout << std::endl << "Out of " << total_lines << " total lines, " << total_declared_lines << " (" << (total_declared_lines * 100.0 / total_lines) << "%) were assigned to declared users." << std::endl;
}

std::unordered_set<std::shared_ptr<UserStats>> Users::get_users()
{
	std::unordered_set<std::shared_ptr<UserStats>> users;

	for (auto pair : this->_users)
		users.insert(pair.second);

	return users;
}

std::shared_ptr<UserStats> Users::get_user(const Glib::ustring & nick, const Glib::ustring & userhost, std::shared_ptr<const Glib::DateTime> timestamp)
{
	Glib::ustring nickuserhost = Glib::ustring::compose("%1!%2", nick, userhost);
	Glib::ustring user_key = "";
	std::shared_ptr<TimeRange> user_time_range;

	// Search for time-restricted nicknames, as new ones can pop up at any time.
	for (auto pair : this->_time_restricted_nicks)
	{
		if (pair.second.first->match(nickuserhost) && pair.second.second->check(timestamp))
		{
			user_key = pair.first;
			user_time_range = pair.second.second;
		}
	}

	// Search the user cache and verify that any time range is still applicable.
	if (user_key.empty() && this->_user_cache.count(nickuserhost) > 0)
	{
		auto pair = this->_user_cache[nickuserhost];

		if (!pair.first || pair.first->check(timestamp))
		{
			user_key = pair.second;
			user_time_range = pair.first;
		}
	}

	// Search for any unrestricted nicknames.
	if (user_key.empty())
	{
		for (auto pair: this->_unrestricted_nicks)
		{
			if (pair.second->match(nickuserhost))
			{
				user_key = pair.first;
			}
		}
	}

	// If we still haven't found anything, use an implicitly-defined user.
	if (user_key.empty())
	{
		user_key = this->_separate_userhosts ? nickuserhost : nick;
	}

	// Store the key in the cache.
	this->_user_cache[nickuserhost] = std::make_pair(user_time_range, user_key);

	// Retrieve or create the correct user.
	if (this->_users.count(user_key) == 0)
	{
		this->_users[user_key] = std::make_shared<UserStats>(user_key);
		this->_undeclared_users.insert(this->_users[user_key]);
	}

	return this->_users[user_key];
}
