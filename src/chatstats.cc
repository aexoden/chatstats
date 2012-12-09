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

#include <locale.h>

#include <iostream>

#include <giomm/file.h>
#include <giomm/init.h>
#include <glibmm/init.h>
#include <glibmm/optioncontext.h>
#include <glibmm/optionentry.h>
#include <glibmm/optiongroup.h>

#include "operation.hh"

int main(int argc, char **argv)
{
	setlocale(LC_ALL, "");

	Glib::init();
	Gio::init();

	Glib::OptionGroup option_group("options", "Options", "Options to configure program");

	Glib::OptionEntry entry;
	Glib::ustring input_format = "chatstats";
	entry.set_long_name("input-format");
	entry.set_short_name('f');
	entry.set_description("Format of logs in input directory");
	option_group.add_entry(entry, input_format);

	Glib::OptionContext option_context;
	option_context.set_main_group(option_group);
	option_context.parse(argc, argv);

	if (argc < 3)
	{
		std::cerr << "Insufficient number of arguments" << std::endl;
		return -1;
	}

	if (std::string(argv[1]) == "convert")
	{
		if (argc < 4)
		{
			std::cerr << "=" << std::endl;
			return -1;
		}

		Glib::RefPtr<Gio::File> output_directory = Gio::File::create_for_commandline_arg(argv[3]);

		if (output_directory->query_exists())
		{
			std::cerr << "Output directory must not exist." << std::endl;
			return -1;
		}

		output_directory->make_directory();

		std::shared_ptr<LogReader> log_reader = nullptr;

		if (input_format == "chatstats")
			log_reader = std::make_shared<ChatstatsLogReader>();
		else if (input_format == "mirc")
			log_reader = std::make_shared<MircLogReader>();
		else
		{
			std::cerr << "Invalid log format type. " << std::endl;
			return -1;
		}

		ConvertOperation operation(Gio::File::create_for_commandline_arg(argv[2]), log_reader, output_directory);

		operation.execute();
	}
	else
	{
		std::cerr << "Invalid mode." << std::endl;
		return -1;
	}

	return 0;
}

