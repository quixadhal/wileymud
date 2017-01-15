#!/usr/bin/perl -w

package Mud::DB::Option;

=head1 NAME

Mud::DB::Option - Option storage class.

=head1 OVERVIEW

This class maps various game options to a database options table
to ensure the options we pick are persisted between runs.

Command line options should override these.

=cut

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use Mud::DB::Object;
use base 'Mud::DB::Object';

=head1 SQL

It should be noted that this is really a single row table, with
a column for each option.  While a key/value setup would be more
natural for the database, being able to instantiate a single object
and just query or set the options is far more useful.

        CREATE TABLE IF NOT EXISTS options(
            id          TEXT NOT NULL PRIMARY KEY DEFAULT 'live',
            wizlock     BOOLEAN NOT NULL DEFAULT false,
            debug       BOOLEAN NOT NULL DEFAULT false,
            logfile     TEXT,
            pidfile     TEXT,
            libdir      TEXT DEFAULT './lib',
            specials    BOOLEAN NOT NULL DEFAULT true,
            gameport    INTEGER NOT NULL DEFAULT 7000
        );

=cut

__PACKAGE__->meta->setup(
    table       => 'options',
    columns     => [ 
        id      => {
            type => 'text',
            primary_key => 1,
            not_null => 1,
            default => 'live',
        },
        wizlock => {
            type => 'boolean',
            not_null => 1,
            default => 0,
        },
        debug => {
            type => 'boolean',
            not_null => 1,
            default => 0,
        },
        logfile => {
            type => 'text',
        },
        pidfile => {
            type => 'text',
        },
        libdir => {
            type => 'text',
            default => './lib',
        },
        specials => {
            type => 'boolean',
            not_null => 1,
            default => 1,
        },
        gameport => {
            type => 'integer',
            not_null => 1,
            default => 7000,
        },
    ],
    allow_inline_column_values => 1,
);

=head1 FUNCTIONS

=over 8

=item init_db()

This overrides the generic init_db() function we inherit, to provide
the ability to create the table this class needs if it isn't already
there.

=cut

sub init_db {
    my $self = shift;
    die "No valid Mud::DB::Object to get a database handle from!" unless $self->isa('Mud::DB::Object');

    my $db = $self->SUPER::init_db();
    my $dbh = $db->dbh;
    my $sql = qq!
        CREATE TABLE IF NOT EXISTS options(
            id          TEXT NOT NULL PRIMARY KEY DEFAULT 'live',
            wizlock     BOOLEAN NOT NULL DEFAULT false,
            debug       BOOLEAN NOT NULL DEFAULT false,
            logfile     TEXT,
            pidfile     TEXT,
            libdir      TEXT DEFAULT './lib',
            specials    BOOLEAN NOT NULL DEFAULT true,
            gameport    INTEGER NOT NULL DEFAULT 7000
        );
    !;
    my $sth = $dbh->prepare($sql);
    $sth->execute() or die "Cannot create table: $!";
    return $db;
}

=back

=cut

1;
