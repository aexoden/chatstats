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

#include <algorithm>

#include "session.hh"

#include <iostream>

std::shared_ptr<Session> Session::split(const Glib::DateTime & timestamp)
{
	if (timestamp.to_unix() < this->start->to_unix() || timestamp.to_unix() > this->stop->to_unix())
		return nullptr;

	auto session = std::make_shared<Session>();

	auto iter = this->events.begin();
	while (iter != this->events.end() && (*iter)->timestamp->to_unix() < timestamp.to_unix())
		iter++;

	std::move(iter, this->events.end(), session->events.begin());

	if (session->events.empty())
		return nullptr;

	session->start = std::make_shared<Glib::DateTime>(timestamp);
	session->stop = this->stop;
	session->target = this->target;

	this->stop = std::make_shared<Glib::DateTime>(timestamp);

	return session;
}
