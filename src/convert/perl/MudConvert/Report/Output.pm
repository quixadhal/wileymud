#!/usr/bin/perl -w

package MudConvert::Report::Output;

use strict;
use English;
use MudConvert::API qw( zone_name room_name mob_name obj_name
			zone_span zone_reset_desc exit_name exit_type_desc exit_flag_list );

our ($dump_dir, $dump_zone_file, $dump_world_file) = (undef,undef,undef);

use base 'Exporter';

our @EXPORT_OK = qw( dump_game );

sub dump_game {
  my $cfg = shift;
  my $data = shift;
  my $results = {};

  $dump_dir = $cfg->{'destination-dir'}."/Report";
  if( !(mkdir $dump_dir) ) {
    #system("echo rm -rf ".$cfg->{'destination-dir'}.'/WileyMUD' $cfg->{'destination-dir'}.'/WileyMUD.old');
    #system("echo mv ".$cfg->{'destination-dir'}.'/WileyMUD'." ".$cfg->{'destination-dir'}.'/WileyMUD.old');
    printf STDERR "FATAL: Cannot create output directory (%s) for Report!\n", $dump_dir;
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

  if( !(open DUMP, ">$dump_zone_file") ) {
    printf STDERR "FATAL: Cannot create output ZONE file (%s) for Report!\n", $dump_zone_file;
    return undef;
  }

  my $total_cmds = 0;
  $total_cmds += scalar(@{ $data->{'Zones'}->{$_}->{'Resets'} }) foreach (keys %{ $data->{'Zones'} });
  printf DUMP "ZONE REPORT: %d total commands found in %d zones.\n", $total_cmds, scalar(keys %{ $data->{'Zones'} });

  foreach my $vnum (sort { $a <=> $b } keys %{ $data->{'Zones'} }) {
  #for( my ($i, $x) = (0, [ sort { $a <=> $b } keys %{ $data->{'Zones'} } ]); $i < scalar(@{ $x }); $i++) {
  #  my $vnum = $x->[$i];

    printf DUMP "Zone \"%s\"[#%d] spans rooms (#%d,#%d)\n",
           zone_name($data, $vnum), $vnum,
           zone_span($data, $vnum);

    if( (defined $data->{'Zones'}->{$vnum}->{'Mode'}) ) {
      printf DUMP "     %s\n", zone_reset_desc($data, $vnum);
    } else {
      printf DUMP "     It has no reset mode.\n";
    }

    my $cmd_count = scalar(@{ $data->{'Zones'}->{$vnum}->{'Resets'} });
    if( $cmd_count > 0 ) {
      printf DUMP "     There are %d Commands defined:\n", $cmd_count;
    } else {
      printf DUMP "     There are no Commands in this Zone.\n", 
    }

    my ($last_led, $last_mob, $last_obj, $last_room) = (undef,undef,undef,undef);
    foreach my $reset (sort { $a->{'Number'} <=> $b->{'Number'} } @{ $data->{'Zones'}->{$vnum}->{'Resets'} }) {
      printf DUMP "          [%3d] ", $reset->{'Number'};

      if( $reset->{'Name'} eq 'MOBILE' ) {
        printf DUMP "Load Mobile \"%s\"[#%d] to \"%s\"[#%d]\n",
               mob_name($data, $reset->{'MOB_VNUM'}), $last_led = $last_mob = $reset->{'MOB_VNUM'},
               room_name($data, $reset->{'ROOM_VNUM'}), $last_room = $reset->{'ROOM_VNUM'};

      } elsif( $reset->{'Name'} eq 'OBJECT' ) {
        printf DUMP "Load Object \"%s\"[#%d] to \"%s\"[#%d]\n",
               obj_name($data, $reset->{'OBJ_VNUM'}), $last_obj = $reset->{'OBJ_VNUM'},
               room_name($data, $reset->{'ROOM_VNUM'}), $last_room = $reset->{'ROOM_VNUM'};

      } elsif( $reset->{'Name'} eq 'GIVE' ) {
        printf DUMP "Give Object \"%s\"[#%d] to Mobile \"%s\"[#%d]\n",
               obj_name($data, $reset->{'OBJ_VNUM'}), $last_obj = $reset->{'OBJ_VNUM'},
               mob_name($data, $last_mob), $last_mob;

      } elsif( $reset->{'Name'} eq 'EQUIP' ) {
        printf DUMP "Object \"%s\"[#%d] is equipped to %s slot by Mobile \"%s\"[#%d]\n",
               obj_name($data, $reset->{'OBJ_VNUM'}), $last_obj = $reset->{'OBJ_VNUM'},
               $reset->{'EQUIP_POS'},
               mob_name($data, $last_mob), $last_mob;

      } elsif( $reset->{'Name'} eq 'PUT' ) {
        printf DUMP "Put Object \"%s\"[#%d] into Object \"%s\"[#%d]\n",
               obj_name($data, $reset->{'SRC_OBJ'}), $last_obj = $reset->{'SRC_OBJ'},
               obj_name($data, $reset->{'DEST_OBJ'}), $reset->{'DEST_OBJ'};

      } elsif( $reset->{'Name'} eq 'DOOR' ) {
        printf DUMP "Reset %s door of \"%s\"[#%d] to %s\n",
               $reset->{'EXIT_DIR'},
               room_name($data, $reset->{'ROOM_VNUM'}), $last_room = $reset->{'ROOM_VNUM'},
               $reset->{'DOOR_STATE'};

      } elsif( $reset->{'Name'} eq 'REMOVE' ) {
        printf DUMP "Remove Object \"%s\"[#%d] from \"%s\"[#%d]\n",
               obj_name($data, $reset->{'OBJ_VNUM'}), $last_obj = $reset->{'OBJ_VNUM'},
               room_name($data, $reset->{'ROOM_VNUM'}), $last_room = $reset->{'ROOM_VNUM'};

      } elsif( $reset->{'Name'} eq 'LEAD' ) {
        $last_mob = $reset->{'MOB_VNUM'};
        printf DUMP "Load Mobile \"%s\"[#%d] to \"%s\"[#%d], led by Mobile \"%s\"[#%d]%s\n",
               mob_name($data, $last_mob), $last_mob,
               room_name($data, $last_room), $last_room,
               mob_name($data, $last_led), $last_led,
               ($reset->{'DO_GROUP'} ? " (grouped)" : "");

      } elsif( $reset->{'Name'} eq 'HATE' ) {
        printf DUMP "Cause Mobile \"%s\"[#%d] in \"%s\"[#%d] to HATE %s\n",
               mob_name($data, $last_mob), $last_mob,
               room_name($data, $last_room), $last_room,
               "Something";

      } else {
      }
      # pick apart commands here

      printf DUMP "\n";


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

