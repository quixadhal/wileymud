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
use English -no_match_vars;
use Data::Dumper;

use JSON;
use List::Util qw(min max);

use Mud::Logger;
use Mud::Utils;

use Exporter qw(import);
our @EXPORT_OK = ();
our @EXPORT = ();
our %EXPORT_TAGS = (all => [ @EXPORT, @EXPORT_OK ]);

my @sky_conditions = (
    'clear',
    'cloudy',
    'raining',
    'stormy',
);

my @wind_directions = (
    'north',
    'east',
    'south',
    'west',
    'calm',
);

my $filename = 'weather.json';

# These get printed when the month changes.
my @month_messages = (
    'It is bitterly cold outside.',
    'It is bitterly cold outside.',
    'It is bitterly cold outside.',
    'It is very cold.',
    'It is chilly outside.',
    'The flowers start to bloom.',
    'It is warm and pleasent.',
    'It is very warm.',
    'It is hot and humid.',
    'It is hot and humid.',
    'It is warm and humid.',
    'It starts to get alittle windy.',
    'The air is getting chilly.',
    'The leaves start to change colors.',
    'It starts to get cold.',
    'It is very cold.',
    'It is bitterly cold outside.',
);

# These get printed when the weather shifts.
my %weather_messages = (
    'spring' => [
        '',
        'The sky is getting cloudy.',
        'It starts to rain.',
        'The clouds disappear.',
        'You are caught in a lightning storm!',
        'The rain has stopped.',
        'The storm seems to calm, but it is still raining.',
    ],
    'summer' => [
        '',
        'The sky is getting cloudy.',
        'It starts to rain.',
        'The clouds disappear.',
        'You are caught in a lightning storm!',
        'The rain has stopped.',
        'The storm seems to calm, but it is still raining.',
    ],
    'fall' => [
        '',
        'The sky is getting cloudy.',
        'It starts to rain.',
        'The clouds disappear.',
        'You are caught in a lightning storm!',
        'The rain has stopped.',
        'The storm seems to calm, but it is still raining.',
    ],
    'winter' => [
        '',
        'The sky is getting cloudy.',
        'It starts to drizzle.',
        'The clouds disappear.',
        'You are in a layer of dense fog.',
        'The drizzle has stopped.',
        'The fog thins and lifts away, but it continues to drizzle.',
    ],
);

=item new()

Constructor.  WHen we create a new instance, we check for
a way to restore previous values, otherwise we use defaults.

The weather daemon relies on values from the centralized
time daemon.  This should be passed into the constructor.

=cut

sub new {
    my $class = shift;
    my $time_daemon = shift;
    my $self = {};

    die "Cannot initialize weather system without a valid time system!" unless $time_daemon->isa('Mud::GameTime');
    log_boot "- Resetting game weather";

    if( -r $filename ) {
        my $data = load_file $filename;
        $self = decode_json $data;
        $self->{_time_daemon} = $time_daemon;
    } else {
        $self = { 
            _time_daemon    => $time_daemon,
            _pressure       => 960,
            _change         => 0,
            _sky            => 'clear',
            _wind_speed     => 0,
            _wind_direction => 'calm',
            _category_shift => 0,
        };
    }

    bless $self, $class;
    $self->update();
    return $self;
}

=item TO_JSON()

This method is called by the JSON package to convert a blessed reference
into a normal hashref that can be encoded directly with JSON.  We take
advantage of this to omit fields we don't want to save.

=cut

sub TO_JSON {
    my $self = shift;

    my $obj = {};
    $obj->{$_} = $self->{$_} foreach grep { ! /^(_time_daemon)$/ } (keys %$self);
    return $obj;
}

=item DESTROY()

A special function called when the weather object is destroyed.
Normally this will happen during shutdown.  This code should save
the weather state back to disk.

=cut

sub DESTROY {
    my $self = shift;

    log_info "Saving weather data to $filename";
    my $json = JSON->new->allow_blessed(1)->convert_blessed(1);
    my $data = $json->encode($self) or die "Invalid JSON conversion: $!";
    open FP, ">$filename" or die "Cannot open $filename: $!";
    print FP "$data\n";
    close FP;
    log_warn "Weather daemon shut down.";
}

=item time_of_day_message()

This method returns a string describing the current time of day.
It's used during the time updating, so that when the hour changes
from one category to another, you get a nice message if you're
outside to hint what time of day it is.

=cut

sub time_of_day_message {
    my $self = shift;

    my %time_of_day_messages = (
        'midnight'          => sub { "The moon rises far overhead." },
        'the small hours'   => sub { "The stars are clear and bright." },
        'false dawn'        => sub { "The moon sets." },
        'dawn'              => sub { "The sun rises in the east." },
        'early morning'     => sub { "The day has begun." },
        'morning'           => sub { "The sun shines brightly." },
        'noon'              => sub { "It is noon." },
        'afternoon'         => sub { "The sun shines brightly." },
        'late afternoon'    => sub { "The shadows begin to lengthen." },
        'dusk'              => sub { "The sun slowly disappears in the west." },
        'evening'           => sub { "The night has begun." },
        'night'             => sub { sprintf("A %s moon rises in the eastern sky.", $self->{_time_daemon}->moon_phase) },
    );

    return $time_of_day_messages{$self->{_time_daemon}->time_of_day}();
}

