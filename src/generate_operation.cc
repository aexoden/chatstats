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

#include <giomm/datainputstream.h>
#include <glibmm/miscutils.h>
#include <glibmm/regex.h>

#include "generate_operation.hh"
#include "util.hh"
#include "version.hh"

GenerateOperation::GenerateOperation(const Glib::RefPtr<Gio::File> & input_directory, const std::shared_ptr<LogReader> & reader, const Glib::RefPtr<Gio::File> & output_directory, const Glib::RefPtr<Gio::File> & users_file, const bool debug, const bool separate_userhosts) :
	Operation(input_directory, reader),
	_output_directory(output_directory),
	_users_directory(Gio::File::create_for_path(Glib::build_filename(output_directory->get_path(), "users"))),
	_users_file(users_file),
	_database(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE),
	_separate_userhosts(separate_userhosts),
	_debug(debug)
{
	this->_initialize_database();
	this->_users_directory->make_directory();
	this->_load_users_file();
	std::cout << "Processing files..." << std::endl;
}

void GenerateOperation::_cleanup()
{
	this->_initialize_database_indexes();
	this->_apply_users_file();
	this->_create_undeclared_users();
	this->_assign_aliases();

	std::cout << "Generating output" << std::endl;
	this->_output_css_default();
	this->_output_html_index();

	if (this->_debug)
		this->_print_debug_info();
}

void GenerateOperation::_handle_sessions(const std::vector<std::shared_ptr<Session>> &sessions)
{
	SQLite::Transaction transaction(this->_database);
	SQLite::Statement insert_event_query(this->_database, "INSERT INTO events (type, timestamp, subject_nickuserhost_id, object_nickuserhost_id, message) VALUES (:type, :timestamp, :subject_nickuserhost_id, :object_nickuserhost_id, :message)");

	for (auto & session: sessions)
	{
		if (!this->_last_session_stop || (session->start->to_unix() > this->_last_session_stop->to_unix() + 600))
			this->_userhosts.clear();

		for (auto & event : session->events)
		{
			const int subject_nickuserhost_id = this->_get_nickuserhost_id(event->subject);

			if (event->type == EventType::NICK_CHANGE)
				this->_userhosts[event->object.nick] = this->_userhosts[event->subject.nick];

			const int object_nickuserhost_id = this->_get_nickuserhost_id(event->object);

			insert_event_query.bind(":type", static_cast<int>(event->type));
			insert_event_query.bind(":timestamp", event->timestamp->format("%Y-%m-%d %H:%M:%S"));
			insert_event_query.bind(":message", event->message);

			if (subject_nickuserhost_id >= 0)
				insert_event_query.bind(":subject_nickuserhost_id", subject_nickuserhost_id);
			else
				insert_event_query.bind(":subject_nickuserhost_id");

			if (object_nickuserhost_id >= 0)
				insert_event_query.bind(":object_nickuserhost_id", object_nickuserhost_id);
			else
				insert_event_query.bind(":object_nickuserhost_id");

			insert_event_query.exec();
			insert_event_query.reset();
		}

		if (this->_target.empty())
			this->_target = session->target;

		this->_last_session_stop = session->stop;
	}

	transaction.commit();
}

void GenerateOperation::_initialize_database()
{
	this->_database.exec("PRAGMA case_sensitive_like = TRUE");
	this->_database.exec("PRAGMA foreign_keys = TRUE");

	this->_initialize_database_tables();
	this->_initialize_database_queries();
}

void GenerateOperation::_initialize_database_tables()
{
	this->_database.exec(R"EOF(
		CREATE TABLE events (
			id INTEGER PRIMARY KEY,
			type INTEGER NOT NULL,
			timestamp TEXT NOT NULL,
			subject_nickuserhost_id INTEGER REFERENCES nickuserhosts(id),
			object_nickuserhost_id INTEGER REFERENCES nickuserhosts(id),
			message TEXT
		);
	)EOF");

	this->_database.exec(R"EOF(
		CREATE TABLE nickuserhosts (
			id INTEGER PRIMARY KEY,
			user_id INTEGER REFERENCES users(id),
			nickuserhost TEXT NOT NULL
		);
	)EOF");

	this->_database.exec(R"EOF(
		CREATE TABLE users (
			id INTEGER PRIMARY KEY,
			alias TEXT,
			automatic INTEGER NOT NULL
		)
	)EOF");

	this->_database.exec(R"EOF(
		CREATE VIEW nicks AS
			SELECT DISTINCT u.id AS id, SUBSTR(n.nickuserhost, 0, INSTR(n.nickuserhost, '!')) AS nick
			FROM users u, nickuserhosts n
			WHERE u.id = n.user_id
	)EOF");
}

