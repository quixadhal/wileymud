#!/usr/bin/perl -w

package MudConvert::WileyMUD::ZoneSet;

use strict;
use English;
use MudConvert::Utils qw( vnum_index_file report_error );

sub new {
  my $invocant = shift;
  my $class = ref($invocant) || $invocant;
  my $cfg = shift;
  my @args = ( @_ );
  my $self = { 'Config' => $cfg };
  bless( $self, $class );

  foreach (@args) {
    my $zone = $self->load($_);
    $self->{$zone->{'VNum'}} = $zone
      if (defined $zone) and (defined $zone->{'VNum'})
      and !(defined $self->{$zone->{'VNum'}});
  }

  return $self;
}

sub load {
  my $self		= shift;
  my $cfg		= shift || $self->{'Config'};
  my $filename		= shift	|| $self->{'Filename'}	|| "world/tinyworld.zon";

  my $zone_data = vnum_index_file($cfg, $filename);
  $self->{$_->{'VNum'}} = MudConvert::WileyMUD::Zone->new($cfg, $filename, $_)
    foreach (sort { $a <=> $b } $_->{'VNum'});

  return scalar(keys %{ $self });
}

sub save {
  my $self		= shift;
  my $filename		= shift	|| $self->{'Filename'}	|| "world/tinyworld.zon";

  # Be sure to open for append!
  print STDERR "Override me!\n";
  return undef;
}

1;
