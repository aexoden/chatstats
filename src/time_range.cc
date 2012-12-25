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

bool TimeRange::check(std::shared_ptr<const Glib::DateTime> timestamp)
{
	return this->_check(this->_start_date, this->_end_date, timestamp->format("%Y-%m-%d")) && this->_check(this->_start_time, this->_end_time, timestamp->format("%H:%M:%S"));
}

bool TimeRange::_check(const Glib::ustring & start, const Glib::ustring & end, const Glib::ustring & value)
{
	if (start.empty() && end.empty())
		return true;
	if (start.empty())
		return value < end;
	else if (end.empty())
		return value >= start;
	else if (start <= end)
		return value >= start && value < end;
	else
		return value >= start || value < end;
}

TimeRange::TimeRange(const Glib::ustring & start_date, const Glib::ustring & end_date, const Glib::ustring & start_time, const Glib::ustring & end_time) :
	_start_date(start_date),
	_end_date(end_date),
	_start_time(start_time),
	_end_time(end_time)
{ }
