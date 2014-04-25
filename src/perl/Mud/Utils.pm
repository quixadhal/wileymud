#!/usr/bin/perl -w

package Mud::Utils;

=head1 NAME

Mud::Utils - A random collection of utility functions

=head1 OVERVIEW

This is just a place to put globaly useful functions
that get called from lots of different modules, but
don't rely on anything in them.

=head1 FUNCTIONS

=over 8

=cut

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use POSIX qw(floor);
use Scalar::Util qw(looks_like_number);

use Exporter qw(import);
our @EXPORT_OK = qw();
our @EXPORT = qw(dice load_file trim lines );
our %EXPORT_TAGS = (all => [ @EXPORT, @EXPORT_OK ]);

=item dice()

This function is a simple dice roller.

It can be passed arguments in one of two ways.  Either a single
string of the form "xdy+z", or as three integer arguments, being
(x, y, z).

In either case, this returns a random number generated as if
you had rolled a y sided die x times, and then added z to it.

=cut

sub dice {
    my $x = shift;
    my $y = shift;
    my $z = shift;

    if (defined $x) {
        $x =~ /(\d+)?d(\d+)([+-]\d+)?/i;
        ($x, $y, $z) = ($1, $2, $3);
        $x = 1 if !defined $x and defined $y; # allows us to use 'd6' instead of '1d6'.
    }
    $z = 0 if !defined $z or !looks_like_number $z;
    return $z if !defined $x or !defined $y or !looks_like_number $x or !looks_like_number $y or $x <= 0 or $y <= 0;
    my $sum = $z;
    for (my $i = 0; $i < $x; $i++) {
        my $r = (floor rand $y) + 1;
        $sum += $r;
    }
    return $sum;
}

=item load_file()

A simple routine to read in a text file from disk and
return the result as a string.  This is trivial, but it's
more convenient to use 1 line of code elsewhere than 3 or 4.

=cut

sub load_file {
    my $filename = shift;

    return undef if !defined $filename or !-r $filename;
    my $data = '';
    open FP, "<$filename" or die "Cannot open $filename: $!";
    while(<FP>) {
        $data .= $_;
    }
    close FP;
    return $data;
}

=item trim()

A function that removes leading or trailing whitespace from
a given string.

=cut

sub trim {
    my $msg = shift;

    return undef if !defined $msg;
    $msg =~ s/^\s+//;
    $msg =~ s/\s+$//;
    return $msg;
}

=item lines()

This function takes a string and returns an array of lines.
It removes any line endings, no matter what combination of CR
and LF they might be.  The return is an array ref, for ease
of handling.

=cut

sub lines {
    my $msg = shift;

    return undef if !defined $msg;
    return $msg if length $msg < 1;
    return [ split /\n|\r|\r\n|\n\r/, $msg ];
    #my @stuff = split /\n|\r|\r\n|\n\r/, $msg, -1;
    #pop @stuff if length $stuff[-1] < 1;
    # The pop is to get rid of the single extra delimiter
    # that appears when the last line ends in a proper line ending.
    # split /\n/, "a\nb\n" yields ("a", "b", "") which is
    # correct, but not what one expects ("a", "b").
    #return @stuff;
}

=back

=cut

1;

