#!/usr/bin/perl -w

package Mud::ColorCodes;

=head1 NAME

Mud::ColorCodes - The raw color code mappings, used by Mud::Terminal

=head1 OVERVIEW

This is just the raw color code mappings used by Mud::Terminal.
I split it off as a seperate module because if you include
XTERM-256 support, it can get very large, and having to jump
to the bottom of the file and back up to edit the code would
be annoying.

The "unknown" terminal type should return the empty string for
every token.  This is used to strip out colors entirely.

Note that there is a @color_tokens list which is used to
initialize the "unknown" terminal type.  This is also exported,
to allow future code to determine if a token is valid, possibly
to support custom color preferences, or defining a whole new
terminal type on the fly.

=cut

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use Exporter qw(import);
our @EXPORT_OK = qw( %color_map @terminal_types @color_tokens );
our @EXPORT = qw();
our %EXPORT_TAGS = (all => [ @EXPORT, @EXPORT_OK ]);

our %color_map = ();

#our @terminal_types = sort (qw( unknown ansi imc2 i3 mxp xterm-256color xterm-grey));
our @terminal_types = sort(qw( unknown ansi imc2 i3 mxp html ));

our @color_tokens = sort(qw(
    BOLD FLASH ITALIC RESET REVERSE STRIKETHRU UNDERLINE
    CLEARLINE CURS_DOWN CURS_LEFT CURS_RIGHT CURS_UP
    ENDTERM HOME INITTERM RESTORE SAVE
    BLACK RED GREEN ORANGE BLUE MAGENTA CYAN GREY
    DARKGREY LIGHTRED LIGHTGREEN YELLOW LIGHTBLUE PINK LIGHTCYAN WHITE
    B_BLACK B_RED B_GREEN B_ORANGE B_BLUE B_MAGENTA B_CYAN B_GREY
    B_DARKGREY B_LIGHTRED B_LIGHTGREEN B_YELLOW B_LIGHTBLUE B_PINK B_LIGHTCYAN B_WHITE
));

$color_map{$_} = {} foreach @terminal_types;

$color_map{unknown}->{$_} = "" foreach @color_tokens;

$color_map{ansi} = {
    BOLD                => "\033[1m",
    FLASH               => "\033[5m",
    ITALIC              => "\033[3m",
    RESET               => "\033[0m",
    REVERSE             => "\033[7m",
    STRIKETHRU          => "\033[9m",
    UNDERLINE           => "\033[4m",

    CLEARLINE           => "\033[L\033[G",
    CURS_DOWN           => "\033[B",
    CURS_LEFT           => "\033[D",
    CURS_RIGHT          => "\033[C",
    CURS_UP             => "\033[A",

    ENDTERM             => "",
    HOME                => "\033[H",
    INITTERM            => "\033[H\033[2J",
    RESTORE             => "\033[u",
    SAVE                => "\033[s",

    BLACK               => "\033[30m",
    RED                 => "\033[31m",
    GREEN               => "\033[32m",
    ORANGE              => "\033[33m",
    BLUE                => "\033[34m",
    MAGENTA             => "\033[35m",
    CYAN                => "\033[36m",
    GREY                => "\033[37m",

    DARKGREY            => "\033[1;30m",
    LIGHTRED            => "\033[1;31m",
    LIGHTGREEN          => "\033[1;32m",
    YELLOW              => "\033[1;33m",
    LIGHTBLUE           => "\033[1;34m",
    PINK                => "\033[1;35m",
    LIGHTCYAN           => "\033[1;36m",
    WHITE               => "\033[1;37m",

    B_BLACK             => "\033[40m",
    B_RED               => "\033[41m",
    B_GREEN             => "\033[42m",
    B_ORANGE            => "\033[43m",
    B_BLUE              => "\033[44m",
    B_MAGENTA           => "\033[45m",
    B_CYAN              => "\033[46m",
    B_GREY              => "\033[47m",

    B_DARKGREY          => "\033[40m",
    B_LIGHTRED          => "\033[41m",
    B_LIGHTGREEN        => "\033[42m",
    B_YELLOW            => "\033[43m",
    B_LIGHTBLUE         => "\033[44m",
    B_PINK              => "\033[45m",
    B_LIGHTCYAN         => "\033[46m",
    B_WHITE             => "\033[47m",
};

