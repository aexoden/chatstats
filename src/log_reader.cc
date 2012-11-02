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

#include <iostream>

#include <glibmm/convert.h>
#include <giomm/datainputstream.h>

#include "log_reader.hh"

void LogReader::read(const Glib::RefPtr<Gio::File> & file)
{
	this->_load_file_contents(file);

	this->_lines.clear();
}

void LogReader::_load_file_contents(const Glib::RefPtr<Gio::File> & file)
{
	Glib::RefPtr<Gio::DataInputStream> file_stream = Gio::DataInputStream::create(file->read());
	std::string line;

	const std::string encodings[] = {"UTF-8", "CP1252", "ISO-8859-1"};

	while (file_stream->read_line(line))
	{
		for (const std::string & encoding : encodings)
		{
			try
			{
				this->_lines.push_back(Glib::convert(line, "UTF-8", encoding));
				break;
			}
			catch (Glib::ConvertError e) {}
		}
	}
}
