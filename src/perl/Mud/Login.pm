#!/usr/bin/perl -w

package Mud::Login;

=head1 NAME

Mud::Login - The login handler for new sockets

=head1 OVERVIEW

This implements the login process for new connections
that show up on the game.  It should emit a welcome
screen (login banner), and provide a list of possible
options at login, followed by a prompt.

Input on this connection should move the potential
player through various login states until they either
decide to create a new account or validate an existing
one.

In either case, control will then be transferred to
the Account module.

=head1 FUNCTIONS

=over 8

=cut

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use Mud::Logger;
use Mud::Utils;

=back

=head1 METHODS

=over 8

=item new()

Constructor.

=cut

sub new {
    my $class = shift;
    my $socket = shift;

    my $self = { 
        _socket     => undef,
    };

    die "Cannot login without a valid IO::Socket::INET object!" unless $socket->isa('IO::Socket::INET');
    die "Cannot login without a valid Mud::Docs object initialized in main package!" unless defined $main::docs;
    $self->{_socket} = $socket;

    bless $self, $class;
    log_info "New connection from %s!", $socket->peerhost;
    syswrite($socket, join("\n\r", @{ $main::docs->greeting }) . "\n\r");
    return $self;
}

=item DESTROY()

Destructor.

=cut

sub DESTROY {
    my $self = shift;
    log_info "Closing login object";
}

=back

=cut

1;

