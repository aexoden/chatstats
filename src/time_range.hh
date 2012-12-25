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

#ifndef CHATSTATS_TIME_RANGE_HH
#define CHATSTATS_TIME_RANGE_HH

#include <memory>

#include <glibmm/datetime.h>

class TimeRange
{
	public:
		TimeRange(const Glib::ustring & start_date, const Glib::ustring & end_date, const Glib::ustring & start_time, const Glib::ustring & end_time);

		bool check(std::shared_ptr<const Glib::DateTime> timestamp);

	private:
		bool _check(const Glib::ustring & start, const Glib::ustring & end, const Glib::ustring & value);

		Glib::ustring _start_date;
		Glib::ustring _end_date;

		Glib::ustring _start_time;
		Glib::ustring _end_time;
};

#endif // CHATSTATS_TIME_RANGE_HH

