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

There is currently only one option available, which controls the format the
input logs are in. chatstats currently supports two input formats:

#### chatstats

The native log format of chatstats. At this time, chatstats is the only software
that outputs this format. It is designed to be excellent for maintaining
accurate logs.

#### mirc

The log format generated by some versions of [mIRC](http://www.mirc.com).
chatstats should have no trouble reading this format, but it may report a large
number of useless lines as being unrecognized, due to limited testing on logs
actually generated by mIRC.

Certain versions of mIRC generated logs that did not adequately distinguish
between actions and joins, parts, quits, and other events. These logs are not
currently supported, but support for them can be added on request.

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
containing the chatstats sources, and execute `tup upd`. chatstats should be
successfully built. There is currently no installation procedure, but the binary
is self-contained, so you should be able to copy it to any standard binary
location.
