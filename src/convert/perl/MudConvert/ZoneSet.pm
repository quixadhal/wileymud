#!/usr/bin/perl -w

package MudConvert::ZoneSet;

use strict;
use English;

sub new {
  my $invocant = shift;
  my $class = ref($invocant) || $invocant;
  my @args = ( @_ );
  my $self = {};
  bless( $self, $class );

  foreach (@args) {
    my $zone = $self->load($_);
    $self->{$zone->{'VNum'}} = $zone
      if (defined $zone) and (defined $zone->{'VNum'});
  }

  return $self;
}

sub load {
  my $self		= shift;
  my $filename		= shift	|| $self->{'Filename'}	|| "world/tinyworld.zon";

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

sub _vnum_index_file {
  my $cfg = shift;
  my $filename = shift;
  my $vnum_map = {};

  die "No file specified!" if !(defined $filename);
  die "Cannot open file $filename!" if !(-r $filename);
  print "Indexing $filename..." if !$cfg->{'quiet'};
  open FP, "$filename";
  my $line = 1;
  my $count = 0;
  my $prev_pos = tell FP;
  while(<FP>) {
    chomp;
    if( $_ =~ /^\#(\d+)/ ) {
      my $vnum = $1;
      my $len = length "$vnum";
      printf "%d%s%s", $vnum, spin($count), "\b"x($len + 1) if $cfg->{'verbose'};
      $vnum_map->{$vnum}->{'VNum'} = $vnum;
      $vnum_map->{$vnum}->{'Filename'} = $filename;
      $vnum_map->{$vnum}->{'Number'} = $count++;
      $vnum_map->{$vnum}->{'Line'} = $line;
      $vnum_map->{$vnum}->{'BytePos'} = $prev_pos;
    }
    $line++;
    $prev_pos = tell FP;
  }
  close FP;
  print "done\n" if !$cfg->{'quiet'};
  return $vnum_map;
}


1;
