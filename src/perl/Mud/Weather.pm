#!/usr/bin/perl -w

package Mud::Weather;

=head1 NAME

Mud::Weather - Handles events related to time and weather.

=head1 OVERVIEW

This package provides functions to deal with the game's
weather systems, as well as time related functions.

=head1 FUNCTIONS

=over 8

=cut

use strict;
use warnings;
use English;
use Data::Dumper;

use Mud::Logger qw(log_boot log_info log_warn log_error log_fatal);

use Exporter qw(import);
our @EXPORT_OK = ();

=item new()

Constructor.  WHen we create a new instance, we check for
a way to restore previous values, otherwise we use defaults.

=cut

sub new {
    my $class = shift;
    my $self = { 
        _weather => {
            pressure        => 960,
            change          => 0,
            sky             => 0,
            sunlight        => 0,
            wind_speed      => 0,
            wind_direction  => 0,
            moon            => 0,
        },
    };

    log_boot "- Resetting game time and weather";
    # do stuff
    # weather_and_time(1);
    #log_info("   Current Gametime: %dH %dD %dM %dY.", time_info.hours, time_info.day, time_info.month, time_info.year);

    bless $self, $class;
    return $self;
}

=item DESTROY()

A special function called when the weather object is destroyed.
Normally this will happen during shutdown.  This code should save
the weather state back to disk.

=cut

sub DESTROY {
    log_warn "- Weather data should be saved here";
}

=back

=cut

1;

