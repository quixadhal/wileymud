#!/usr/bin/perl -w

package Mud::Logger;

=head1 NAME

Mud::Logger - A logging package for our perl mud

=head1 OVERVIEW

This package provides a centralized way to manage all sorts of
erorr messages, warnings, and general information the programmer
wants to pass along to log files, or to administrative staff
in-game.

=head1 FUNCTIONS

=over 8

=cut

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use Time::HiRes qw( time sleep alarm );
use POSIX qw(strftime);

use Exporter qw(import);
our @EXPORT_OK = qw(switch_logfile buffer clear_buffer buffer_limit setup_options);
our @EXPORT = qw(log_boot log_info log_debug log_warn log_error log_fatal);
our %EXPORT_TAGS = (all => [ @EXPORT, @EXPORT_OK ], logging => [ @EXPORT ]);

my $self = { 
    _logfile        => 'STDERR',
    _buffer         => [],
    _buffer_limit   => 200,
    _options        => undef,
};

=item setup_options()

This function is used to pass an instance of the Mud::Options object
into the Mud::Logger namespace, which allows things like log_debug()
to only log if the debug flag is enabled.

If the options data is not available, sensible defaults will be assumed.

=cut

sub setup_options {
    my $options = shift;

    die "Cannot initialize logging options without a valid Mud::Options object!" unless $options->isa('Mud::Options');
    $self->{_options} = $options;
}

=item buffer()

This function returns the contents of the log buffer.  Each log
entry is stored in a FIFO buffer, so that future system hooks can
choose to retrieve and process it, if so desired.

The size of this buffer is limited to avoid resource abuse.

If a level is specified as an argument, only entries of that level
will be returned.

=cut

sub buffer {
    my $level = shift;

    return grep { $_->{level} eq $level } @{ $self->{_buffer} } if defined $level;
    return $self->{_buffer};
}

=item clear_buffer()

This function empties the logging buffer.

If a level is specified as an argument, only entries of that level
will be removed.

=cut

sub clear_buffer {
    my $level = shift;

    $self->{_buffer} = [ grep { $_->{level} ne $level } @{ $self->{_buffer} } ] if defined $level;
    $self->{_buffer} = [] if !defined $level;
    return scalar @{ $self->{_buffer} };
}


=item buffer_limit()

This function allows you to adjust the buffer size of previous log
entries that are kept.

=cut

sub buffer_limit {
    my $new_size = shift;

    $self->{_buffer_limit} = $new_size if defined $new_size and $new_size =~ /\d+/;
    pop @{ $self->{_buffer} } until scalar @{ $self->{_buffer} } < $self->{_buffer_limit};
    return $self->{_buffer_limit};
}

=item logging()

The basic logging function.  This is not meant to
be called directly, but acts as the dispatcher for
all log functions.

=cut

sub logging {
    my $now = time;
    my $level = shift;
    my $fmt = shift;
    my $fraction = sprintf("%.03f", ($now - int $now));
    $fraction =~ s/\b0\./\./;
    my $timestamp = strftime("%Y-%m-%d %H:%M:%S", localtime($now)) . $fraction;
    $fmt =~ s/\%\^/\%\%\^/g;
    my $message = sprintf $fmt, @_;
    my $padlen = (length $timestamp) + (length $level) + 4;
    my $pad = " " x $padlen;
    my $debug_info = "";

    if ($level eq 'DEBUG') {
        my ($package, $filename, $line) = caller(1);
        my (undef, undef, undef, $subroutine) = caller(2);
        $debug_info = sprintf " (%s, line %d", $filename, $line;
        $debug_info .= sprintf ", in %s", $subroutine if defined $subroutine;
        $debug_info .= ")";
    }

    $message = join "\n$pad", (split /[\r\n]+/, $message) if $message =~ /\n/gsmix;
    print STDERR "$timestamp [$level] $message$debug_info\n";
    push @{ $self->{_buffer} }, { timestamp => $timestamp, level => $level, message => $message };
    pop @{ $self->{_buffer} } until scalar @{ $self->{_buffer} } < $self->{_buffer_limit};
}

=item log_boot()

This function is to log activities at boot-time.

=cut

sub log_boot {
    logging 'BOOT', @_;
}

=item log_info()

This function is to log general information that
an admin might want to review.

=cut

sub log_info {
    logging 'INFO', @_;
}

=item log_debug()

This function is like log_info, but is used for
debugging purposes.  It prints additional information
showing the program location.

=cut

sub log_debug {
    return if defined $self->{_options} and ! $self->{_options}->debug;
    logging 'DEBUG', @_;
}

=item log_warn()

This is used to log warnings, which are potential
errors or abnormal events that an admin should
address.

=cut

sub log_warn {
    logging 'WARN', @_;
}

=item log_error()

This function is to log errors which need to
be addressed, but which don't prevent the game
from continuing to function.

=cut

sub log_error {
    logging 'ERROR', @_;
}

=item log_fatal()

This is used to log fatal events which did, or will,
cause the game to stop functioning!

=cut

sub log_fatal {
    logging 'FATAL', @_;
}

=item switch_logfile()

This function switches STDOUT and STDERR to the given
logfile, which is also stored in the module's data.

Calling with the special name 'STDOUT' or 'STDERR'
will restore the original descriptors.

Calling with no arguments will return the current
setting.

=cut

sub switch_logfile {
    my $logfile = shift;

    return $self->{'_logfile'} if ! defined $logfile;

    if( $self->{'_logfile'} eq 'STDERR' ) {
        return $self->{'_logfile'} if $logfile =~ /^std(err|out)$/i;

        my ($olderr, $oldout);
        log_boot "Switching logging to %s... bye!", $logfile;
        open $olderr, ">&STDERR" or die "Cannot save original STDERR handle";
        open $oldout, ">&STDOUT" or die "Cannot save original STDOUT handle";
        open STDERR, ">>$logfile" or die "Cannot redirect STDERR to $logfile";
        open STDOUT, ">&STDERR" or die "Cannot dup STDOUT to STDERR";
        select STDERR; $| = 1;
        select STDOUT; $| = 1;
        log_boot "Logging switched to %s... hello!", $logfile;
        $self->{'_logfile'} = $logfile;
        $self->{'_olderr'} = $olderr;
        $self->{'_oldout'} = $oldout;
    } else {
        if( $logfile =~ /^std(err|out)$/i ) {
            log_boot "Switching logging to %s... bye!", 'STDERR';
            open STDERR, ">&", $self->{'_olderr'} or die "Cannot redirect STDERR to original STDERR";
            open STDOUT, ">&", $self->{'_oldout'} or die "Cannot redirect STDOUT to original STDOUT";
            select STDERR; $| = 1;
            select STDOUT; $| = 1;
            log_boot "Logging switched to %s... hello!", 'STDERR';
            $self->{'_logfile'} = 'STDERR';
        } else {
            log_boot "Switching logging to %s... bye!", $logfile;
            open STDERR, ">>$logfile" or die "Cannot redirect STDERR to $logfile";
            open STDOUT, ">&STDERR" or die "Cannot dup STDOUT to STDERR";
            select STDERR; $| = 1;
            select STDOUT; $| = 1;
            log_boot "Logging switched to %s... hello!", $logfile;
            $self->{'_logfile'} = $logfile;
        }
    }

    return $self->{'_logfile'};
}

=back

=cut

1;

