#!/usr/bin/perl -w

=head1 NAME

wileymud - The venerable WileyMUD III, rewritten in perl.

=head1 SYNOPSYS

Please see L<Mud::Options> for usage details.

=head1 DESCRIPTION

WileyMUD was the first text MUD I ever played.  It was a very
popular DikuMUD that ran at Western Michigan University for
several years, and ate the lives and free time of far too many
students.

A few years after the original team shut it down, I managed to
ressurect it briefly.  During my tenure, the code was stabilized
to the point of being quite solid and reliable, and some new
features were added... some good, some not so good.

In any case, I've neglected it over the years because modern
languages have given me a severe dislike for dealing with strings
as pointers and arrays, and having to constantly fiddle with
memory management.

Thus, I finally decided to try to rewrite the entire game in
a langauge that avoids those issues... the one I know best is
perl.  So, this project is born.


=cut

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use Getopt::Long qw(:config no_ignore_case bundling no_pass_through);
use Time::HiRes qw( time sleep alarm );
use POSIX qw(strftime);
use Socket qw(IPPROTO_TCP TCP_NODELAY);
use Fcntl qw(F_GETFL F_SETFL O_NONBLOCK O_NDELAY);
use IO::Select;
use IO::Socket::INET;

use Mud::Options;
use Mud::Logger qw(:all);
use Mud::Signals;
use Mud::GameTime;
use Mud::Weather;
use Mud::Terminal;
use Mud::Theme;
use Mud::Docs;

my $options = Mud::Options->new(@ARGV);

Mud::Logger::setup_options($options);

my $start_time = time();

if (defined $options->libdir) {
    my $libdir = $options->libdir;

    die "Cannot change directory to $libdir" if ! -d $libdir;
    chdir $libdir;
}

if (defined $options->pidfile) {
    my $pidfile = $options->pidfile;

    log_boot "Dumping PID to %s", $pidfile;
    open FP, ">$pidfile" or die "Cannot open PID file $pidfile";
    print FP "$$\n";
    close FP;
}

switch_logfile $options->logfile if defined $options->logfile;

log_boot "Game port: %d", $options->gameport;
log_boot "Random number seed value: %d", srand;

my @descriptor_list = ();

log_boot "Boot DB -- BEGIN.";

my $time_daemon = Mud::GameTime->new();
my $weather_daemon = Mud::Weather->new($time_daemon);
my $term = Mud::Terminal->new('ansi', 132, 40);
my $theme = Mud::Theme->new('default', $term);

log_debug $theme->colorize( "%^RED%^Hello%^RESET%^\n" );
log_debug $theme->colorize( "%^DAMAGE%^Heal me!%^RESET%^  %^HEALING%^Done.%^RESET%^\n" );
log_debug $theme->terminal_type;
log_debug $theme->width;
log_debug $theme->height;

my $docs = Mud::Docs->new();

log_info "News: %s\n", Dumper($docs->news);
log_info "Credits: %s\n", Dumper($docs->credits);
log_info "MOTD: %s\n", Dumper($docs->motd);

exit 1;

#    log_boot("- Reading help");
#    file_to_string(HELP_PAGE_FILE, help);
#    log_boot("- Reading info");
#    file_to_string(INFO_FILE, info);
#    log_boot("- Reading wizlist");
#    file_to_string(WIZLIST_FILE, wizlist);
#    log_boot("- Reading wiz motd");
#    file_to_string(WMOTD_FILE, wmotd);
#    log_boot("- Reading greetings");
#    file_to_string(GREETINGS_FILE, greetings);
#    log_boot("- Reading login menu");
#    file_to_prompt(LOGIN_MENU_FILE, login_menu);
#    log_boot("- Reading sex menu");
#    file_to_prompt(SEX_MENU_FILE, sex_menu);
#    log_boot("- Reading race menu");
#    file_to_prompt(RACE_MENU_FILE, race_menu);
#    log_boot("- Reading class menu");
#    file_to_prompt(CLASS_MENU_FILE, class_menu);
#    log_boot("- Reading race help");
#    file_to_prompt(RACE_HELP_FILE, race_help);
#    log_boot("- Reading class help");
#    file_to_prompt(CLASS_HELP_FILE, class_help);
#    log_boot("- Reading story");
#    file_to_string(STORY_FILE, the_story);
#    log_boot("- Reading suicide warning");
#    file_to_prompt(SUICIDE_WARN_FILE, suicide_warn);
#    log_boot("- Reading suicide result");
#    file_to_string(SUICIDE_DONE_FILE, suicide_done);

#    log_boot("- Loading rent mode");
#    log_boot("- Loading player list");
#    log_boot("- Loading reboot frequency");
#    log_boot("- Loading help files");

#    log_boot("- Loading fight messages");
#    load_messages();
#    log_boot("- Loading social messages");
#    boot_social_messages();
#    log_boot("- Loading pose messages");
#    boot_pose_messages();


log_boot "Opening mother connection.";

sub tweak_socket {
    my $socket = shift;
    
    my $flags = fcntl($socket, F_GETFL, 0) or warn "Cannot get flags for socket: $!";
    fcntl($socket, F_SETFL, $flags | O_NONBLOCK) or warn "Cannot set socket to nonblocking: $!";
    setsockopt($socket,SOL_SOCKET,SO_LINGER,pack("l*",0,0)) or warn "Cannot set SO_LINGER option for socket: $!";
    setsockopt($socket,IPPROTO_TCP,TCP_NODELAY,1) or warn "Cannot disable Nagle for socket: $!";
}

my $listening_socket = IO::Socket::INET->new(
    Listen      => 5,
    LocalPort   => $options->gameport,
    ReuseAddr   => 1,
) or die "Cannot bind main socket: $!";
tweak_socket($listening_socket);





#load_db();
#log_boot("Opening WHO port.");
#init_whod(port);
#log_boot("Opening I3 connection.");
#i3_startup(FALSE, 3000, FALSE);
#log_boot("Entering game loop.");
#game_loop(s);
#i3_shutdown(0);
#close_sockets(s);
#close_whod();
#unload_db();
#if (diku_reboot) {
#    log_boot("Rebooting.");
#    return MUD_REBOOT;
#}
#log_boot("Normal termination of game.");
#return MUD_HALT;					       /* what's so great about HHGTTG, anyhow? */

for( my $i = 0; $i < 60; $i++ ) {
    $time_daemon->update;
    $weather_daemon->update;
    log_info "Tick! Hour: %d, Month: %d", $time_daemon->hour, $time_daemon->month;
    sleep 5;
}

END { log_fatal "System Halted."; }

1;

