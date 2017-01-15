#!/usr/bin/perl -w

package Mud::Documents;

=head1 NAME

Mud::Documents - The documentation system for PikuMUD

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

use JSON;
use Pod::POM;

use Mud::Utils;
use Mud::Logger;

use Exporter qw(import);
our @EXPORT_OK = qw();
our @EXPORT = qw();
our %EXPORT_TAGS = (all => [ @EXPORT, @EXPORT_OK ]);

my $filename = 'documentation.json';
my $news_file = 'news.txt';
my $credits_file = 'credits.txt';
my $motd_file = 'motd.txt';
my $wizmotd_file = 'wizmotd.txt';
my $wizlist_file = 'wizlist.txt';
my $info_file = 'info.txt';
my $story_file = 'story.txt';
my $license_file = 'license.doc'; # Filename is fixed by the license itself
my $greeting_file = 'greeting.txt';

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
            _news           => undef,
            _credits        => undef,
            _motd           => undef,
            _wizmotd        => undef,
            _wizlist        => undef,
            _info           => undef,
            _story          => undef,
            _license        => undef,
            _greeting       => undef,

            _login_menu     => undef,
            _sex_menu       => undef,
            _race_menu      => undef,
            _class_menu     => undef,
            _race_help      => undef,
            _class_help     => undef,
            _suicide_warn   => undef,
            _suicide_done   => undef,
            _main_help      => undef,
        };
    }

    bless $self, $class;

    $self->news('IMPORT');
    $self->credits('IMPORT');
    $self->motd('IMPORT');
    $self->wizmotd('IMPORT');
    $self->info('IMPORT');
    $self->story('IMPORT');
    $self->license('IMPORT');
    $self->greeting('IMPORT');

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
        return lines $data;
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
                $self->{_news} = lines $setting;
                push @{ $self->{_news} }, @{ lines $_ } foreach (@more);
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
                $self->{_credits} = lines $setting;
                push @{ $self->{_credits} }, @{ lines $_ } foreach (@more);
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
                $self->{_motd} = lines $setting;
                push @{ $self->{_motd} }, @{ lines $_ } foreach (@more);
            }
        }
    }
    return $self->{_motd};
}

=item wizmotd()

Just like news(), this method returns the wizard motd file.

=cut

sub wizmotd {
    my ($self, $setting, @more) = @_;

    if (defined $setting) {
        if (ref $setting eq 'ARRAY') {
            $self->{_wizmotd} = $setting;
        } else {
            if ($setting =~ /^IMPORT$/) {
                my $data = import_file($wizmotd_file);
                $self->{_wizmotd} = $data if defined $data;
            } else {
                $self->{_wizmotd} = lines $setting;
                push @{ $self->{_wizmotd} }, @{ lines $_ } foreach (@more);
            }
        }
    }
    return $self->{_wizmotd};
}

=item wizlist()

Just like news(), this method returns the wizlist file.

=cut

sub wizlist {
    my ($self, $setting, @more) = @_;

    if (defined $setting) {
        if (ref $setting eq 'ARRAY') {
            $self->{_wizlist} = $setting;
        } else {
            if ($setting =~ /^IMPORT$/) {
                my $data = import_file($wizlist_file);
                $self->{_wizlist} = $data if defined $data;
            } else {
                $self->{_wizlist} = lines $setting;
                push @{ $self->{_wizlist} }, @{ lines $_ } foreach (@more);
            }
        }
    }
    return $self->{_wizlist};
}

=item info()

Just like news(), this method returns the newbie info file.

=cut

sub info {
    my ($self, $setting, @more) = @_;

    if (defined $setting) {
        if (ref $setting eq 'ARRAY') {
            $self->{_info} = $setting;
        } else {
            if ($setting =~ /^IMPORT$/) {
                my $data = import_file($info_file);
                $self->{_info} = $data if defined $data;
            } else {
                $self->{_info} = lines $setting;
                push @{ $self->{_info} }, @{ lines $_ } foreach (@more);
            }
        }
    }
    return $self->{_info};
}

=item story()

Just like news(), this method returns the story of the game.

=cut

sub story {
    my ($self, $setting, @more) = @_;

    if (defined $setting) {
        if (ref $setting eq 'ARRAY') {
            $self->{_story} = $setting;
        } else {
            if ($setting =~ /^IMPORT$/) {
                my $data = import_file($story_file);
                $self->{_story} = $data if defined $data;
            } else {
                $self->{_story} = lines $setting;
                push @{ $self->{_story} }, @{ lines $_ } foreach (@more);
            }
        }
    }
    return $self->{_story};
}

=item license()

Just like news(), this method returns the license file.

=cut

sub license {
    my ($self, $setting, @more) = @_;

    if (defined $setting) {
        if (ref $setting eq 'ARRAY') {
            $self->{_license} = $setting;
        } else {
            if ($setting =~ /^IMPORT$/) {
                my $data = import_file($license_file);
                $self->{_license} = $data if defined $data;
            } else {
                $self->{_license} = lines $setting;
                push @{ $self->{_license} }, @{ lines $_ } foreach (@more);
            }
        }
    }
    return $self->{_license};
}

=item greeting()

Just like news(), this method returns the greeting (login) file.

=cut

sub greeting {
    my ($self, $setting, @more) = @_;

    if (defined $setting) {
        if (ref $setting eq 'ARRAY') {
            $self->{_greeting} = $setting;
        } else {
            if ($setting =~ /^IMPORT$/) {
                my $data = import_file($greeting_file);
                $self->{_greeting} = $data if defined $data;
            } else {
                $self->{_greeting} = lines $setting;
                push @{ $self->{_greeting} }, @{ lines $_ } foreach (@more);
            }
        }
    }
    return $self->{_greeting};
}

=back

=cut

1;