=item month_message()

This method returns a string describing the feel of the new month.
It should be emitted to anyone outdoors at the time the month rolls
over as flavor text.  It might be useful to emit this when a player
first logs in as well, or perhaps when they first step outside after
logging in.

=cut

sub month_message {
    my $self = shift;

    return $month_messages[$self->{_time_daemon}->month];
}

=item update()

This method should be called regularly, near the top of the main
processing loop.  It will calculate changes to the global weather
system and emit messages to anyone outdoors about those changes.

It should also print messages about the time of day, and the new
month changes, to help players recognize the passage of time.

=cut

sub update {
    my $self = shift;

    $self->change_weather;

    log_info "WEATHER_D: %s", $self->time_of_day_message if $self->{_time_daemon}->is_new_time_of_day;
    log_info "WEATHER_D: %s", $self->month_message if $self->{_time_daemon}->is_new_month;
    log_info "WEATHER_D: %s", $self->change_message if $self->has_weather_changed;
}

=item change_weather()

This method randomly adjusts the global weather conditions to (poorly)
simulate weather conditions.

=cut

sub change_weather {
    my $self = shift;

    my $diff = 0;
    my $change = 0;
    my $season = $self->{_time_daemon}->season;
    my $tod = $self->{_time_daemon}->time_of_day;

    $self->{_category_shift} = 0;

    if ($season eq 'summer' or $season eq 'fall') {
        $diff = $self->{_pressure} > 985 ? -2 : 2;
    } else {
        $diff = $self->{_pressure} > 1015 ? -2 : 2;
    }

    $self->{_change} += dice('1d4') * $diff + dice('2d6') - dice('2d6');
    $self->{_change} = min $self->{_change}, 12;
    $self->{_change} = max $self->{_change}, -12;
    $self->{_pressure} += $self->{_change};
    $self->{_pressure} = min $self->{_pressure}, 1040;
    $self->{_pressure} = max $self->{_pressure}, 960;

    if( $self->{_sky} eq 'clear' ) {
        $self->{_category_shift} = 1 if $self->{_pressure} < 1010 and dice('1d4') == 1;
        $self->{_category_shift} = 1 if $self->{_pressure} < 990;
    } elsif( $self->{_sky} eq 'cloudy' ) {
        $self->{_category_shift} = 3 if $self->{_pressure} < 1030 and dice('1d4') == 1;
        $self->{_category_shift} = 2 if $self->{_pressure} < 990 and dice('1d4') == 1;
        $self->{_category_shift} = 2 if $self->{_pressure} < 970;
    } elsif( $self->{_sky} eq 'raining' ) {
        $self->{_category_shift} = 4 if $self->{_pressure} < 970 and dice('1d4') == 1;
        $self->{_category_shift} = 5 if $self->{_pressure} > 1010 and dice('1d4') == 1;
        $self->{_category_shift} = 5 if $self->{_pressure} > 1030;
    } elsif( $self->{_sky} eq 'stormy' ) {
        $self->{_category_shift} = 6 if $self->{_pressure} > 990 and dice('1d4') == 1;
        $self->{_category_shift} = 6 if $self->{_pressure} > 1010;
    } else {
        $self->{_category_shift} = 0;
        $self->{_sky} = 'clear';
    }

    $self->{_category_shift} = min $self->{_category_shift}, 6;
    $self->{_category_shift} = max $self->{_category_shift}, 0;

    if ($self->{_category_shift} == 1) {
        $self->{_sky} = 'cloudy';
    } elsif ($self->{_category_shift} == 2) {
        $self->{_sky} = 'raining';
    } elsif ($self->{_category_shift} == 3) {
        $self->{_sky} = 'clear';
    } elsif ($self->{_category_shift} == 4) {
        $self->{_sky} = 'stormy';
    } elsif ($self->{_category_shift} == 5) {
        $self->{_sky} = 'cloudy';
    } elsif ($self->{_category_shift} == 6) {
        $self->{_sky} = 'raining';
    }

    log_debug "season: %s", $season;
    log_debug "tod: %s", $tod;
    log_debug "diff: %d", $diff;
    log_debug "change: %d", $self->{_change};
    log_debug "pressure: %d", $self->{_pressure};
    log_debug "category_shift: %d", $self->{_category_shift};
    log_debug "sky: %s", $self->{_sky};
}

=item change_message()

This method figures out the appropriate message to inform the user
of the weather shifting.  In the change_weather method, the weather
system details are manipulated and an overall shift is determined.

=cut

sub change_message {
    my $self = shift;

    return undef if ! defined $self->{_category_shift} or $self->{_category_shift} < 1 or $self->{_category_shift} > 6;
    return $weather_messages{$self->{_time_daemon}->season}->[$self->{_category_shift}];
}

=item has_weather_changed()

A tiny convenience method to show that the last update caused
the weather to shift... cosmetic.

=cut

sub has_weather_changed {
    my $self = shift;

    return $self->{_category_shift} > 0 ? 1 : undef;
}

=back

=cut

1;