void GenerateOperation::_initialize_database_indexes()
{
	std::cout << "Creating indexes..." << std::endl;

	this->_database.exec(R"EOF(
		CREATE INDEX subject_nickuserhost_id_index ON events (subject_nickuserhost_id ASC);
		CREATE INDEX object_nickuserhost_id_index ON events (object_nickuserhost_id ASC);
		CREATE INDEX timestamp_index ON events (timestamp ASC);

		CREATE INDEX user_id_index ON nickuserhosts (user_id ASC);
	)EOF");
}

void GenerateOperation::_initialize_database_queries()
{
	this->_nickuserhost_insert_query = std::make_shared<SQLite::Statement>(this->_database, "INSERT INTO nickuserhosts (user_id, nickuserhost) VALUES (:user_id, :nickuserhost)");
}

void GenerateOperation::_insert_nick_specification(std::list<std::pair<std::shared_ptr<const NickSpecification>, int>> & nick_specifications, const std::shared_ptr<const NickSpecification> & nick_specification, const int user_id)
{
	auto iter = nick_specifications.begin();

	for (; iter != nick_specifications.end(); iter++)
		if (iter->first->regex->match(nick_specification->nickuserhost_specification))
			break;

	nick_specifications.insert(iter, std::make_pair(nick_specification, user_id));
}

void GenerateOperation::_load_users_file()
{
	std::cout << "Loading users file..." << std::endl;

	SQLite::Transaction transaction(this->_database);
	SQLite::Statement query(this->_database, "INSERT INTO users (alias, automatic) VALUES (:alias, 0)");

	int current_user_id = -1;

	for (auto & user : this->_parse_users_file())
	{
		query.bind(":alias", user->alias);
		query.exec();
		query.reset();

		current_user_id = this->_database.getLastInsertRowid();

		for (auto & nick_specification : user->nick_specifications)
		{
			if (nick_specification->time_range)
				this->_insert_nick_specification(this->_timed_nick_specifications, nick_specification, current_user_id);
			else
				this->_insert_nick_specification(this->_untimed_nick_specifications, nick_specification, current_user_id);
		}
	}

	transaction.commit();
}

void GenerateOperation::_apply_users_file()
{
	std::cout << "Applying users file..." << std::endl;

	SQLite::Transaction transaction(this->_database);
	auto query = std::make_shared<SQLite::Statement>(this->_database, "SELECT id, nickuserhost FROM nickuserhosts");

	std::vector<std::pair<int, std::string>> nickuserhosts;

	while (query->executeStep())
		nickuserhosts.push_back(std::make_pair(query->getColumn(0).getInt(), query->getColumn(1).getText()));

	SQLite::Statement insert_nickuserhost_query(this->_database, "INSERT INTO nickuserhosts (user_id, nickuserhost) VALUES (:user_id, :nickuserhost)");
	SQLite::Statement update_nickuserhost_query(this->_database, "UPDATE nickuserhosts SET user_id = :user_id WHERE id = :nickuserhost_id");
	Glib::ustring update_events_query_template("UPDATE events SET %1_nickuserhost_id = :new_nickuserhost_id WHERE %1_nickuserhost_id = :nickuserhost_id AND %2");

	for (auto & spec_pair : this->_timed_nick_specifications)
	{
		const auto & nick_specification = spec_pair.first;
		const int user_id = spec_pair.second;

		Glib::ustring time_range_expression = nick_specification->time_range->get_sql_expression();

		std::vector<std::shared_ptr<SQLite::Statement>> update_events_queries;
		update_events_queries.push_back(std::make_shared<SQLite::Statement>(this->_database, Glib::ustring::compose(update_events_query_template, "subject", time_range_expression).data()));
		update_events_queries.push_back(std::make_shared<SQLite::Statement>(this->_database, Glib::ustring::compose(update_events_query_template, "object", time_range_expression).data()));

		for (auto & pair : nickuserhosts)
		{
			const int nickuserhost_id = pair.first;
			const Glib::ustring & nickuserhost = pair.second;

			if (nick_specification->regex->match(nickuserhost))
			{
				insert_nickuserhost_query.bind(":user_id", user_id);
				insert_nickuserhost_query.bind(":nickuserhost", nickuserhost);
				insert_nickuserhost_query.exec();
				insert_nickuserhost_query.reset();

				const int new_nickuserhost_id = this->_database.getLastInsertRowid();

				for (auto & query : update_events_queries)
				{
					query->bind(":new_nickuserhost_id", new_nickuserhost_id);
					query->bind(":nickuserhost_id", nickuserhost_id);
					query->exec();
					query->reset();
				}
			}
		}
	}

	transaction.commit();
}

