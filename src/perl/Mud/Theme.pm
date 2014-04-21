#!/usr/bin/perl -w

package Mud::Theme;

=head1 NAME

Mud::Theme - Module to provide customizable task-based color

=head1 OVERVIEW

This packge extends the Mud::Terminal package by providing
custom task-based tokens that give builders a way to use
color codes for specific purposes, rather than hard coding
the colors themselves.

The user should be able to select a theme, which is a set
of token mapping to normal pinkfish tokens.

=head1 FUNCTIONS

=over 8

=cut

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use Mud::Logger;
use Mud::Utils;
use Mud::ColorThemes qw( %theme_map @theme_tokens @themes );

use Exporter qw(import);
our @EXPORT_OK = qw();
our @EXPORT = qw();
our %EXPORT_TAGS = (all => [ @EXPORT, @EXPORT_OK ]);

=item new()

Constructor.  Initializes the symbolic color subsystem.

If passed a theme name, initialize the system to that theme,
otherwise use the default.

If passed a Mud::Terminal object, use that to obtain the
final code sequence.  Otherwise, return the pinkfish code
itself, which can then be further processed.

=cut

sub new {
    my $class = shift;
    my $theme = shift;
    my $terminal = shift;

    my $self = { 
        _theme      => "default",
        _terminal   => undef,
    };

    bless $self, $class;
    $self->theme( $theme ) if defined $theme;
    $self->{_terminal} = $terminal if defined $terminal and $terminal->isa('Mud::Terminal');
    log_info "THEME_D: %s", $self->theme;
    log_info "THEME_D: Terminal is %s", $self->{_terminal}->terminal_type if defined $self->{_terminal};
    return $self;
}

=item valid_theme()

This method takes a string and returns true if
that string is a valid theme name.

It returns undef otherwise.

=cut

sub valid_theme {
    my $self = shift;
    my $theme = shift;

    return undef if !defined $theme;
    return grep { "$_" eq "$theme" } @themes;
}

=item theme()

This method allows you to set the theme used
by an instance of the Mud::Theme class.

If called without an argument, it returns the current
setting.

=cut

sub theme {
    my ($self, $setting) = @_;

    if (defined $setting) {
        $setting = lc trim $setting;
        $self->{_theme} = $setting if $self->valid_theme( $setting );
    }
    return $self->{_theme};
}

=item valid_token()

This method accepts a symbolic color name and returns true
if that name is a valid token for the current theme,
or false otherwise.

=cut

sub valid_token {
    my $self = shift;
    my $token = shift;

    return undef if !defined $token;
    return 1 if exists $theme_map{$self->theme}->{$token};
    return undef;
}

=item convert_token()

This method accepts a symbolic color name and returns the converted
value, if a valid conversion exists for the current theme,
or the original string if no such conversion exists.

=cut

sub convert_token {
    my $self = shift;
    my $token = shift;

    return undef if !defined $token;
    return $token if length $token < 1;

    my $theme = $self->theme;
    return $token unless exists $theme_map{$theme}->{$token};
    return $theme_map{$theme}->{$token};
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
        $bits[$i] = $self->{_terminal}->colorize( $bits[$i] ) if defined $self->{_terminal};
    }
    return join '', @bits;
}

=back

=cut

1;

