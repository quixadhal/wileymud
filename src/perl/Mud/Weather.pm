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

use Time::HiRes qw( time sleep alarm );
use JSON;

use Mud::Logger;

use Exporter qw(import);
our @EXPORT_OK = ();

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

my $filename = 'weather.dat';

# These get printed when the month changes.
my @month_messages = (
    "It is bitterly cold outside.",
    "It is bitterly cold outside.",
    "It is bitterly cold outside.",
    "It is very cold.",
    "It is chilly outside.",
    "The flowers start to bloom.",
    "It is warm and pleasent.",
    "It is very warm.",
    "It is hot and humid.",
    "It is hot and humid.",
    "It is warm and humid.",
    "It starts to get alittle windy.",
    "The air is getting chilly.",
    "The leaves start to change colors.",
    "It starts to get cold.",
    "It is very cold.",
    "It is bitterly cold outside.",
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
        my $data = '';
        open FP, "<$filename" or die "Cannot open $filename: $!";
        while(<FP>) {
            $data .= $_;
        }
        close FP;
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

    log_info $self->time_of_day_message if $self->{_time_daemon}->is_new_time_of_day;
    log_info $self->month_message if $self->{_time_daemon}->is_new_month;
}

=back

=cut

1;

