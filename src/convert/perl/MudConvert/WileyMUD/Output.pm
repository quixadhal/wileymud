#!/usr/bin/perl -w

package MudConvert::WileyMUD::Output;

use strict;
use English;
use Data::Dumper;
use MudConvert::Utils			qw(	vnum_index_file report_error );
use MudConvert::WileyMUD::Constants	qw(	$sector_types $rev_sector_types
						$room_flags $rev_room_flags
						$zone_commands
						$exit_directions $rev_exit_directions
						$zone_reset_flags $rev_zone_reset_flags
					);
use MudConvert::API			qw(	flags_value
					);

our ($dump_dir, $dump_zone_file, $dump_world_file) = (undef,undef,undef);

use base 'Exporter';

our @EXPORT_OK = qw( dump_game dump_zones dump_rooms dump_mobs dump_objects dump_shops );

sub dump_game {
  my $cfg = shift;
  my $data = shift;
  my $results = {};

  $dump_dir = $cfg->{'destination-dir'}."/WileyMUD";
  if( !(mkdir $dump_dir) ) {
    #system("echo rm -rf ".$cfg->{'destination-dir'}.'/WileyMUD' $cfg->{'destination-dir'}.'/WileyMUD.old');
    #system("echo mv ".$cfg->{'destination-dir'}.'/WileyMUD'." ".$cfg->{'destination-dir'}.'/WileyMUD.old');
    printf STDERR "FATAL: Cannot create output directory (%s) for WileyMUD!\n", $dump_dir;
    #return undef;
  }
  $dump_zone_file = "$dump_dir/tinyworld.zon";
  $dump_world_file = "$dump_dir/tinyworld.wld";

  $results->{Zones}	= dump_zones($cfg, $data);
  $results->{Rooms}	= dump_rooms($cfg, $data);
  $results->{Mobs}	= dump_mobs($cfg, $data);
  $results->{Objects}	= dump_objects($cfg, $data);
  $results->{Shops}	= dump_shops($cfg, $data);

  return $results;
}

sub dump_zones {
  my $cfg = shift;
  my $data = shift;

  my $zone_data = $data->{'Zones'};

  if( !(open DUMP, ">$dump_zone_file") ) {
    printf STDERR "FATAL: Cannot create output ZONE file (%s) for WileyMUD!\n", $dump_zone_file;
    return undef;
  }
  foreach my $vnum (sort { $a <=> $b } keys %{ $zone_data }) {
    printf DUMP "#%d\n",	$vnum;
    printf DUMP "%s~\n",	$zone_data->{$vnum}->{'Name'};
    printf DUMP "%d %d %d\n",	$zone_data->{$vnum}->{'Top'},
				$zone_data->{$vnum}->{'Time'},
				$rev_zone_reset_flags->{$zone_data->{$vnum}->{'Mode'}};
    foreach my $reset (sort { $a->{'Number'} <=> $b->{'Number'} } @{ $zone_data->{$vnum}->{'Resets'} }) {
      #printf STDERR Dumper($reset);
      printf DUMP "%s",		$reset->{'Command'};
      printf DUMP " %s",	join(" ", @{ $reset->{'Args'} })
        if (defined $reset->{'Args'}) and scalar(@{ $reset->{'Args'} }) > 0;
      printf DUMP " * %s",	$reset->{'Comment'}
        if (defined $reset->{'Comment'});
      printf DUMP "\n";
    }
    printf DUMP "%s\n\n",	"S";
  }
  printf DUMP "%s\n",		"\$~";
  close DUMP;

  return undef;
}

sub dump_rooms {
  my $cfg = shift;
  my $data = shift;

  my $room_data = $data->{'Rooms'};

  if( !(open DUMP, ">$dump_world_file") ) {
    printf STDERR "FATAL: Cannot create output WORLD file (%s) for WileyMUD!\n", $dump_world_file;
    return undef;
  }
  foreach my $vnum (sort { $a <=> $b } keys %{ $room_data }) {
    printf DUMP "#%d\n",	$vnum;
    printf DUMP "%s~\n",	$room_data->{$vnum}->{'Name'};
    printf DUMP "%s~\n",	$room_data->{$vnum}->{'Description'};
    printf DUMP "%d %d %d\n",	$room_data->{$vnum}->{'Zone'},
				flags_value($room_data->{$vnum}->{'Flags'}, $rev_room_flags),
				$rev_sector_types->{$room_data->{$vnum}->{'Sector'}};

    foreach my $reset (sort { $a->{'Number'} <=> $b->{'Number'} } @{ $room_data->{$vnum}->{'Resets'} }) {
      #printf STDERR Dumper($reset);
      printf DUMP "%s",		$reset->{'Command'};
      printf DUMP " %s",	join(" ", @{ $reset->{'Args'} })
        if (defined $reset->{'Args'}) and scalar(@{ $reset->{'Args'} }) > 0;
      printf DUMP " * %s",	$reset->{'Comment'}
        if (defined $reset->{'Comment'});
      printf DUMP "\n";
    }
    printf DUMP "%s\n\n",	"S";
  }
  printf DUMP "%s\n",		"\$~";
  close DUMP;

  return undef;
}

sub dump_mobs {
  my $cfg = shift;
  my $data = shift;

  return undef;
}

sub dump_objects {
  my $cfg = shift;
  my $data = shift;

  return undef;
}

sub dump_shops {
  my $cfg = shift;
  my $data = shift;

  return undef;
}

1;

