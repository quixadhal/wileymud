#!/usr/bin/perl -w

package Mud::Signals;

=head1 NAME

Mud::Signals - Sets up signal handlers

=head1 OVERVIEW

This package just provides functions to setup some default
signal handlers for the MUD.

=head1 FUNCTIONS

=over 8

=cut

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use Mud::Logger;

use Exporter qw(import);
our @EXPORT_OK = qw(checkpoint shutdown_request reboot_request);

my $ticks = 0;
my $MUD_REBOOT = 0;
my $MUD_HALT = 42;

=item checkpoint()

A function which should be called from a timer, and which
should detect game "ticks" not updating... a symptom of the MUD
driver being too slow to keep up at a reasonable pace.

=cut

sub checkpoint {
    my $signame = shift;

    if ($ticks <= 0) {
        log_fatal "CHECKPOINT shutdown:  ticks not updated.";
        #close_sockets(0);
        #close_whod();
        #proper_exit(MUD_HALT);
        exit $MUD_HALT;
    } else {
        $ticks = 0;
    }
}

=item shutdown_request()

The game is being shutdown, either due to an external signal, or as a result
of a command calling this function.

=cut

sub shutdown_request {
    my $signame = shift;

    log_fatal "Received SIG$signame.  Shutting down!";
    #close_sockets(0);
    #close_whod();
    #proper_exit(MUD_HALT);
    exit $MUD_HALT;
}

=item reboot_request()

The game is being shutdown, either due to an external signal, or as a result
of a command calling this function.  The game will exit with a different
return code to indicate to any controlling script that it should restart.

=cut

sub reboot_request {
    my $signame = shift;

    log_fatal "Received SIG$signame.  Rebooting!";
    #close_sockets(0);
    #close_whod();
    #proper_exit(MUD_REBOOT);
    exit $MUD_REBOOT;
}

=back

=cut

$SIG{$_}        = 'IGNORE' foreach qw( HUP QUIT USR2 PIPE ALRM CHLD CONT STOP TSTP TTIN TTOU URG XCPU XFSZ WINCH IO SYS);
$SIG{$_}        = \&shutdown_request foreach qw( INT USR1 PWR );
$SIG{$_}        = \&reboot_request foreach qw( HUP TERM );
$SIG{VTALRM}    = \&checkpoint;
$SIG{__WARN__}  = \&log_warn;
$SIG{__DIE__}   = \&log_fatal;

log_boot "Signal trapping.";

1;

