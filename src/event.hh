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

#ifndef CHATSTATS_EVENT_HH
#define CHATSTATS_EVENT_HH

#include <memory>

#include <glibmm/datetime.h>

#include "user.hh"

enum class EventType
{
	ACTION,
	CTCP,
	JOIN,
	KICK,
	MESSAGE,
	MODE_CHANGE,
	NICK_CHANGE,
	NOTICE,
	PART,
	TOPIC_CHANGE,
	QUIT,
	PARSE_IGNORE,
	PARSE_SESSION_START,
	PARSE_SESSION_STOP,
	PARSE_SESSION_TARGET
};

class Event
{
	public:
		Event(const EventType type, std::shared_ptr<const Glib::DateTime> timestamp, const User & subject, const User & object, const Glib::ustring & message);

		const EventType type;

		const std::shared_ptr<const Glib::DateTime> timestamp;

		const User subject;
		const User object;

		const Glib::ustring message;
};

#endif // CHATSTATS_EVENT_HH