$color_map{imc2} = {
    BOLD                => "~L",
    FLASH               => "~\$",
    ITALIC              => "~i",
    RESET               => "~!",
    REVERSE             => "~v",
    STRIKETHRU          => "~s",
    UNDERLINE           => "~u",

    CLEARLINE           => "",
    CURS_DOWN           => "",
    CURS_LEFT           => "",
    CURS_RIGHT          => "",
    CURS_UP             => "",

    ENDTERM             => "",
    HOME                => "",
    INITTERM            => "",
    RESTORE             => "",
    SAVE                => "",

    BLACK               => "~x",
    RED                 => "~r",
    GREEN               => "~g",
    ORANGE              => "~y",
    BLUE                => "~b",
    MAGENTA             => "~p",
    CYAN                => "~c",
    GREY                => "~w",

    DARKGREY            => "~z",
    LIGHTRED            => "~R",
    LIGHTGREEN          => "~G",
    YELLOW              => "~Y",
    LIGHTBLUE           => "~B",
    PINK                => "~P",
    LIGHTCYAN           => "~C",
    WHITE               => "~W",

    B_BLACK             => "^x",
    B_RED               => "^r",
    B_GREEN             => "^g",
    B_ORANGE            => "^O",
    B_BLUE              => "^b",
    B_MAGENTA           => "^p",
    B_CYAN              => "^c",
    B_GREY              => "^w",

    B_DARKGREY          => "^z",
    B_LIGHTRED          => "^R",
    B_LIGHTGREEN        => "^G",
    B_YELLOW            => "^Y",
    B_LIGHTBLUE         => "^B",
    B_PINK              => "^P",
    B_LIGHTCYAN         => "^C",
    B_WHITE             => "^W",
};

$color_map{i3} = {
    BOLD                => "%^BOLD%^",
    FLASH               => "%^FLASH%^",
    ITALIC              => "%^ITALIC%^",
    RESET               => "%^RESET%^",
    REVERSE             => "%^REVERSE%^",
    STRIKETHRU          => "%^STRIKETHRU%^",
    UNDERLINE           => "%^UNDERLINE%^",

    CLEARLINE           => "%^CLEARLINE%^",
    CURS_DOWN           => "%^CURS_DOWN%^",
    CURS_LEFT           => "%^CURS_LEFT%^",
    CURS_RIGHT          => "%^CURS_RIGHT%^",
    CURS_UP             => "%^CURS_UP%^",

    ENDTERM             => "%^ENDTERM%^",
    HOME                => "%^HOME%^",
    INITTERM            => "%^INITTERM%^",
    RESTORE             => "%^RESTORE%^",
    SAVE                => "%^SAVE%^",

    BLACK               => "%^BLACK%^",
    RED                 => "%^RED%^",
    GREEN               => "%^GREEN%^",
    ORANGE              => "%^ORANGE%^",
    BLUE                => "%^BLUE%^",
    MAGENTA             => "%^MAGENTA%^",
    CYAN                => "%^CYAN%^",
    GREY                => "%^WHITE%^",

    DARKGREY            => "%^BOLD%^BLACK%^",
    LIGHTRED            => "%^BOLD%^RED%^",
    LIGHTGREEN          => "%^BOLD%^GREEN%^",
    YELLOW              => "%^YELLOW%^",
    LIGHTBLUE           => "%^BOLD%^BLUE%^",
    PINK                => "%^BOLD%^MAGENTA%^",
    LIGHTCYAN           => "%^BOLD%^CYAN%^",
    WHITE               => "%^BOLD%^WHITE%^",

    B_BLACK             => "%^B_BLACK%^",
    B_RED               => "%^B_RED%^",
    B_GREEN             => "%^B_GREEN%^",
    B_ORANGE            => "%^B_ORANGE%^",
    B_BLUE              => "%^B_BLUE%^",
    B_MAGENTA           => "%^B_MAGENTA%^",
    B_CYAN              => "%^B_CYAN%^",
    B_GREY              => "%^B_WHITE%^",

    B_DARKGREY          => "%^B_BLACK%^",
    B_LIGHTRED          => "%^B_RED%^",
    B_LIGHTGREEN        => "%^B_GREEN%^",
    B_YELLOW            => "%^B_YELLOW%^",
    B_LIGHTBLUE         => "%^B_BLUE%^",
    B_PINK              => "%^B_MAGENTA%^",
    B_LIGHTCYAN         => "%^B_CYAN%^",
    B_WHITE             => "%^B_WHITE%^",
};

