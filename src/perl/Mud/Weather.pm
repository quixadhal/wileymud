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
our @EXPORT_OK = qw(mud_time_passed);

my $beginning_of_time = 650336715; # Fri Aug 10 21:05:15 1990

my $secs_per_mud_hour  	= 75;
my $hours_per_mud_day	= 24;
my $days_per_mud_month	= 35;
my $months_per_mud_year	= 17;
my $secs_per_mud_day    = $hours_per_mud_day * $secs_per_mud_hour;
my $secs_per_mud_month  = $days_per_mud_month * $secs_per_mud_day;
my $secs_per_mud_year   = $months_per_mud_year * $secs_per_mud_month;

#define SECS_PER_REAL_MIN  60
#define SECS_PER_REAL_HOUR (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY  (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR (365*SECS_PER_REAL_DAY)

=item new()

Constructor.  WHen we create a new instance, we check for
a way to restore previous values, otherwise we use defaults.

=cut

sub new {
    my $class = shift;
    my $self = { 
        _gametime => {
            current_time    => $beginning_of_time,
            hours           => 0,
            days            => 0,
            months          => 0,
            years           => 0,
        },
        _weather => {
            pressure        => 0,
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

=item mud_time_passed()

This function returns the amount of MUD game time
passed between two unix timestamps (real-time).

If the first argument is omitted, the current time
is used.  If the second argument is omitted, the
special "beginning of time" symbol is used, which
was the date specified by the game's original
creators as the start of time.

=cut

sub mud_time_passed {
    my $t2 = shift || time;
    my $t1 = shift || $beginning_of_time;

    if ($t2 < $t1) {    # Oops, passed them in backwards!
        my $tmp = $t1;
        $t1 = $t2;
        $t2 = $tmp;
    }
    my $diff = (int $t2) - (int $t1);
    my $hours = int ($diff / $secs_per_mud_hour) % $hours_per_mud_day;
    $diff -= ($secs_per_mud_hour * $hours);
    my $days = int ($diff / $secs_per_mud_day) % $days_per_mud_month;
    $diff -= ($secs_per_mud_day * $days);
    my $months = int ($diff / $secs_per_mud_month) % $months_per_mud_year;
    $diff -= ($secs_per_mud_month * $months);
    my $years = int ($diff / $secs_per_mud_year);

    return (
        hours           => $hours,
        days            => $days,
        months          => $months,
        years           => $years,
    );
}

=back

=cut

1;

