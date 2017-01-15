#!/usr/bin/perl -w

package Mud::DB;

=head1 NAME

Mud::DB - The main DB class to tie Rose::DB to an SQLite database

=head1 OVERVIEW

Rose::DB::Object is an ORM system for perl that lets you tie
objects to database tables.  It relies on Rose::DB to establish
the database connection and manage the raw queries.

This is the generic base class that establishes our connection to
the backend database (SQLite), and ensures it is ready to go.  Should
you want to change databases, this should be the only file that needs
significan changes.

=cut

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use base qw( Rose::DB );

__PACKAGE__->use_private_registry;
__PACKAGE__->register_db(
    driver      => 'sqlite',
    database    =>  'wileymud.sqlite',
);

1;
