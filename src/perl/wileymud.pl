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
use English;
use Data::Dumper;

use Getopt::Long qw(:config no_ignore_case bundling no_pass_through);
use Time::HiRes qw( time sleep alarm );
use POSIX qw(strftime);
use Socket qw(IPPROTO_TCP TCP_NODELAY);
use Fcntl qw(F_GETFL F_SETFL O_NONBLOCK O_NDELAY);
use IO::Select;
use IO::Socket::INET;

use Mud::Options;
use Mud::Logger qw(log_boot log_info log_warn log_error log_fatal switch_logfile);
use Mud::Signals;
use Mud::Weather qw(mud_time_passed);

my $options = Mud::Options->new(@ARGV);

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

my $weather = Mud::Weather->new();

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

sleep 30;

END { log_fatal "System Halted."; }

1;

