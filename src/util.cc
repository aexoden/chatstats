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

#include "util.hh"

void encode_character(Glib::ustring & string, char search, const Glib::ustring & replace)
{
	size_t pos = 0;

	while ((pos = string.find(search, pos)) != std::string::npos)
	{
		string.replace(pos, 1, replace);
		pos += replace.length();
	}
}

Glib::ustring encode_html_characters(Glib::ustring string)
{
	encode_character(string, '&', "&amp;");
	encode_character(string, '<', "&lt;");
	encode_character(string, '>', "&gt;");
	encode_character(string, '"', "&quot;");

	return string;
}

void string_replace(Glib::ustring & string, const Glib::ustring & search, const Glib::ustring & replace)
{
	size_t pos = 0;

	while ((pos = string.find(search, pos)) != std::string::npos)
	{
		string.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}

Glib::ustring urlify(const Glib::ustring & string)
{
	return Glib::Regex::create("[[:^alnum:]]+")->replace_literal(string, 0, "_", static_cast<Glib::RegexMatchFlags>(0));
}