std::vector<std::shared_ptr<UserSpecification>> GenerateOperation::_parse_users_file() const
{
	std::vector<std::shared_ptr<UserSpecification>> users;

	if (!this->_users_file || !this->_users_file->query_exists())
		return users;

	auto users_stream = Gio::DataInputStream::create(this->_users_file->read());
	std::string line;

	std::shared_ptr<UserSpecification> user;

	while (users_stream->read_line(line))
	{
		if (line.empty())
			continue;

		std::vector<Glib::ustring> tokens = Glib::Regex::create("[\t ]+")->split(line);

		if (tokens[0] == "USER" || (tokens[0] == "NICK" && !user))
		{
			const Glib::ustring user_alias = tokens[0] == "USER" && line.length() > 5 ? line.substr(5) : "";
			user = std::make_shared<UserSpecification>(user_alias);
			users.push_back(user);
		}

		if (tokens[0] == "NICK")
		{
			for (size_t i = 1; i < tokens.size(); i++)
			{
				if (tokens[i].empty())
					continue;

				user->nick_specifications.push_back(std::make_shared<NickSpecification>(tokens[i]));
			}
		}
	}

	return users;
}

Glib::RefPtr<Gio::File> GenerateOperation::_get_user_directory(const Glib::ustring & alias) const
{
	Glib::ustring url_alias = urlify(alias);

	int i = 1;
	Glib::RefPtr<Gio::File> directory = Gio::File::create_for_path(Glib::build_filename(this->_users_directory->get_path(), url_alias));

	while (directory->query_exists())
	{
		directory = Gio::File::create_for_path(Glib::build_filename(this->_users_directory->get_path(), Glib::ustring::compose("%1-%2", url_alias, i)));
		i++;
	}

	directory->make_directory();

	return directory;
}

void GenerateOperation::_assign_aliases()
{
	std::cout << "Assigning aliases" << std::endl;

	SQLite::Transaction transaction(this->_database);
	SQLite::Statement select_query(this->_database, "SELECT u.id, SUBSTR(n.nickuserhost, 0, INSTR(n.nickuserhost, '!')) AS nick, COUNT(*) FROM nickuserhosts n, users u LEFT OUTER JOIN events e ON e.subject_nickuserhost_id = n.id AND (e.type == :action_type OR e.type == :message_type) WHERE n.user_id = u.id AND u.alias = '' GROUP BY u.id, nick;");

	std::unordered_map<int, std::pair<std::string, int>> aliases;

	select_query.bind(":action_type", static_cast<int>(EventType::ACTION));
	select_query.bind(":message_type", static_cast<int>(EventType::MESSAGE));

	while (select_query.executeStep())
	{
		const int user_id = select_query.getColumn(0).getInt();
		const int count = select_query.getColumn(2).getInt();

		if (count > aliases[user_id].second)
			aliases[user_id] = std::make_pair(select_query.getColumn(1).getText(), count);
	}

	SQLite::Statement update_query(this->_database, "UPDATE users SET alias = :alias WHERE id = :user_id");

	for (auto & pair : aliases)
	{
		const int user_id = pair.first;
		Glib::ustring alias = pair.second.first;

		update_query.bind(":alias", alias);
		update_query.bind(":user_id", user_id);

		update_query.exec();
		update_query.reset();
	}

	transaction.commit();
}

