#!/usr/bin/perl -w

package Mud::Docs;

=head1 NAME

Mud::Docs - The documentation system for PikuMUD

=head1 OVERVIEW

This package implements the help system as well as
man pages and other documentation.  The goal here is
to integrate function lookups, so they pull directly
from the perl source embedded pods, as well as allowing
on-disk documentation that was written by hand.

=head1 FUNCTIONS

=over 8

=cut

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use POSIX qw(strftime);
use JSON;
use Pod::POM;

use Mud::Utils;
use Mud::Logger;

use Exporter qw(import);
our @EXPORT_OK = qw();
our @EXPORT = qw();
our %EXPORT_TAGS = (all => [ @EXPORT, @EXPORT_OK ]);

my $filename = 'documentation.dat';
my $news_file = 'news.txt';
my $credits_file = 'credits.txt';
my $motd_file = 'motd.txt';

=item new()

Constructor.  Initializes the document subsystem.

=cut

sub new {
    my $class = shift;
    my $options = shift;
    my $self = {};

    log_boot "- Setting up documentation";

    if( -r $filename ) {
        my $data = load_file $filename;
        $self = decode_json $data;
    } else {
        $self = {
            _news       => undef,
            _credits    => undef,
            _motd       => undef,
        };
    }

    bless $self, $class;

    $self->news('IMPORT');
    $self->credits('IMPORT');
    $self->motd('IMPORT');

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

A special function called when the documentation object is destroyed.
Normally this will happen during shutdown.  This code should save
parts of the document set back to disk.

=cut

sub DESTROY {
    my $self = shift;

    log_info "Saving documentation to $filename";
    my $json = JSON->new->allow_blessed(1)->convert_blessed(1);
    my $data = $json->encode($self) or die "Invalid JSON conversion: $!";
    open FP, ">$filename" or die "Cannot open $filename: $!";
    print FP "$data\n";
    close FP;
    log_warn "Documentation daemon shut down.";
}

=item import_file()

A private method which handles the import logic for various text files
such as the news, motd, and credits entries.  The actual methods for
each will call this to update their contents from disk, if needed.

=cut

sub import_file {
    my $text_file = shift;

    # compare date of text file vs. json save file, load if newer.
    my ($json_date, $file_date) = (0,0);
    $json_date = (stat $filename)[9] if -f $filename;
    $file_date = (stat $text_file)[9] if -f $text_file;
    log_debug "JSON: %d\n", $json_date;
    log_debug "FILE: %d\n", $file_date;
    if ($file_date > $json_date) {
        my $data = load_file $text_file;
        return [ lines $data ];
    }
    return undef;
}

=item news()

A method to get or set the news file.  If passed the
special argument 'IMPORT', it will load the news from a
text file on disk.  If the argument is any other text
string, it will be broken up into an array of lines
and stored to JSON.  An array-ref will be considered
a pre-processed array of lines.

No argument (undef) will return the current news data.

=cut

sub news {
    my ($self, $setting, @more) = @_;

    if (defined $setting) {
        if (ref $setting eq 'ARRAY') {
            $self->{_news} = $setting;
        } else {
            if ($setting =~ /^IMPORT$/) {
                my $data = import_file($news_file);
                $self->{_news} = $data if defined $data;
            } else {
                $self->{_news} = [ lines $setting ];
                push @{ $self->{_news} }, (lines $_) foreach (@more);
            }
        }
    }
    return $self->{_news};
}

=item credits()

Just like news(), this method returns the credits file.

=cut

sub credits {
    my ($self, $setting, @more) = @_;

    if (defined $setting) {
        if (ref $setting eq 'ARRAY') {
            $self->{_credits} = $setting;
        } else {
            if ($setting =~ /^IMPORT$/) {
                my $data = import_file($credits_file);
                $self->{_credits} = $data if defined $data;
            } else {
                $self->{_credits} = [ lines $setting ];
                push @{ $self->{_credits} }, (lines $_) foreach (@more);
            }
        }
    }
    return $self->{_credits};
}

=item motd()

Just like news(), this method returns the motd file.

=cut

sub motd {
    my ($self, $setting, @more) = @_;

    if (defined $setting) {
        if (ref $setting eq 'ARRAY') {
            $self->{_motd} = $setting;
        } else {
            if ($setting =~ /^IMPORT$/) {
                my $data = import_file($motd_file);
                $self->{_motd} = $data if defined $data;
            } else {
                $self->{_motd} = [ lines $setting ];
                push @{ $self->{_motd} }, (lines $_) foreach (@more);
            }
        }
    }
    return $self->{_motd};
}

=back

=cut

1;

