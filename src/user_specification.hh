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

#ifndef CHATSTATS_USER_SPECIFICATION_HH
#define CHATSTATS_USER_SPECIFICATION_HH

#include <memory>
#include <vector>

#include <glibmm/regex.h>
#include <glibmm/ustring.h>

#include "time_range.hh"

class NickSpecification
{
	public:
		NickSpecification(const Glib::ustring & specification);

		Glib::ustring get_like_expression() const;
		Glib::RefPtr<Glib::Regex> get_regex() const;

		const Glib::ustring specification;
		Glib::ustring nickuserhost_specification;

		std::shared_ptr<const TimeRange> time_range;
};

class UserSpecification
{
	public:
		UserSpecification(const Glib::ustring & alias);

		const Glib::ustring alias;

		std::vector<std::shared_ptr<const NickSpecification>> nick_specifications;
};

#endif // CHATSTATS_USER_SPECIFICATION_HH