void GenerateOperation::_create_undeclared_users()
{
	std::cout << "Creating undeclared users" << std::endl;

	SQLite::Transaction transaction(this->_database);

	if (this->_separate_userhosts)
	{
		std::vector<std::pair<int, std::string>> nickuserhosts;
		SQLite::Statement select_query(this->_database, "SELECT id, nickuserhost FROM nickuserhosts WHERE user_id IS NULL");

		while (select_query.executeStep())
			nickuserhosts.push_back(std::make_pair(select_query.getColumn(0).getInt(), select_query.getColumn(1).getText()));

		SQLite::Statement insert_query(this->_database, "INSERT INTO users (alias, automatic) VALUES (:alias, 1)");
		SQLite::Statement update_query(this->_database, "UPDATE nickuserhosts SET user_id = :user_id WHERE id = :id");

		for (auto & pair : nickuserhosts)
		{
			insert_query.bind(":alias", pair.second);
			insert_query.exec();
			insert_query.reset();

			const int user_id = this->_database.getLastInsertRowid();

			update_query.bind(":user_id", user_id);
			update_query.bind(":id", pair.first);
			update_query.exec();
			update_query.reset();
		}
	}
	else
	{
		std::vector<std::string> nicks;
		SQLite::Statement select_query(this->_database, "SELECT SUBSTR(nickuserhost, 0, INSTR(nickuserhost, '!')) AS nick FROM nickuserhosts WHERE user_id IS NULL GROUP BY nick");

		while (select_query.executeStep())
			nicks.push_back(select_query.getColumn(0).getText());

		SQLite::Statement insert_query(this->_database, "INSERT INTO users (alias, automatic) VALUES ('', 1)");
		SQLite::Statement update_query(this->_database, "UPDATE nickuserhosts SET user_id = :user_id WHERE user_id IS NULL AND SUBSTR(nickuserhost, 0, INSTR(nickuserhost, '!')) = :nick");

		for (auto nick : nicks)
		{
			insert_query.exec();
			insert_query.reset();

			const int user_id = this->_database.getLastInsertRowid();

			update_query.bind(":user_id", user_id);
			update_query.bind(":nick", nick);
			update_query.exec();
			update_query.reset();
		}
	}

	transaction.commit();
}

void GenerateOperation::_output_css_default() const
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

void GenerateOperation::_output_html_index()
{
	Glib::RefPtr<Gio::File> output_file = Gio::File::create_for_path(Glib::build_filename(this->_output_directory->get_path(), "index.html"));
	Glib::RefPtr<Gio::DataOutputStream> output_stream = Gio::DataOutputStream::create(output_file->create_file());

	this->_output_html_header(output_stream, "Overview");
	this->_output_html_section_overall_ranking(output_stream);
	this->_output_html_footer(output_stream);
}

void GenerateOperation::_output_html_header(const Glib::RefPtr<Gio::DataOutputStream> & output_stream, const Glib::ustring & title, const Glib::ustring & media_prefix) const
{
	output_stream->put_string("<!DOCTYPE html>\n");
	output_stream->put_string("<html>\n");
	output_stream->put_string("\t<head>\n");
	output_stream->put_string("\t\t<meta charset=\"utf-8\">\n");
	output_stream->put_string(Glib::ustring::compose("\t\t<title>%1 Statistics &raquo; %2</title>\n", this->_target, title));
	output_stream->put_string(Glib::ustring::compose("\t\t<link rel=\"stylesheet\" href=\"%1css/blueprint/screen.css\" media=\"screen, projection\">\n", media_prefix));
	output_stream->put_string(Glib::ustring::compose("\t\t<link rel=\"stylesheet\" href=\"%1css/blueprint/print.css\" media=\"print\">\n", media_prefix));
	output_stream->put_string(Glib::ustring::compose("\t\t<link rel=\"stylesheet\" href=\"%1css/default.css\">\n", media_prefix));
	output_stream->put_string("\t</head>\n");
	output_stream->put_string("\t<body>\n");
	output_stream->put_string("\t\t<div id=\"header\">\n");
	output_stream->put_string(Glib::ustring::compose("\t\t\t<h1>%1 Statistics &raquo; %2</h1>\n", this->_target, title));
	output_stream->put_string("\t\t</div>\n");
	output_stream->put_string("\t\t<div id=\"content\">\n");
}

void GenerateOperation::_output_html_footer(const Glib::RefPtr<Gio::DataOutputStream> & output_stream) const
{
	Glib::DateTime now = Glib::DateTime::create_now_utc();

	Glib::ustring duration = Glib::ustring::format(std::fixed, std::setprecision(5), now.difference(*this->_start_time) / 1000000.0);

	output_stream->put_string("\t\t</div>\n");
	output_stream->put_string("\t\t<div id=\"footer\">\n");
	output_stream->put_string(Glib::ustring::compose("\t\t\t<p>Generated by <a href=\"https://github.com/aexoden/chatstats\">chatstats</a> %1 in %2 seconds on %3 at %4 UTC</p>\n", CHATSTATS_VERSION, duration, now.format("%Y-%m-%d"), now.format("%H:%M:%S")));
	output_stream->put_string("\t\t</div>\n");
	output_stream->put_string("\t</body>\n");
	output_stream->put_string("</html>\n");
}

