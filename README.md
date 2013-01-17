chatstats
=========

chatstats is a tool for generating HTML statistics from the logs from some kind
of chat room (generally an IRC channel). Ultimately, it is intended to be
similar to the following projects:

* [IRCStats](http://humdi.net/ircstats/)
* [mIRCStats](http://www.nic.fi/~mauvinen/mircstats/)
* [pisg](http://pisg.sourceforge.net/)

chatstats is currently nowhere near as powerful as any of these other solutions,
so feel free to try them out.

Usage
-----

`chatstats [OPTION...] [COMMAND] [COMMAND-PARAMETERS]...`

### Commands

The heart of chatstats is the set of various commands it understands. Several
commands are either remnants of earlier development or built for more esoteric
purposes, while the `generate` command is the primary one used for generating
HTML statistics. Nonetheless, the following commands are currently available:

#### `convert`

`convert [INPUT-DIRECTORY] [OUTPUT-DIRECTORY]`

Converts the logs in the given input directory into chatstats' native format.
The given output directory must not already exist (as a safety precaution).

#### `count`

`count [INPUT-DIRECTORY]`

Counts and lists the number of lines (taken as the number of messages plus the
number of actions) on each day for the set of log files in the given input
directory.

#### `coverage`

`coverage [INPUT-DIRECTORY]`

Displays coverage statistics for the logs in the given input directory. That is,
it reports on how complete the logs are (as a percentage of the total time
spanned) and also displays some of the largest gaps in coverage.

#### `frequency`

`frequency [INPUT-DIRECTORY] [TARGET]`

Finds words in logs in the given input directory that occur roughly as often as
specified by the target period (in seconds). It excludes words that ever have
a gap longer than eight times the given period, and ranks the remaining words by
their difference to the target. (This command is more esoteric, used for finding
suitable words for a secret word game.)

#### `generate`

`generate [INPUT-DIRECTORY] [OUTPUT-DIRECTORY]`

Generates HTML statistics for the logs in the given input directory and writes
the HTML files to the given output directory. The output directory must not
already exist. The generated HTML files assume that
[Blueprint](http://www.blueprintcss.org/) is available in the `css/blueprint`
directory, so you should add these files after the HTML is generated.

### Options

These are the available options:

#### `--debug-users`

This option prints out a debug report to help identify important nicknames that
have not been explicitly assigned to a user in the users file.

#### `--separate-userhosts`

Tells chatstats to separate users by the full nick!user@host specification,
rather than merely by nick. In cases where the hostname is indeterminable, the
resultant user will have a description of <nick>!@.

This option is best used in conjunction with a nick configuration file
specifying users and their nicks or userhosts. Explicitly declared users will
display by their alias (if defined) or by their most used nick.

#### `--input-format`

This option controls the format the input logs are in. chatstats currently
supports two input formats:

##### chatstats

The native log format of chatstats. At this time, chatstats is the only software
that outputs this format. It is designed to be excellent for maintaining
accurate logs.

##### mirc

The log format generated by some versions of [mIRC](http://www.mirc.com).
chatstats should have no trouble reading this format, but it may report a large
number of useless lines as being unrecognized, due to limited testing on logs
actually generated by mIRC.

Certain versions of mIRC generated logs that did not adequately distinguish
between actions and joins, parts, quits, and other events. These logs are not
currently supported, but support for them can be added on request.

#### `--users-file`

This option allows the user to specify a file to control manually linking
nicknames for statistics purposes. The file currently supports two directives:

##### `USER`

Declares a new user. Subsequent directives will apply to this user, until
another `USER` directive is encountered. An optional alias for the user may be
specified after `USER` and a single tab or space.

##### `NICK`

Assigns nicknames to the most recently declared user. Accepts any number of
nicknames, separated by tabs or spaces. Multiple `NICK` directives are allowed.

An extended nickname syntax is available to restrict nickname linkages to
specific date or time ranges or specific user/host combinations. This is useful
if two different users used the same nickname at different points in time. (This
is most useful on logs spanning many years, where nicknames may be potentially
reused after some time.) It's also useful for linking a large number of
nicknames used by a single unique user/host combination. The syntax is as
follows:

`<nickname>!<user>@<host>#<start_date>/<end_date>+<start_time>/<end_time>`

A complete example is `Joe!*@*.com#2002-10-03/2004-11-01@00:00:00/12:00:00`.
This would capture events from a user named Joe connecting from a .com hostname,
occurring from 2002-10-03 up to 2004-11-01 (but not including 2004-11-01),
occurring from midnight to noon. The date and time sections are completely
independent and tested separately.

Linking based on user/host combinations is limited based on the completeness of
the logs used. Since hosts are generally not available on each line in logs,
this information must be inferred from join and nick change events.

All fields are optional, but the delimiters are required if using the extended
syntax. The dates must be in YYYY-MM-DD format, and the times must be in
HH:MM:SS format. You may use either the hostname extended section, the time
range extended section, or both.

Two different wildcard operators are supported: `*` and `?`. `*` matches zero
or more characters, and `?` matches a single optional character.

Bugs and Feature Requests
-------------------------

Please submit bug reports or feature requests to the issue tracker at the
[chatstats GitHub page](https://github.com/aexoden/chatstats).

If requesting support for a new log format, or reporting a bug in an existing
format, please submit a log sample in your report. The more complete the sample,
the better. (Rarer event types such as channel CTCPs or channel notices are
recommended to be included.)

License
-------

chatstats is licensed under the
[MIT license](http://opensource.org/licenses/MIT).

Installation
------------

chatstats has the following dependencies:

* [GCC 4.6+](http://www.gcc.org)
* [glibmm 2.28](http://www.gtkmm.org)
* [tup](http://gittup.org/tup/)

To build chatstats, ensure dependencies are installed, change to the directory
containing the chatstats sources, and execute `./build`. chatstats should be
successfully built. There is currently no installation procedure, but the binary
is self-contained, so you should be able to copy it to any standard binary
location.
