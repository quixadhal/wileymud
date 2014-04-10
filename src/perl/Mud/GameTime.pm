#!/usr/bin/perl -w

package Mud::GameTime;

=head1 NAME

Mud::GameTime - Handles events related to game time.

=head1 OVERVIEW

This package keeps track of the current game time and
provides functions to manipulate it.

=head1 FUNCTIONS

=over 8

=cut

use strict;
use warnings;
use English;
use Data::Dumper;

use Time::HiRes qw( time sleep alarm );
use Lingua::EN::Numbers::Ordinate;

use Mud::Logger qw(log_boot log_info log_warn log_error log_fatal);

use Exporter qw(import);
our @EXPORT_OK = ();

my $beginning_of_time = 650336715; # Fri Aug 10 21:05:15 1990

my $secs_per_mud_hour  	= 75;
my $hours_per_mud_day	= 24;
my $days_per_mud_week   = 7;
my $days_per_mud_month	= 35;
my $months_per_mud_year	= 17;
my $secs_per_mud_day    = $hours_per_mud_day * $secs_per_mud_hour;
my $secs_per_mud_week   = $days_per_mud_week * $secs_per_mud_day;
my $secs_per_mud_month  = $days_per_mud_month * $secs_per_mud_day;
my $secs_per_mud_year   = $months_per_mud_year * $secs_per_mud_month;

#define SECS_PER_REAL_MIN  60
#define SECS_PER_REAL_HOUR (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY  (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR (365*SECS_PER_REAL_DAY)

my @month_names = (
    'Winter', 'the Winter Wiley Wolf', 'the Frost Giant', 'the Old Forces',
    'the Grand Struggle', 'Spring', 'Nature',
    'Futility', 'the Dragon', 'the Sun', 'Heat', 'the Battle',
    'Dark Shades', 'the Shadows',
    'the Long Shadows', 'the Ancient Darkness', 'the Great Evil',
);

my @weekday_names = (
    'the Moon', 'the Bull', 'the Deception', 'Thunder', 'Freedom', 'the Great Gods', 'the Sun',
);

my @moon_phase_names = (
    "new", "new", "new", "new",
    "waxing crescent", "waxing crescent", "waxing crescent", "waxing crescent",
    "waxing half", "waxing half", "waxing half", "waxing half",
    "waxing gibbus", "waxing gibbus", "waxing gibbus", "waxing gibbus",
    "full", "full", "full", "full",
    "waning gibbus", "waning gibbus", "waning gibbus", "waning gibbus",
    "waning half", "waning half", "waning half", "waning half",
    "waning crescent", "waning crescent", "waning crescent", "waning crescent",
);

my @season_names = (
    "winter", "winter", "winter", "winter",
    "spring", "spring", "spring",
    "summer", "summer", "summer", "summer", "summer",
    "fall", "fall",
    "winter", "winter", "winter",
);

my %time_of_day_names = (
    spring  => [
        "midnight",
        "the small hours",
        "the small hours",
        "the small hours",
        "the small hours",
        "the small hours",
        "dawn", #6am
        "morning",
        "morning",
        "morning",
        "morning",
        "morning",
        "noon",
        "afternoon",
        "afternoon",
        "afternoon",
        "afternoon",
        "afternoon",
        "evening", # 6pm
        "evening",
        "dusk", # 8pm
        "night",
        "night",
        "night",
    ],
    summer  => [
        "midnight",
        "the small hours",
        "the small hours",
        "the small hours",
        "the small hours",
        "dawn", # 5am
        "morning",
        "morning",
        "morning",
        "morning",
        "morning",
        "morning",
        "noon",
        "afternoon",
        "afternoon",
        "afternoon",
        "afternoon",
        "afternoon",
        "afternoon",
        "evening", # 7pm
        "evening",
        "dusk", # 9pm
        "night",
        "night",
    ],
    fall    => [
        "midnight",
        "the small hours",
        "the small hours",
        "the small hours",
        "the small hours",
        "the small hours",
        "dawn", # 6am
        "morning",
        "morning",
        "morning",
        "morning",
        "morning",
        "noon",
        "afternoon",
        "afternoon",
        "afternoon",
        "afternoon",
        "afternoon",
        "evening", # 6pm
        "evening",
        "dusk", # 8pm
        "night",
        "night",
        "night",
    ],
    winter  => [
        "midnight",
        "the small hours",
        "the small hours",
        "the small hours",
        "the small hours",
        "the small hours",
        "the small hours",
        "dawn", # 7am
        "morning",
        "morning",
        "morning",
        "morning",
        "noon",
        "afternoon",
        "afternoon",
        "afternoon",
        "evening", # 4pm
        "evening",
        "dusk", # 6pm
        "night",
        "night",
        "night",
        "night",
        "night",
    ],
);

=back

=head1 METHODS

=over 8

=cut

=item new()

Constructor.  WHen we create a new instance, we check for
a way to restore previous values, otherwise we use defaults.

=cut

sub new {
    my $class = shift;
    my $self = { 
        _last_updated   => undef,
        _current_time   => time,
        _hour           => 0,
        _day            => 0,
        _month          => 0,
        _year           => 0,
    };

    #log_info("   Current Gametime: %dH %dD %dM %dY.", time_info.hours, time_info.day, time_info.month, time_info.year);

    log_boot "- Resetting game time";
    bless $self, $class;
    $self->update();
    log_boot $self->game_time;
    log_boot "There is a %s moon in the sky.", $self->moon_phase;

    return $self;
}

