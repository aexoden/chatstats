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

#include "user_specification.hh"
#include "util.hh"

UserSpecification::UserSpecification(const Glib::ustring & alias) :
	alias(alias)
{ }

NickSpecification::NickSpecification(const Glib::ustring & specification) :
	specification(specification)
{
	auto match_regex = Glib::Regex::create("(?P<nick>[^!#]*)(!(?P<userhost>[^@]*@[^#]*))?(#(?P<start_date>[0-9]{4}-[0-9]{2}-[0-9]{2})?/(?P<end_date>[0-9]{4}-[0-9]{2}-[0-9]{2})?\\+(?P<start_time>[0-9]{2}:[0-9]{2}:[0-9]{2})?/(?P<end_time>[0-9]{2}:[0-9]{2}:[0-9]{2})?)?");

	Glib::MatchInfo match_info;

	if (match_regex->match(this->specification, match_info))
	{
		auto userhost = match_info.fetch_named("userhost");

		if (userhost.empty())
			userhost = "*@*";

		this->nickuserhost_specification = Glib::ustring::compose("%1!%2", match_info.fetch_named("nick"), userhost);

		if (!match_info.fetch_named("start_date").empty() || !match_info.fetch_named("end_date").empty() || !match_info.fetch_named("start_time").empty() || !match_info.fetch_named("end_time").empty())
		{
			Glib::ustring start_date = match_info.fetch_named("start_date");
			Glib::ustring end_date = match_info.fetch_named("end_date");
			Glib::ustring start_time = match_info.fetch_named("start_time");
			Glib::ustring end_time = match_info.fetch_named("end_time");

			this->time_range = std::make_shared<TimeRange>(start_date, end_date, start_time, end_time);
		}

		Glib::ustring regex_string = Glib::ustring::compose("^%1$", Glib::Regex::escape_string(this->nickuserhost_specification));

		string_replace(regex_string, "\\*", ".*");
		string_replace(regex_string, "\\?", ".?");

		this->regex = Glib::Regex::create(regex_string);
	}
}

Glib::ustring NickSpecification::get_like_expression() const
{
	Glib::ustring expression = this->nickuserhost_specification;

	string_replace(expression, "!", "!!");
	string_replace(expression, "%", "!%");
	string_replace(expression, "_", "!_");
	string_replace(expression, "*", "%");
	string_replace(expression, "?", "_");

	return expression;
}