void GenerateOperation::_output_html_user_index(const Glib::RefPtr<Gio::File> & user_file, const int user_id)
{
	auto query = std::make_shared<SQLite::Statement>(this->_database, ("SELECT alias FROM users WHERE id = :user_id"));
	query->bind(":user_id", user_id);
	query->executeStep();

	Glib::ustring alias = query->getColumn(0).getText();

	const Glib::RefPtr<Gio::DataOutputStream> output_stream = Gio::DataOutputStream::create(user_file->create_file());

	this->_output_html_header(output_stream, Glib::ustring::compose("Users &raquo; %1", encode_html_characters(alias)), "../../");

	output_stream->put_string("\t\t\t<table>\n");
	output_stream->put_string("\t\t\t\t<thead>\n");
	output_stream->put_string("\t\t\t\t\t<tr><th>Nickname</th><th>Lines</th></tr>\n");
	output_stream->put_string("\t\t\t\t<tbody>\n");

	query = std::make_shared<SQLite::Statement>(this->_database, "SELECT SUBSTR(n.nickuserhost, 0, INSTR(n.nickuserhost, '!')) AS nick, COUNT(e.id) AS lines FROM nickuserhosts n LEFT OUTER JOIN events e ON n.id = e.subject_nickuserhost_id AND (e.type == :action_type OR e.type == :message_type) WHERE n.user_id = :user_id GROUP BY nick ORDER BY lines DESC");
	query->bind(":user_id", user_id);
	query->bind(":action_type", static_cast<int>(EventType::ACTION));
	query->bind(":message_type", static_cast<int>(EventType::MESSAGE));

	while (query->executeStep())
		output_stream->put_string(Glib::ustring::compose("\t\t\t\t\t<tr><td>%1</td><td>%2</td></tr>\n", query->getColumn(0).getText(), query->getColumn(1).getInt()));

	output_stream->put_string("\t\t\t\t</tbody>\n");
	output_stream->put_string("\t\t\t</table>\n");

	this->_output_html_footer(output_stream);
}

void GenerateOperation::_output_html_section_overall_ranking(const Glib::RefPtr<Gio::DataOutputStream> & output_stream)
{
	output_stream->put_string("\t\t\t<table>\n");
	output_stream->put_string("\t\t\t\t<thead>\n");
	output_stream->put_string("\t\t\t\t\t<tr><th>Rank</th><th>User</th><th>Lines</th><th>Nicknames</th></tr>\n");
	output_stream->put_string("\t\t\t\t</thead>\n");
	output_stream->put_string("\t\t\t\t<tbody>\n");

	std::shared_ptr<SQLite::Statement> query;

	std::unordered_map<int, unsigned int> nick_counts;

	query = std::make_shared<SQLite::Statement>(this->_database, "SELECT id, COUNT(*) AS nicks FROM nicks GROUP BY id");

	while (query->executeStep())
		nick_counts[query->getColumn(0).getInt()] = query->getColumn(1).getInt();

	unsigned int index = 0;
	unsigned int current_rank = 0;
	unsigned int last_score = 0;

	query = std::make_shared<SQLite::Statement>(this->_database, "SELECT u.id, u.alias, COUNT(e.id) AS count FROM users u, nickuserhosts n LEFT OUTER JOIN events e ON n.id = e.subject_nickuserhost_id AND (e.type = :action_type OR e.type = :message_type) WHERE u.id = n.user_id GROUP BY u.id ORDER BY count DESC");

	query->bind(":action_type", static_cast<int>(EventType::ACTION));
	query->bind(":message_type", static_cast<int>(EventType::MESSAGE));

	while (query->executeStep())
	{
		const int user_id = query->getColumn(0).getInt();
		const Glib::ustring alias = query->getColumn(1).getText();
		const unsigned int score = query->getColumn(2).getInt();

		if (score < last_score || last_score == 0)
			current_rank = index + 1;

		if (current_rank > 1000)
			break;

		const Glib::RefPtr<Gio::File> user_directory = this->_get_user_directory(alias);
		this->_output_html_user_index(Gio::File::create_for_path(Glib::build_filename(user_directory->get_path(), "index.html")), user_id);

		output_stream->put_string(Glib::ustring::compose("\t\t\t\t\t<tr><td>%1</td><td><a href=\"%2\">%3</a></td><td>%4</td><td>%5</td></tr>\n", current_rank, Glib::ustring::compose("users/%1/", user_directory->get_basename()), encode_html_characters(alias), score, nick_counts[user_id]));

		last_score = score;
		index++;
	}

	output_stream->put_string("\t\t\t\t</tbody>\n");
	output_stream->put_string("\t\t\t</table>\n");

	if (index < nick_counts.size())
		output_stream->put_string(Glib::ustring::compose("\t\t\t<p>Plus %1 others who obviously weren't important enough for the table</p>\n", nick_counts.size() - index));
}

