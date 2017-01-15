#!/usr/bin/perl -w

package Mud::DB::Document;

=head1 NAME

Mud::DB::Document - Documentation storage class.

=head1 OVERVIEW

This class maps documentents by name, storing them as text
fields which can thus be easily searched or retrieved.

=cut

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use Mud::DB::Object;
use base 'Mud::DB::Object';

=head1 SQL

The document table is pretty simple, just a name as the main
key field, equivalent to the filename, and the document text
itself.

We also have a doctype field which references the doctype
table, to ensure the document is of a known type.

The actual document text is in the body field.

We can also add level restrictions, class restrictions, or
whatever else seems appropriate, later on.

        CREATE TABLE IF NOT EXISTS documents(
            id          TEXT NOT NULL PRIMARY KEY,
            doctype     TEXT NOT NULL REFERENCES doctype(id),
            body        TEXT
        );

=cut

__PACKAGE__->meta->setup(
    table       => 'documents',
    columns     => [ 
        id      => {
            type => 'text',
            primary_key => 1,
            not_null => 1,
        },
        doctype => {
            type => 'text',
            not_null => 1,
        },
        body => {
            type => 'text',
        },
    ],
    foreign_keys    => [
        doctype => {
            class       => 'Doctype',
            key_columns => { doctype => 'id' },
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
        CREATE TABLE IF NOT EXISTS documents(
            id          TEXT NOT NULL PRIMARY KEY,
            doctype     TEXT NOT NULL REFERENCES doctype(id),
            body        TEXT
        );
    !;
    my $sth = $dbh->prepare($sql);
    $sth->execute() or die "Cannot create table: $!";
    return $db;
}

=back

=cut

1;
