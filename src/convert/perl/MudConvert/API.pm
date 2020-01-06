#!/usr/bin/perl -w

package MudConvert::API;

use strict;
use English;
use base 'Exporter';

our @EXPORT_OK =  qw(
			zone_name
			room_name
			mob_name
			obj_name
			zone_span
			zone_reset_desc
			exit_name
			exit_type_desc
			exit_flag_list
			flags_value
                 );


sub _check_vnum {
  my $data = shift;
  my $id = shift;

  return undef if !(defined $data) or !(defined $id) or (length $id) < 1;
  return 1 if $id =~ /^\-?\d+$/ and (defined $data->{$id}) and (defined $data->{$id}->{'Name'});
  return undef;
}

sub _get_prop {
  my $data = shift;
  my $id = shift;
  my $field = shift || 'Name';

  return undef if !(defined $data) or !(defined $id) or (length $id) < 1;
  if( $id =~ /^\-?\d+$/ ) { #vnum
    return $data->{$id}->{$field} if (defined $data->{$id});
  } else {
    return $id if( grep { $_->{$field} eq $id } ( keys %{$data} ) );
  }
}

sub _get_name {
  my $data = shift;
  my $id = shift;

  my $name = _get_prop($data, $id, 'Name');
  return (defined $name) ? $name : 'UNKNOWN';
}

sub zone_name {
  my $data = shift;

  return 'UNKNOWN' if !(defined $data) or !(defined $data->{'Zones'});
  return _get_name($data->{'Zones'}, @_);
}

sub room_name {
  my $data = shift;

  return 'UNKNOWN' if !(defined $data) or !(defined $data->{'Rooms'});
  return _get_name($data->{'Rooms'}, @_);
}

sub mob_name {
  my $data = shift;

  return 'UNKNOWN' if !(defined $data) or !(defined $data->{'Mobs'});
  return _get_name($data->{'Mobs'}, @_);
}

sub obj_name {
  my $data = shift;

  return 'UNKNOWN' if !(defined $data) or !(defined $data->{'Objects'});
  return _get_name($data->{'Objects'}, @_);
}

sub zone_span {
  my $data = shift;
  my $vnum = shift;

  return (-1,-1) if !(defined $data) or !(defined $data->{'Zones'}) or !(defined $vnum);
  return (-1,-1) if !_check_vnum($data->{'Zones'}, $vnum);
  for( my ($i, $x) = (0, [ sort { $a <=> $b } keys %{ $data->{'Zones'} } ]); $i < scalar(@{ $x }); $i++) {
    my $vnum = $x->[$i];

    next if $vnum < $vnum;
    last if $vnum > $vnum;
    return ( ((!$i)? 0: $data->{'Zones'}->{$x->[$i-1]}->{'Top'} + 1), $data->{'Zones'}->{$vnum}->{'Top'} );
  }
  return (-1,-1);
}

sub zone_reset_desc {
  my $data = shift;
  my $vnum = shift;

  my $zone_reset_map = {
	'RESET_UNKNOWN'	=> "I do not know how it resets.",
	'RESET_NEVER'	=> "It NEVER resets.",
	'RESET_PC'	=> "It resets every %d minutes, if NO players are present.",
	'RESET_ALWAYS'	=> "It ALWAYS resets every %d minutes.",
	};

  return 'UNKNOWN' if !(defined $vnum) or (length $vnum) < 1 or !(defined $data) or !(defined $data->{'Zones'});
  if( ($vnum =~ /^\-?\d+$/) and (defined $data->{'Zones'}->{$vnum}) ) {
    my $mode = $data->{'Zones'}->{$vnum}->{'Mode'};
    my $time = $data->{'Zones'}->{$vnum}->{'Time'};
    return sprintf( $zone_reset_map->{$mode}, $time )
      if  (defined $data->{'Zones'}->{$vnum}->{'Mode'})
      and (defined $data->{'Zones'}->{$vnum}->{'Time'})
      and grep /^$mode$/, (keys %{ $zone_reset_map });
  } else {
    print STDERR "WARNING: Unknown ZONE RESET TYPE for Zone #$vnum\n";
  }
  return 'UNKNOWN';
}