=item DESTROY()

A special function called when the weather object is destroyed.
Normally this will happen during shutdown.  This code should save
the weather state back to disk.

=cut

sub DESTROY {
    my $self = shift;

    log_warn "- Time data should be saved here";
}

=back

=item current_time()

This returns the current time, as seen by the game driver.  It is usually
the value of time() as it was at the very top of the processing loop.  We
use that, rather than actual real-time, so every combat round and effect
is consistent for the full processing stage.

Passing an argument sets the time to that value and should be done at
the top of each processing loop.

=cut

sub current_time {
    my ($self, $setting) = @_;
    $self->{_current_time} = $setting if defined $setting;
    return $self->{_current_time};
}

=item hour()

This returns the current hour of the game clock, in game time.

This is often used by game mechanics to determine lighting or
NPC behavior patterns.

Setting the hour by hand may cause issues, and may be auto-corrected
at the top of the next processing loop, as game-time is normally
derived as a function of real-time.

=cut

sub hour {
    my ($self, $setting) = @_;
    $self->{_hour} = $setting if defined $setting;
    return $self->{_hour};
}

=item day()

This returns the current day of the game clock, in game time.

Setting the day by hand may cause issues, and may be auto-corrected
at the top of the next processing loop, as game-time is normally
derived as a function of real-time.

=cut

sub day {
    my ($self, $setting) = @_;
    $self->{_day} = $setting if defined $setting;
    return $self->{_day};
}

=item month()

This returns the current month of the game clock, in game time.

This is often used by game mechanics to determine the season.

Setting the month by hand may cause issues, and may be auto-corrected
at the top of the next processing loop, as game-time is normally
derived as a function of real-time.

=cut

sub month {
    my ($self, $setting) = @_;
    $self->{_month} = $setting if defined $setting;
    return $self->{_month};
}

=item year()

This returns the current year of the game clock, in game time.

Setting the year by hand may cause issues, and may be auto-corrected
at the top of the next processing loop, as game-time is normally
derived as a function of real-time.

=cut

sub year {
    my ($self, $setting) = @_;
    $self->{_year} = $setting if defined $setting;
    return $self->{_year};
}


=item mud_time_passed()

This method returns the amount of MUD game time
passed between two unix timestamps (real-time).

If the first argument is omitted, the current time
is used.  This is not the real-time value of time(),
but the value used by the game driver, usually
refreshed at the top of each processing loop.

If the second argument is omitted, the
special "beginning of time" symbol is used, which
was the date specified by the game's original
creators as the start of time.

=cut

sub mud_time_passed {
    my $self = shift;
    my $t2 = shift || $self->{_current_time};
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

    return {
        hours   => $hours,
        days    => $days,
        months  => $months,
        years   => $years,
    };
}

=item update()

This method should be called every time we're at the
top of the main processing loop.

It ensures that all time values are up-to-date with
the wall clock, giving us a synchronized set of values
to use for the entire processing loop.

As an optimization, we only recalculate values every
"mud hour", which is every 75 real-time seconds by
default, since none of the values will change more
frequently.

=cut

sub update {
    my $self = shift;
    my $now = shift || time;

    my $raw_hour = int $now / $secs_per_mud_hour;
    return if defined $self->{_last_updated} and $raw_hour <= $self->{_last_updated};

    my $diff = $self->mud_time_passed($now);
    $self->{_last_updated} = $raw_hour;
    $self->{_current_time} = $now;
    $self->{_hour} = $diff->{hours};
    $self->{_day} = $diff->{days};
    $self->{_month} = $diff->{months};
    $self->{_year} = $diff->{years};
}

=item weekday()

This method returns the name of the day of the week,
in game terms.  It can be used to control NPC behavior
where certain days have different schedules.

=cut

sub weekday {
    my $self = shift;

    my $weekday = $self->{_day} % $days_per_mud_week;
    return $weekday_names[$weekday];
}

=item month_name()

This method returns the name of the current game month.
It is intended for cosmetic use.

=cut

sub month_name {
    my $self = shift;

    return $month_names[$self->{_month}];
}

=item moon_phase()

This method returns the current phase of the moon.
The phase is a text string that describes how the moon
is currently displayed and can be used to adjust spell
powers, clerical abilities, or anything else.

=cut

sub moon_phase {
    my $self = shift;

    my $raw_day = int $self->{_current_time} / $secs_per_mud_day;
    my $phase = $raw_day % scalar @moon_phase_names;
    return $moon_phase_names[$phase];
}

=item season()

This method returns the name of the current season, which
is derived from the current month of the game year.

=cut

sub season {
    my $self = shift;

    return $season_names[$self->{_month}];
}

=item time_of_day()

This method returns the current time of the game day,
as a text string which describes the period of daytime
or nighttime it is.  This can be used to adjust NPC
behavior.

=cut

sub time_of_day {
    my $self = shift;

    return $time_of_day_names{$self->season}->[$self->{_hour}];
}

=cut

=item game_time()

A method to return a pretty textual display of the
current game time.

=cut

sub game_time {
    my $self = shift;

    my $tod = $self->time_of_day;
    my $weekday = $self->weekday;
    my $ordinal = ordinate($self->{_day} + 1);
    my $month_name = $self->month_name;
    my $year = $self->year;

    return "It is $tod on the day of $weekday, the $ordinal day of $month_name, in the year $year.";
}

1;

