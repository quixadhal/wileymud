#!/usr/bin/perl -w

package MudConvert::Zone;

use strict;
use English;
#use base 'Exporter';

#our @EXPORT_OK = qw( );

#
# To properly initialize a zone so that it can be loaded, you should
# pass in a hash (NOT a ref!) containing the following:
#
# VNum, Filename, Number, Line, BytePos
#
sub new {
  my $invocant = shift;
  my $class = ref($invocant) || $invocant;
  my %args = ( @_ );
  my $self = {};
  bless( $self, $class );

  foreach my $field (qw(
			VNum Filename Number Line BytePos
		     )) {
  no strict "refs";
    *$field = sub : lvalue {
      my $self = shift;
      $self->{$field};
    }
  }
  $self->{$_} = $args{$_} foreach (keys %args);
  return $self;
}

sub load {
  my $self		= shift;
  my $vnum		= shift	|| $self->{'VNum'}	|| 0;
  my $filename		= shift	|| $self->{'Filename'}	|| "world/tinyworld.zon";
  my $bytepos		= shift	|| $self->{'BytePos'}	|| 0;

  print STDERR "Override me!\n";
  return undef;
}

sub save {
  my $self		= shift;
  my $filename		= shift	|| $self->{'Filename'}	|| "world/tinyworld.zon";

  # Be sure to open for append!
  print STDERR "Override me!\n";
  return undef;
}


1;
