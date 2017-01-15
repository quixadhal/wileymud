#!/usr/bin/perl -w

package Mud::DB::Object;

=head1 NAME

Mud::DB::Object - Inherited base class for all data storage objects

=head1 OVERVIEW

To avoid repetition, this is a stub object that establishes our
Rose::DB::Object hierarchy, and ensures the database connection
is established.  Instead of doing this by hand for each data
store class, each one can just inherit this.

This is also a great place to put JSON import/export code,
should you want to dump SQL table data to exportable files.

=cut

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use Mud::DB;

use base qw( Rose::DB::Object );

=head1 FUNCTIONS

=over 8

=item init_db()

This function just ensures that the constructor of our base
class (Mud::DB) has been called.  In particular, we will attempt
to use the same DBI connection handle for any class that inherits
this one.  We also ensure that strings are unicode, and will try
to automatically create the SQLite database file if it doesn't
already exist.

Note that you will usually want to override init_db() in your
individual table classes, because Rose::DB doesn't appear to have
a direct way to create database elements from the schema you use
to define it.  As such, you'll want to provide your own method
to do so, and invoke SUPER::init_db() to make sure this code
gets called.

=cut

sub init_db {
    my $db = Mud::DB->new_or_cached;
    $db->sqlite_unicode(1);
    $db->auto_create(1);
    return $db;
}

=back

=cut

1;
