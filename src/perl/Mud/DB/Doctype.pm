#!/usr/bin/perl -w

package Mud::DB::Doctype;

=head1 NAME

Mud::DB::Doctype - All valid document types.

=head1 OVERVIEW

This class just maps valid document types to documents,
so that differnt types of documents can be treated in
different ways.

=cut

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use Mud::DB::Object;
use base 'Mud::DB::Object';

=head1 SQL

The primary key is simply the name of the document type.

        CREATE TABLE IF NOT EXISTS doctypes(
            id          TEXT NOT NULL PRIMARY KEY
        );

=cut

__PACKAGE__->meta->setup(
    table       => 'doctypes',
    columns     => [ 
        id      => {
            type => 'text',
            primary_key => 1,
            not_null => 1,
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
        CREATE TABLE IF NOT EXISTS doctypes(
            id          TEXT NOT NULL PRIMARY KEY
        );
    !;
    my $sth = $dbh->prepare($sql);
    $sth->execute() or die "Cannot create table: $!";
    return $db;
}

=back

=cut

1;
