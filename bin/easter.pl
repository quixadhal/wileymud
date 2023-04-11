#!/usr/bin/perl -w

use strict;
use Date::Calc qw(Easter_Sunday Add_Delta_Days);

die "Wallpaper directory not found" unless -d "/home/www/log/gfx/wallpaper";
chdir "/home/www/log/gfx/wallpaper";
die "Easter not found!" unless -d "easter";

my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
$year += 1900;
$mon += 1;
my $last_year = $year - 1;

my ($last_easter_year, $last_easter_month, $last_easter_day) = Easter_Sunday($last_year);
foreach my $i (-3, -2, -1, 0, 1, 2, 3) {
    my ($holiday_year, $holiday_month, $holiday_day) = Add_Delta_Days($last_easter_year, $last_easter_month, $last_easter_day, $i);
    my $holiday = sprintf "%02d-%02d", $holiday_month, $holiday_day;
    if (-f $holiday ) {
        unlink "$holiday";
    }
}

my ($easter_year, $easter_month, $easter_day) = Easter_Sunday($year);

foreach my $i (-3, -2, -1, 0, 1, 2, 3) {
    my ($holiday_year, $holiday_month, $holiday_day) = Add_Delta_Days($year, $easter_month, $easter_day, $i);
    my $holiday = sprintf "%02d-%02d", $holiday_month, $holiday_day;
    system("/usr/bin/ln -svf -T easter $holiday");
}

