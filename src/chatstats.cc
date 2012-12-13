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

Glib::OptionEntry create_option_entry(const Glib::ustring & long_name, const gchar & short_name, const Glib::ustring & description)
{
	Glib::OptionEntry entry;

	entry.set_long_name(long_name);
	entry.set_short_name(short_name);
	entry.set_description(description);

	return entry;
}

int main(int argc, char **argv)
{
	setlocale(LC_ALL, "");

	Glib::init();
	Gio::init();

	Glib::ustring input_format = "chatstats";

	Glib::OptionGroup option_group("options", "Options", "Options to configure program");
	Glib::OptionEntry input_format_entry = create_option_entry("input-format", 'f', "Format of logs in input directory");
	option_group.add_entry(input_format_entry, input_format);

	Glib::OptionContext option_context("[COMMAND] [COMMAND-PARAMETERS]...");
	option_context.set_main_group(option_group);
	option_context.set_summary("Commands:\n  count [INPUT-DIRECTORY]\n  convert [INPUT-DIRECTORY] [OUTPUT-DIRECTORY]");
	option_context.parse(argc, argv);

	if (argc < 3)
	{
		std::cout << option_context.get_help();
		exit(EXIT_FAILURE);
	}

	std::shared_ptr<LogReader> log_reader = nullptr;

	if (input_format == "chatstats")
		log_reader = std::make_shared<ChatstatsLogReader>();
	else if (input_format == "mirc")
		log_reader = std::make_shared<MircLogReader>();
	else
	{
		std::cerr << "Invalid log format: " << input_format << std::endl;
		exit(EXIT_FAILURE);
	}

	Glib::ustring command(argv[1]);

	if (command == "convert")
	{
		if (argc < 4)
		{
			std::cout << option_context.get_help();
			exit(EXIT_FAILURE);
		}

		Glib::RefPtr<Gio::File> output_directory = Gio::File::create_for_commandline_arg(argv[3]);

		if (output_directory->query_exists())
		{
			std::cerr << "Output directory must not exist." << std::endl;
			exit(EXIT_FAILURE);
		}

		output_directory->make_directory();

		ConvertOperation operation(Gio::File::create_for_commandline_arg(argv[2]), log_reader, output_directory);
		operation.execute();
	}
	else if (command == "count")
	{
		CountOperation operation(Gio::File::create_for_commandline_arg(argv[2]), log_reader);
		operation.execute();
	}
	else
	{
		std::cout << option_context.get_help();
		exit(EXIT_FAILURE);
	}

	return 0;
}
