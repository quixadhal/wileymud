#!/usr/bin/perl -w

package Mud::ColorThemes;

=head1 NAME

Mud::ColorThemes - The raw color theme mappings, used by Mud::Theme

=head1 OVERVIEW

This is the list of theme maps, which convert symbolic tokens
like %^ROOM_DESC%^ into actual color tokens, such as %^RED%^.

=head1 FUNCTIONS

=over 8

=cut

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use Exporter qw(import);
our @EXPORT_OK = qw( %theme_map @theme_tokens @themes );
our @EXPORT = qw();
our %EXPORT_TAGS = (all => [ @EXPORT, @EXPORT_OK ]);

our %theme_map = ();

our @themes = sort(qw(
    default
));

our @theme_tokens = sort(qw(
    DAMAGE HEALING POISON
    ROOM_TITLE ROOM_DESC
));

$theme_map{$_} = {} foreach @themes;

$theme_map{default} = {
    DAMAGE              => "%^RED%^",
    HEALING             => "%^LIGHTGREEN%^",
    POISON              => "%^GREEN%^",
    ROOM_TITLE          => "%^CYAN%^",
    ROOM_DESC           => "",
};

=back

=cut

1;

