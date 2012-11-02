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

#include <glibmm/init.h>
#include <glibmm/miscutils.h>
#include <giomm/init.h>

#include "log_reader.hh"

int main(int argc, char **argv)
{
	Glib::init();
	Gio::init();

	LogReader log_reader;

	Glib::RefPtr<Gio::File> input_directory = Gio::File::create_for_commandline_arg(argv[1]);

	Glib::RefPtr<Gio::FileEnumerator> files = input_directory->enumerate_children("standard::name");

	while (Glib::RefPtr<Gio::FileInfo> file_info = files->next_file())
	{
		Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(Glib::build_filename(input_directory->get_path(), file_info->get_name()));

		log_reader.read(file);
	}

	return 0;
}

