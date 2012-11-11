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

#include <set>

#include <glibmm/init.h>
#include <glibmm/miscutils.h>
#include <giomm/init.h>

#include "log_reader.hh"

std::set<std::string> get_filenames(Glib::RefPtr<Gio::File> directory)
{
	std::set<std::string> filenames;

	Glib::RefPtr<Gio::FileEnumerator> file_enumerator = directory->enumerate_children("standard::name");

	while (Glib::RefPtr<Gio::FileInfo> file_info = file_enumerator->next_file())
	{
		filenames.insert(Glib::build_filename(directory->get_path(), file_info->get_name()));
	}

	return filenames;
}

int main(int argc, char **argv)
{
	Glib::init();
	Gio::init();

	LogReader log_reader;

	for (const std::string & filename : get_filenames(Gio::File::create_for_commandline_arg(argv[1])))
	{
		log_reader.read(Gio::File::create_for_path(filename));
	}

	return 0;
}

