#!/usr/bin/perl -w

package Mud::Terminal;

=head1 NAME

Mud::Terminal - The color symbol translating subsystem

=head1 OVERVIEW

This package takes text that has embedded color codes,
implemented in the Pinkfish style of a color keyword
surrounded by '%^' tokens, and translates them to an
output format, which is typically ANSI escape codes,
but can be symbolic, or even the empty string.

=head1 FUNCTIONS

=over 8

=cut

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use POSIX qw(floor);
use Scalar::Util qw(looks_like_number);

use Mud::Logger;
use Mud::Utils;
use Mud::ColorCodes qw( %color_map @terminal_types @color_tokens );

use Exporter qw(import);
our @EXPORT_OK = qw();
our @EXPORT = qw();
our %EXPORT_TAGS = (all => [ @EXPORT, @EXPORT_OK ]);

=item new()

Constructor.  Initializes the color subsystem.

=cut

sub new {
    my $class = shift;
    my $ttype = shift;

    my $self = { 
        _terminal_type  => "unknown",
    };

    bless $self, $class;
    $self->terminal_type( $ttype ) if defined $ttype;
    log_info "TERMINAL_D: %s", $self->terminal_type;
    return $self;
}

=item valid_terminal()

This method takes a string and returns true if
that string is considered a valid terminal type.

It returns undef otherwise.

=cut

sub valid_terminal {
    my $self = shift;
    my $ttype = shift;

    return undef if !defined $ttype;
    return grep { "$_" eq "$ttype" } @terminal_types;
}

=item terminal_type()

This method allows you to set the terminal type used
by an instance of the Mud::Terminal class.  Usually, this
is only used when the instance is initialized, but if
nothing was passed in to the constructor, or if the
terminal type was unknown until later (perhaps via
TELNET negotiaton, or a change in the user's settings?)
this can change the color system's behavior for that
instance.

If called without an argument, it returns the current
setting.

=cut

sub terminal_type {
    my ($self, $setting) = @_;

    if (defined $setting) {
        $setting = lc trim $setting;
        $setting = 'xterm-256color' if $setting =~ /^xterm([_-])?256(colou?r)?$/;
        $self->{_terminal_type} = $setting if $self->valid_terminal( $setting );
    }
    return $self->{_terminal_type};
}

=item valid_token()

This method accepts a color token name and returns true
if that name is a valid token for the current terminal type,
or false otherwise.

=cut

sub valid_token {
    my $self = shift;
    my $token = shift;

    return undef if !defined $token;
    return 1 if exists $color_map{$self->terminal_type}->{$token};
    return undef;
}

=item convert_token()

This method accepts a color token name and returns the converted
value, if a valid conversion exists for the current terminal type,
or the original string if no such conversion exists.

=cut

sub convert_token {
    my $self = shift;
    my $token = shift;

    return undef if !defined $token;
    return $token if length $token < 1;

    my $ttype = $self->terminal_type;
    return $token unless exists $color_map{$ttype}->{$token};
    return $color_map{$ttype}->{$token};
}


=item colorize()

This function takes a string with embedded color tokens and
returns a string where those tokens have been replaced with
the appropriate characer sequences for the selected terminal type.

=cut

sub colorize {
    my $self = shift;
    my $msg = shift;

    return undef if !defined $msg;
    return $msg if length $msg < 1;

    my @bits = split /\%\^/, $msg;
    for (my $i = 0; $i < scalar @bits; $i++) {
        next if !defined $bits[$i];
        next if $bits[$i] eq '';
        $bits[$i] = $self->convert_token( $bits[$i] );
    }
    return join '', @bits;
}

=back

=cut

1;