$color_map{mxp} = {
    BOLD                => "<BOLD>",
    FLASH               => "<FONT COLOR=BLINK>",
    ITALIC              => "<ITALIC>",
    RESET               => "<RESET>",
    REVERSE             => "<FONT COLOR=INVERSE>",
    STRIKETHRU          => "<STRIKEOUT>",
    UNDERLINE           => "<UNDERLINE>",

    CLEARLINE           => "",
    CURS_DOWN           => "",
    CURS_LEFT           => "",
    CURS_RIGHT          => "",
    CURS_UP             => "",

    ENDTERM             => "",
    HOME                => "",
    INITTERM            => "",
    RESTORE             => "",
    SAVE                => "",

    BLACK               => "<COLOR FORE=\"#000000\">",
    RED                 => "<COLOR FORE=\"#bb0000\">",
    GREEN               => "<COLOR FORE=\"#00bb00\">",
    ORANGE              => "<COLOR FORE=\"#bbbb00\">",
    BLUE                => "<COLOR FORE=\"#0000bb\">",
    MAGENTA             => "<COLOR FORE=\"#bb00bb\">",
    CYAN                => "<COLOR FORE=\"#00bbbb\">",
    GREY                => "<COLOR FORE=\"#bbbbbb\">",

    DARKGREY            => "<COLOR FORE=\"#555555\">",
    LIGHTRED            => "<COLOR FORE=\"#ff5555\">",
    LIGHTGREEN          => "<COLOR FORE=\"#55ff55\">",
    YELLOW              => "<COLOR FORE=\"#ffff55\">",
    LIGHTBLUE           => "<COLOR FORE=\"#5555ff\">",
    PINK                => "<COLOR FORE=\"#ff55ff\">",
    LIGHTCYAN           => "<COLOR FORE=\"#55ffff\">",
    WHITE               => "<COLOR FORE=\"#ffffff\">",

    B_BLACK             => "<COLOR BACK=\"#000000\">",
    B_RED               => "<COLOR BACK=\"#bb0000\">",
    B_GREEN             => "<COLOR BACK=\"#00bb00\">",
    B_ORANGE            => "<COLOR BACK=\"#bbbb00\">",
    B_BLUE              => "<COLOR BACK=\"#0000bb\">",
    B_MAGENTA           => "<COLOR BACK=\"#bb00bb\">",
    B_CYAN              => "<COLOR BACK=\"#00bbbb\">",
    B_GREY              => "<COLOR BACK=\"#bbbbbb\">",
                                                      
    B_DARKGREY          => "<COLOR BACK=\"#555555\">",
    B_LIGHTRED          => "<COLOR BACK=\"#ff5555\">",
    B_LIGHTGREEN        => "<COLOR BACK=\"#55ff55\">",
    B_YELLOW            => "<COLOR BACK=\"#ffff55\">",
    B_LIGHTBLUE         => "<COLOR BACK=\"#5555ff\">",
    B_PINK              => "<COLOR BACK=\"#ff55ff\">",
    B_LIGHTCYAN         => "<COLOR BACK=\"#55ffff\">",
    B_WHITE             => "<COLOR BACK=\"#ffffff\">",
};

$color_map{html} = {
    BOLD                => "<span style=\"font-weight: bold;\">",
    FLASH               => "",
    ITALIC              => "<span style=\"font-style: italic;\">",
    RESET               => "</span>",
    REVERSE             => "",
    STRIKETHRU          => "<span style=\"text-decoration: line-through;\">",
    UNDERLINE           => "<span style=\"text-decoration: underline;\">",

    CLEARLINE           => "",
    CURS_DOWN           => "",
    CURS_LEFT           => "",
    CURS_RIGHT          => "",
    CURS_UP             => "",

    ENDTERM             => "",
    HOME                => "",
    INITTERM            => "",
    RESTORE             => "",
    SAVE                => "",

    BLACK               => "<span style=\"color: #000000;\">",
    RED                 => "<span style=\"color: #bb0000;\">",
    GREEN               => "<span style=\"color: #00bb00;\">",
    ORANGE              => "<span style=\"color: #bbbb00;\">",
    BLUE                => "<span style=\"color: #0000bb;\">",
    MAGENTA             => "<span style=\"color: #bb00bb;\">",
    CYAN                => "<span style=\"color: #00bbbb;\">",
    GREY                => "<span style=\"color: #bbbbbb;\">",

    DARKGREY            => "<span style=\"color: #555555;\">",
    LIGHTRED            => "<span style=\"color: #ff5555;\">",
    LIGHTGREEN          => "<span style=\"color: #55ff55;\">",
    YELLOW              => "<span style=\"color: #ffff55;\">",
    LIGHTBLUE           => "<span style=\"color: #5555ff;\">",
    PINK                => "<span style=\"color: #ff55ff;\">",
    LIGHTCYAN           => "<span style=\"color: #55ffff;\">",
    WHITE               => "<span style=\"color: #ffffff;\">",

    B_BLACK             => "<span style=\"background-color: #000000;\">",
    B_RED               => "<span style=\"background-color: #bb0000;\">",
    B_GREEN             => "<span style=\"background-color: #00bb00;\">",
    B_ORANGE            => "<span style=\"background-color: #bbbb00;\">",
    B_BLUE              => "<span style=\"background-color: #0000bb;\">",
    B_MAGENTA           => "<span style=\"background-color: #bb00bb;\">",
    B_CYAN              => "<span style=\"background-color: #00bbbb;\">",
    B_GREY              => "<span style=\"background-color: #bbbbbb;\">",
                                                      
    B_DARKGREY          => "<span style=\"background-color: #555555;\">",
    B_LIGHTRED          => "<span style=\"background-color: #ff5555;\">",
    B_LIGHTGREEN        => "<span style=\"background-color: #55ff55;\">",
    B_YELLOW            => "<span style=\"background-color: #ffff55;\">",
    B_LIGHTBLUE         => "<span style=\"background-color: #5555ff;\">",
    B_PINK              => "<span style=\"background-color: #ff55ff;\">",
    B_LIGHTCYAN         => "<span style=\"background-color: #55ffff;\">",
    B_WHITE             => "<span style=\"background-color: #ffffff;\">",
};

1;
