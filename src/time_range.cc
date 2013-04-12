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

#include <glibmm/regex.h>

#include "time_range.hh"

TimeRange::TimeRange(const Glib::ustring & start_date, const Glib::ustring & end_date, const Glib::ustring & start_time, const Glib::ustring & end_time) :
	start_date(start_date),
	end_date(end_date),
	start_time(start_time),
	end_time(end_time)
{ }

Glib::ustring TimeRange::get_sql_expression() const
{
	Glib::ustring expression;

	this->_append_sql_expression(expression, "DATE(timestamp) >= '%1'", this->start_date);
	this->_append_sql_expression(expression, "DATE(timestamp) < '%1'", this->end_date);
	this->_append_sql_expression(expression, "TIME(timestamp) >= '%1'", this->start_time);
	this->_append_sql_expression(expression, "TIME(timestamp) < '%1'", this->end_time);

	return expression;
}

void TimeRange::_append_sql_expression(Glib::ustring & expression, const Glib::ustring & parameter_template, const Glib::ustring & value) const
{
	if (!value.empty())
	{
		if (!expression.empty())
			expression += " AND ";

		expression += Glib::ustring::compose(parameter_template, value);
	}
}