int GenerateOperation::_get_nickuserhost_id(const User & user)
{
	if (user.nick.empty())
		return -1;

	if (!user.user.empty() && !user.host.empty())
		this->_userhosts[user.nick] = Glib::ustring::compose("%1@%2", user.user, user.host);

	if (this->_userhosts.count(user.nick) == 0)
		this->_userhosts[user.nick] = "@";

	std::string nickuserhost(user.nick + '!' + this->_userhosts[user.nick]);

	if (this->_nickuserhost_ids.count(nickuserhost) == 0)
	{
		this->_nickuserhost_insert_query->bind(":user_id");

		for (auto & pair : this->_untimed_nick_specifications)
		{
			const auto & nick_specification = pair.first;
			const int user_id = pair.second;

			if (nick_specification->regex->match(nickuserhost))
			{
				this->_nickuserhost_insert_query->bind(":user_id", user_id);
				break;
			}
		}

		this->_nickuserhost_insert_query->bind(":nickuserhost", nickuserhost);
		this->_nickuserhost_insert_query->exec();
		this->_nickuserhost_insert_query->reset();

		this->_nickuserhost_ids[nickuserhost] = this->_database.getLastInsertRowid();
	}

	return this->_nickuserhost_ids[nickuserhost];
}

void GenerateOperation::_print_debug_info()
{
	unsigned int total_lines = 0;
	unsigned int declared_lines = 0;

	std::cout << "Declared Users" << std::endl;
	std::cout << "--------------------------------------------------------------------------------" << std::endl;

	auto query = std::make_shared<SQLite::Statement>(this->_database, "SELECT u.alias, COUNT(e.id) AS lines FROM users u, nickuserhosts n LEFT OUTER JOIN events e on e.subject_nickuserhost_id = n.id AND e.type IN (:action_type, :message_type) WHERE u.id = n.user_id AND u.automatic = 0 GROUP BY u.id ORDER BY lines DESC");
	query->bind(":action_type", static_cast<int>(EventType::ACTION));
	query->bind(":message_type", static_cast<int>(EventType::MESSAGE));

	while (query->executeStep())
	{
		std::cout << Glib::ustring::format(std::left, std::setw(70), query->getColumn(0).getText()).raw() << " " << query->getColumn(1).getInt() << std::endl;
		declared_lines += query->getColumn(1).getInt();
		total_lines += query->getColumn(1).getInt();
	}

	std::cout << std::endl;
	std::cout << "Automatic Users" << std::endl;
	std::cout << "--------------------------------------------------------------------------------" << std::endl;

	query = std::make_shared<SQLite::Statement>(this->_database, "SELECT u.alias, COUNT(e.id) AS lines FROM users u, nickuserhosts n LEFT OUTER JOIN events e on e.subject_nickuserhost_id = n.id AND e.type IN (:action_type, :message_type) WHERE u.id = n.user_id AND u.automatic = 1 GROUP BY u.id ORDER BY lines DESC");
	query->bind(":action_type", static_cast<int>(EventType::ACTION));
	query->bind(":message_type", static_cast<int>(EventType::MESSAGE));

	while (query->executeStep())
	{
		std::cout << Glib::ustring::format(std::left, std::setw(70), query->getColumn(0).getText()).raw() << " " << query->getColumn(1).getInt() << std::endl;
		total_lines += query->getColumn(1).getInt();
	}

	std::cout << std::endl;
	std::cout << "Summary" << std::endl;
	std::cout << "--------------------------------------------------------------------------------" << std::endl;
	std::cout << "Total Lines: " << total_lines << std::endl;
	std::cout << "Lines Assigned to Declared Users: " << declared_lines << " (" << (declared_lines * 100.0 / total_lines) << "%)" << std::endl;
}
