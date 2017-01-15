#!/usr/bin/perl -w

package Mud::Comm;

=head1 NAME

Mud::Comm - The socket handling system

=head1 OVERVIEW

This package tries to encapsulate the socket handling
system so that the rest of the game only needs to get
the descriptors and read/write to them, without having
to care about any socket-related stuff.

=cut

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use Socket qw(IPPROTO_TCP TCP_NODELAY);
use Fcntl qw(F_GETFL F_SETFL O_NONBLOCK O_NDELAY);
use IO::Select;
use IO::Socket::INET;
use Net::Telnet::Options;
use JSON;
use Scalar::Util qw(looks_like_number);
use Tie::RefHash;

use Mud::Logger;
use Mud::Utils;
use Mud::Login;

use Exporter qw(import);
our @EXPORT_OK = qw();
our @EXPORT = qw();
our %EXPORT_TAGS = (all => [ @EXPORT, @EXPORT_OK ]);

my $filename = "warmboot.json";

=head1 FUNCTIONS

=over 8

=item tweak_socket()

A function to set a few commonly used options on the
given socket, called right after the socket is created.

=cut

sub tweak_socket {
    my $socket = shift;
    
    my $flags = fcntl($socket, F_GETFL, 0) or warn "Cannot get flags for socket: $!";
    fcntl($socket, F_SETFL, $flags | O_NONBLOCK) or warn "Cannot set socket to nonblocking: $!";
    setsockopt($socket,SOL_SOCKET,SO_LINGER,pack("l*",0,0)) or warn "Cannot set SO_LINGER option for socket: $!";
    setsockopt($socket,IPPROTO_TCP,TCP_NODELAY,1) or warn "Cannot disable Nagle for socket: $!";
}

=back

=head1 METHODS

=over 8

=item new()

Constructor.

This will set up the initial listening socket.  It needs to be
passed the Mud::Options object to get the port number.

=cut

sub new {
    my $class = shift;
    my $options = shift;

    tie my %connections, 'Tie::RefHash';

    my $self = { 
        _options        => undef,
        _warmboot       => undef,
        _listener       => undef,
        _selector       => undef,
        _connections    => \%connections,
    };

    die "Cannot initialize socket system without a valid Mud::Options object!" unless $options->isa('Mud::Options');
    $self->{_options} = $options;

    bless $self, $class;
    log_boot "Opening mother connection.";
    my $socket = IO::Socket::INET->new(
        Listen      => 5,
        LocalPort   => $options->gameport,
        ReuseAddr   => 1,
    ) or die "Cannot bind main socket: $!";
    tweak_socket $socket;
    $self->{_listener} = $socket;
    my $selector = IO::Select->new() or die "Cannot open IO::Select object!";
    $selector->add($socket);
    $self->{_selector} = $selector;
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
    $obj->{$_} = $self->{$_} foreach grep { ! /^(_options|_listener|_selector)$/ } (keys %$self);
    # Here we need to break out info about which file descriptors match
    # to which sockets, so we can properly restore them.  We will also
    # need to tie this to user data when we have such things.

    # Since sockets are IO::Socket objects, we can use $i->fileno to 
    # get their descriptor.
    return $obj;
}

=item DESTROY()

A special function called when the comm object is destroyed.
Normally this will happen during shutdown.  This code should
ensure all sockets are closed properly *OR* implement the
copyover/warmboot mechanic to allow them to be restored by
a new executable.

=cut

sub DESTROY {
    my $self = shift;

    if ($self->warmboot) {
        log_info "Saving comm status for warmboot";
        my $json = JSON->new->allow_blessed(1)->convert_blessed(1);
        my $data = $json->encode($self) or die "Invalid JSON conversion: $!";
        open FP, ">$filename" or die "Cannot open $filename: $!";
        print FP "$data\n";
        close FP;
    } else {
        log_info "Shutting down comm system";
        $self->selector->remove($self->listener);
        shutdown $self->listener, 2;
        foreach (keys %{ $self->{_connections} }) {
            $_->close;
        }
    }

    log_warn "Comm system shut down.";
}

=item listener()

A method to retrieve the main listening socket, upon
which new connections will appear.  Also known as the
game port.  This can also be passed a socket object to
assign a new one, usually in the case of a warmboot
recovery.

=cut

sub listener {
    my ($self, $setting) = @_;
    if (defined $setting) {
        die "Cannot assign non-socket object as gameport!" unless $setting->isa('IO::Socket::INET');
        $self->{_listener} = $setting;
    }
    return $self->{_listener};
}

=item selector()

This returns the selector object.  Mostly it's as a shorthand
to avoid having to dig into the hash to get it, but maybe an
external module in the future will need it.

=cut

sub selector {
    my $self = shift;

    return $self->{_selector};
}

=item connections()

This method returns the list of connected sockets.

=cut

sub connections {
    my $self = shift;

    return keys %{ $self->{_connections} };
}

=item object()

This method returns the object (if any) associated with
a given socket.  Typically, this will be used to handle
parsing input from the socket.

=cut

sub object {
    my $self = shift;
    my $sock = shift;

    return $self->{_connections}->{$sock};
}

=item warmboot()

This method either sets the warmboot status (true or false), or returns that
status.  It is used by the shutdown process to decide if we are attempting a
warmboot, or a standard shutdown.

=cut

sub warmboot {
    my ($self, $setting) = @_;
    $self->{_warmboot} = 1 if $setting;
    return $self->{_warmboot};
}

=item poll()

A method which polls the connections for data to read, and returns the list
of connection objects which have data ready to be processed.  You can pass
in a timeout value, or the default of 0 will be used (non-blocking, returns
immediately if no data is ready).

=cut

sub poll {
    my ($self, $setting) = @_;

    $setting = 0 if !defined $setting or !looks_like_number $setting;

    my @ready = $self->selector->can_read( $setting );
    foreach (@ready) {
        if( $_ == $self->listener ) {
            my $new_socket = $_->accept;
            tweak_socket $new_socket;
            $self->selector->add($new_socket);
            $self->{_connections}->{$new_socket} = Mud::Login->new($new_socket);
            last;
        }
    }
    return ( grep { $_ != $self->listener } @ready );
}

=item close()

This method allows something outside the comm module to request that
the socket passed in be closed, with the appropriate cleanup done
to ensure we stop trying to poll it, or keep track of it.

=cut

sub close {
    my ($self, $socket) = @_;

    return if !defined $socket;
    die "Cannot close non-socket object!" unless $socket->isa('IO::Socket::INET');
    delete $self->{_connections}->{$socket};
    $self->selector->remove($socket);
    shutdown $socket, 2;
}

=back

=cut

1;

