#!/usr/bin/perl -w

package Mud::Options;

=head1 NAME

Mud::Options - A small module to get command-line arguments

=head1 SYNOPSIS

wileymud.pl [-h?wD] [-L logfile] [-P pidfile] [-d directory] [-p port]

=head1 OPTIONS

=over 8

=item B<--help> or B<-h> or B<-?>

A more concise help screen, for typical use.

=item B<--pod>

This helpful POD document!

=item B<--wizlock> or B<-w>

Enabled wizlock mode, which prevents players from logging in.

=item B<--debug> or B<-D>

Enables extra debugging code.

=item B<--log> filename or B<-L> filename

Logs various errors, warnings, and information to the specified filename.

=item B<--pid> filename or B<-P> filename

Writes the current process PID out to a file, for system startup scripts.

=item B<--dir> directory or B<-d> directory

Specifies an alternate data directory.  The default is "../lib"

=item B<--specials>

Enables special routines for mobiles.  This defaults to true, but can be
turned off via --no-specials to aid in debugging.

=item B<--port> port-number or B<-p> port-number

Specifies the login port number for connections.  This defaults to 7000.

=back

=head1 DESCRIPTION

This is a module which handles collecting command line arguments and
parsing their values for use in controlling some aspects of this perl
mud.

=cut

=head1 FUNCTIONS

=over 8

=cut

use strict;
use warnings;
use English;
use Data::Dumper;

use Pod::Usage;
use Pod::Find qw(pod_where);
use Getopt::Long qw(:config no_ignore_case bundling no_pass_through);

use Exporter qw(import);
our @EXPORT_OK = (); #qw(wizlock debug logfile pidfile libdir specials gameport);

my $wizlock     = undef;
my $debug       = undef;
my $logfile     = undef;
my $pidfile     = undef;
my $libdir      = "./lib";
my $specials    = 1;
my $gameport    = 7000;

=item usage()

This is a simple usage function to describe the command line
parameters and how to use them.  It is probably redundant
with the POD documentation, but we'll see which works better.

=cut

sub usage {
    my $long = shift;

    print "\n  usage: wileymud.pl [-w] [-D] [-L file] [-P file] [-d dir] [-p port]\n\n";
    print <<EOM
options:
        --help              This lovely help screen
        --pod               Show POD documentation
        --wizlock           Prevents players from logging in
        --debug             Enables extra debugging code
        --log <logfile>     Logs to a filename instead of stdout
        --pid <pidfile>     Writes the process ID to a filename
        --dir <libdir>      Specifies where the top data dir is, rather than .
        --no-specials       Disables special code for NPC's
        --port <port>       Port number to run on

EOM
    if $long;
    exit 1;
}

=back

=head1 METHODS

=over 8

=cut

=item new()

Constructor.  WHen we create a new instance, we parse the ARGV
arguments and fill the options hash, which all our accessor
functions refer to.

=cut

sub new {
    my $class = shift;
    my $self = { 
        _wizlock    => $wizlock,
        _debug      => $debug,
        _logfile    => $logfile,
        _pidfile    => $pidfile,
        _libdir     => $libdir,
        _specials   => $specials,
        _gameport   => $gameport,
    };
    GetOptions(
        'pod'               => sub { pod2usage(
                                     '-input'       => pod_where({'-inc' => 1}, __PACKAGE__),
                                     '-verbose'     => 1,
                                     #'-noperldoc'   => 1,
                                 ); exit;
                             },
        'help|h|?'          => sub { usage(1); },
        'wizlock|w'         => \$self->{_wizlock},
        'debug|D'           => \$self->{_debug},
        'log|L=s'           => \$self->{_logfile},
        'pid|P=s'           => \$self->{_pidfile},
        'dir|d=s'           => \$self->{_libdir},
        'specials!'         => \$self->{_specials},
        "port|p:$gameport"  => \$self->{_gameport},
    ) or usage(0);
#    ) or pod2usage( '-input'       => pod_where({'-inc' => 1}, __PACKAGE__),
#                    '-verbose'     => 0) && exit;
    bless $self, $class;
    return $self;
}

=item wizlock()

Wizlock prevents players from logging in.

Calling this without an argument returns the current
state, FALSE by default.  Giving it an argument will
set the wizlock.

=cut

sub wizlock {
    my ($self, $setting) = @_;
    $self->{_wizlock} = $setting if defined $setting;
    return $self->{_wizlock};
}

=item debug()

Debug mode enables extra debugging output.  

Calling this without an argument returns the current
state, FALSE by default.  Giving it an argument will
set the debug level.

=cut

sub debug {
    my ($self, $setting) = @_;
    $self->{_debug} = $setting if defined $setting;
    return $self->{_debug};
}

=item logfile()

Logfile is simply the filename that logging output
is currently being sent to.  The default is STDERR,
but specifying a filename changes that.

Calling this without an argument returns the current
value, undef by default.  Giving it an argument will
set the logfile to that filename.

=cut

sub logfile {
    my ($self, $setting) = @_;
    $self->{_logfile} = $setting if defined $setting;
    return $self->{_logfile};
}

=item pidfile()

The pidfile is an optional file that the driver will
use to store the current process ID.  This is used
by some /etc/init.d scripts to control startup and
shutdown.

Calling this without an argument returns the current
setting, undef by default.  Giving it an argument will
set the pidfile to that filename.

=cut

sub pidfile {
    my ($self, $setting) = @_;
    $self->{_pidfile} = $setting if defined $setting;
    return $self->{_pidfile};
}

=item libdir()

The libdir is the data directory used by the MUD to
store and retrieve game data.  Normally, this is left
as "lib", but can be changed if desired.

Calling this without an argument returns the current
value, "lib" by default.  Giving it an argument will
set the libdir to that pathname.

Note that the pathname should NOT end in a slash.

=cut

sub libdir {
    my ($self, $setting) = @_;
    $self->{'_libdir'} = $setting if defined $setting;
    return $self->{'_libdir'};
}

=item specials()

The specials parameter controls NPC behavior.  When
special routines are enabled (the default), each NPC
may have a particular chunk of code that drives how
it behaves.

In cases where a special routine is causing issues,
the driver can be run with --no-specials, which casues
all NPC's to use a generic behavior routine that is
(hopefully) well tested.

=cut

sub specials {
    my ($self, $setting) = @_;
    $self->{_specials} = $setting if defined $setting;
    return $self->{_specials};
}

=item gameport()

The gameport is simply the port number which the driver
will open and use to listen for new connections.

Calling this without an argument returns the current
value.  Giving it an argument will set the gameport.

=cut

sub gameport {
    my ($self, $setting) = @_;
    $self->{_gameport} = $setting if defined $setting;
    return $self->{_gameport};
}

=back

=cut

1;