sub exit_name {
  my $id = shift;

  my $exit_direction_map = {
	'EXIT_NONE'	=> 'NONE',
	'EXIT_NORTH'	=> 'North',
	'EXIT_EAST'	=> 'East',
	'EXIT_SOUTH'	=> 'South',
	'EXIT_WEST'	=> 'West',
	'EXIT_UP'	=> 'Up',
	'EXIT_DOWN'	=> 'Down',
	};

  return 'UNKNOWN' if !(defined $id) or (length $id) < 1;
  if( $id =~ /^\-?\d+$/ ) {
    print STDERR "FATAL: Untranslated EXIT DIRECTION\n";
  } else {
    return $exit_direction_map->{$id} if grep /^$id$/, (keys %{ $exit_direction_map });
  }
  return 'UNKNOWN';
}

sub exit_type_desc {
  my $id = shift;

  my $exit_type_map = {
	'EXIT_INVALID'			=> "a BROKEN Exit",
	'EXIT_OPEN'			=> "an Open Passage",
	'EXIT_DOOR'			=> "a Door",
	'EXIT_NOPICK'			=> "an Unpickable Door",
	'EXIT_SECRET'			=> "a Secret Door",
	'EXIT_SECRET_NOPICK'		=> "an Unpickable Secret Door",
	'EXIT_OPEN_ALIAS'		=> "an Aliased Open Passage",
	'EXIT_DOOR_ALIAS'		=> "an Aliased Door",
	'EXIT_NOPICK_ALIAS'		=> "an Aliased Unpickable Door",
	'EXIT_SECRET_ALIAS'		=> "an Aliased Secret Door",
	'EXIT_SECRET_NOPICK_ALIAS'	=> "an Aliased Unpickable Secret Door",
	};

  return 'UNKNOWN' if !(defined $id) or (length $id) < 1;
  if( $id =~ /^\-?\d+$/ ) {
    print STDERR "FATAL: Untranslated EXIT TYPE\n";
  } else {
    return $exit_type_map->{$id}
      if (defined $exit_type_map->{$id});
  }
  return 'UNKNOWN';
}

sub exit_flag_list {
  my $id = shift;

  my $exit_flag_map = {
	'EXIT_INVALID'			=> [],
	'EXIT_OPEN'			=> [],
	'EXIT_DOOR'			=> [ 'EXITFLAG_DOOR' ],
	'EXIT_NOPICK'			=> [ 'EXITFLAG_DOOR', 'EXITFLAG_NOPICK' ],
	'EXIT_SECRET'			=> [ 'EXITFLAG_DOOR', 'EXITFLAG_SECRET' ],
	'EXIT_SECRET_NOPICK'		=> [ 'EXITFLAG_DOOR', 'EXITFLAG_NOPICK', 'EXITFLAG_SECRET' ],
	'EXIT_OPEN_ALIAS'		=> [ 'EXITFLAG_ALIAS' ],
	'EXIT_DOOR_ALIAS'		=> [ 'EXITFLAG_DOOR', 'EXITFLAG_ALIAS' ],
	'EXIT_NOPICK_ALIAS'		=> [ 'EXITFLAG_DOOR', 'EXITFLAG_NOPICK', 'EXITFLAG_ALIAS' ],
	'EXIT_SECRET_ALIAS'		=> [ 'EXITFLAG_DOOR', 'EXITFLAG_SECRET', 'EXITFLAG_ALIAS' ],
	'EXIT_SECRET_NOPICK_ALIAS'	=> [ 'EXITFLAG_DOOR', 'EXITFLAG_NOPICK', 'EXITFLAG_SECRET', 'EXITFLAG_ALIAS' ],
	};

  return [] if !(defined $id) or (length $id) < 1;
  if( $id =~ /^\-?\d+$/ ) {
    print STDERR "FATAL: Untranslated EXIT TYPE\n";
  } else {
    return $exit_flag_map->{$id}
      if (defined $exit_flag_map->{$id});
  }
  return [];
}

sub flags_value {
  my $ref = shift;
  my $conversion = shift;

  return 0 if !(defined $ref) or !(defined $conversion);
  my $a = join(' + ', map { $conversion->{$_} } @{ $ref });
  $a = eval $a;
  return 0 if !(defined $a);
  return $a;
}

1;
