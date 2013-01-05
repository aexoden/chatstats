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
#include <glibmm/stringutils.h>

#include "generate_operation.hh"
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
	Glib::ustring users_filename = "";

	bool debug_users = false;
	bool separate_userhosts = false;

	Glib::OptionGroup option_group("options", "Options", "Options to configure program");
	Glib::OptionEntry debug_users_entry = create_option_entry("debug-users", 'd', "Debug user nickname linking");
	option_group.add_entry(debug_users_entry, debug_users);

	Glib::OptionEntry separate_userhosts_entry = create_option_entry("separate-userhosts", 's', "Whether to separate users by userhost");
	option_group.add_entry(separate_userhosts_entry, separate_userhosts);

	Glib::OptionEntry input_format_entry = create_option_entry("input-format", 'f', "Format of logs in input directory");
	option_group.add_entry(input_format_entry, input_format);

	Glib::OptionEntry users_file_entry = create_option_entry("users-file", 'u', "User configuration file");
	option_group.add_entry(users_file_entry, users_filename);

	Glib::OptionContext option_context("[COMMAND] [COMMAND-PARAMETERS]...");
	option_context.set_main_group(option_group);
	option_context.set_summary("Commands:\n  convert [INPUT-DIRECTORY] [OUTPUT-DIRECTORY]\n  count [INPUT-DIRECTORY]\n  coverage [INPUT-DIRECTORY]\n  frequency [INPUT-DIRECTORY] [TARGET]\n  generate [INPUT-DIRECTORY] [OUTPUT-DIRECTORY]");
	option_context.parse(argc, argv);

	if (argc < 3)
	{
		std::cout << option_context.get_help();
		exit(EXIT_FAILURE);
	}

	Glib::RefPtr<Gio::File> input_directory = Gio::File::create_for_commandline_arg(argv[2]);

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

		ConvertOperation operation(input_directory, log_reader, output_directory);
		operation.execute();
	}
	else if (command == "count")
	{
		CountOperation operation(input_directory, log_reader);
		operation.execute();
	}
	else if (command == "coverage")
	{
		CoverageOperation operation(input_directory, log_reader);
		operation.execute();
	}
	else if (command == "frequency")
	{
		if (argc < 4)
		{
			std::cout << option_context.get_help();
			exit(EXIT_FAILURE);
		}

		double target = Glib::Ascii::strtod(argv[3]);

		FrequencyOperation operation(input_directory, log_reader, target);
		operation.execute();
	}
	else if (command == "generate")
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

		Glib::RefPtr<Gio::File> users_file;

		if (!users_filename.empty())
			users_file = Gio::File::create_for_commandline_arg(users_filename);

		output_directory->make_directory();

		GenerateOperation operation(input_directory, log_reader, output_directory, users_file, debug_users, separate_userhosts);
		operation.execute();
	}
	else
	{
		std::cout << option_context.get_help();
		exit(EXIT_FAILURE);
	}

	return 0;
}
