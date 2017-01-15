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

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use Pod::Usage;
use Pod::Find qw(pod_where);
use Getopt::Long qw(:config no_ignore_case bundling no_pass_through);

use Mud::DB::Option;

use Exporter qw(import);
our @EXPORT_OK = (); #qw(wizlock debug logfile pidfile libdir specials gameport);
our @EXPORT = ();
our %EXPORT_TAGS = (all => [ @EXPORT, @EXPORT_OK ]);

=head1 FUNCTIONS

=over 8

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
        _db         => undef,
        wizlock     => undef,
        debug       => undef,
        logfile     => undef,
        pidfile     => undef,
        libdir      => undef,
        specials    => undef,
        gameport    => undef,
    };
    my $o = Mud::DB::Option->new( id => 'live' );
    $self->{_db} = $o if $o->load( speculative => 1 );
    if( !defined $self->{_db} ) {
        $o->save();
        $self->{_db} = $o;
    }
    die "Error in creating Mud::DB::Option database object: $!" if !defined $self->{_db};

    GetOptions(
        'pod'                           => sub { pod2usage(
                                                '-input'       => pod_where( {'-inc' => 1}, __PACKAGE__ ),
                                                '-verbose'     => 1,
                                                #'-noperldoc'   => 1,
                                            ); exit;
                                        },
        'help|h|?'                      => sub { usage(1); },
        'wizlock|w'                     => \$self->{wizlock},
        'debug|D'                       => \$self->{debug},
        'log|L=s'                       => \$self->{logfile},
        'pid|P=s'                       => \$self->{pidfile},
        'dir|d=s'                       => \$self->{libdir},
        'specials!'                     => \$self->{specials},
        "port|p:".$o->gameport          => \$self->{gameport},
    ) or usage(0);

    $o->wizlock( $self->{wizlock} )     if defined $self->{wizlock};
    $o->debug( $self->{debug} )         if defined $self->{debug};
    $o->logfile( $self->{logfile} )     if defined $self->{logfile};
    $o->pidfile( $self->{pidfile} )     if defined $self->{pidfile};
    $o->libdir( $self->{libdir} )       if defined $self->{libdir};
    $o->specials( $self->{specials} )   if defined $self->{specials};
    $o->gameport( $self->{gameport} )   if defined $self->{gameport} and $self->{gameport} > 0;

    $self->{_db}->save();
    bless $self, $class;
    return $self;
}

=item wizlock( [true|false] )

Wizlock prevents players from logging in.

Calling this without an argument returns the current
state, FALSE by default.  Giving it an argument will
set the wizlock.

=cut

sub wizlock {
    my ($self, $setting) = @_;
    if( defined $setting ) {
        $self->{_db}->wizlock($setting);
        $self->{_db}->save();
    }
    return $self->{_db}->wizlock;
}

=item debug( [<integer>|false] )

Debug mode enables extra debugging output.  

Calling this without an argument returns the current
state, FALSE by default.  Giving it an argument will
set the debug level.

=cut

sub debug {
    my ($self, $setting) = @_;
    if( defined $setting ) {
        $self->{_db}->debug($setting);
        $self->{_db}->save();
    }
    return $self->{_db}->debug;
}

=item logfile( [<filename>] )

Logfile is simply the filename that logging output
is currently being sent to.  The default is STDERR,
but specifying a filename changes that.

Calling this without an argument returns the current
value, undef by default.  Giving it an argument will
set the logfile to that filename.

=cut

sub logfile {
    my ($self, $setting) = @_;
    if( defined $setting ) {
        $self->{_db}->logfile($setting);
        $self->{_db}->save();
    }
    return $self->{_db}->logfile;
}

=item pidfile( [<filename>] )

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
    if( defined $setting ) {
        $self->{_db}->pidfile($setting);
        $self->{_db}->save();
    }
    return $self->{_db}->pidfile;
}

=item libdir( [<pathname>] )

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
    if( defined $setting ) {
        $self->{_db}->libdir($setting);
        $self->{_db}->save();
    }
    return $self->{_db}->libdir;
}

=item specials( [true|false] )

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
    if( defined $setting ) {
        $self->{_db}->specials($setting);
        $self->{_db}->save();
    }
    return $self->{_db}->specials;
}

=item gameport( [<integer>] )

The gameport is simply the port number which the driver
will open and use to listen for new connections.

Calling this without an argument returns the current
value.  Giving it an argument will set the gameport.

=cut

sub gameport {
    my ($self, $setting) = @_;
    if( defined $setting ) {
        $self->{_db}->gameport($setting);
        $self->{_db}->save();
    }
    return $self->{_db}->gameport;
}

=back

=cut

1;

