#!/usr/bin/perl -w

package MudConvert::WileyMUD::Output;

use strict;
use English;
use Data::Dumper;
use MudConvert::Utils                   qw( vnum_index_file report_error );
use MudConvert::WileyMUD::Constants     qw( $sector_types $rev_sector_types
                                            $room_flags $rev_room_flags
                                            $zone_commands
                                            $exit_directions $rev_exit_directions $rev_exit_types
                                            $zone_reset_flags $rev_zone_reset_flags
                                        );
use MudConvert::API                     qw( flags_value
                                        );

our ($dump_dir, $dump_zone_file, $dump_world_file) = (undef,undef,undef);

use base 'Exporter';

our @EXPORT_OK = qw( dump_game dump_zones dump_rooms dump_mobs dump_objects dump_shops );

sub dump_game {
    my $cfg = shift;
    my $data = shift;
    my $results = {};

    $dump_dir = $cfg->{'destination-dir'}."/WileyMUD";
    if( !(-d $dump_dir) && !(mkdir $dump_dir) ) {
        #system("echo rm -rf ".$cfg->{'destination-dir'}.'/WileyMUD' $cfg->{'destination-dir'}.'/WileyMUD.old');
        #system("echo mv ".$cfg->{'destination-dir'}.'/WileyMUD'." ".$cfg->{'destination-dir'}.'/WileyMUD.old');
        printf STDERR "FATAL: Cannot create output directory (%s) for WileyMUD!\n", $dump_dir;
        #return undef;
    }
    $dump_zone_file     = "$dump_dir/newworld.zon";
    $dump_world_file    = "$dump_dir/newworld.wld";

    $results->{Zones}   = dump_zones($cfg, $data);
    $results->{Rooms}   = dump_rooms($cfg, $data);
    $results->{Mobs}    = dump_mobs($cfg, $data);
    $results->{Objects} = dump_objects($cfg, $data);
    $results->{Shops}   = dump_shops($cfg, $data);

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
        printf DUMP "#%d\n",        $vnum;
        printf DUMP "%s~\n",        $zone_data->{$vnum}->{'Name'};
        printf DUMP "%d %d %d\n",   $zone_data->{$vnum}->{'Top'},
                                    $zone_data->{$vnum}->{'Time'},
                                    $rev_zone_reset_flags->{$zone_data->{$vnum}->{'Mode'}};
        foreach my $reset (sort { $a->{'Number'} <=> $b->{'Number'} } @{ $zone_data->{$vnum}->{'Resets'} }) {
            my $tmp = "";

            $tmp .= sprintf "%s",   $reset->{'Command'};
            $tmp .= sprintf " %s",  join(" ", @{ $reset->{'Args'} }) if (defined $reset->{'Args'}) and scalar(@{ $reset->{'Args'} }) > 0;

            if (defined $reset->{'Comment'}) {
                printf DUMP "%-30s * %s\n", $tmp, $reset->{'Comment'};
            } else {
                printf DUMP "%s\n", $tmp;
            }
        }
        printf DUMP "%s\n\n",    "S";
    }
    printf DUMP "%s\n",        "\$~";
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
        printf DUMP "#%d\n",        $vnum;
        printf DUMP "%s~\n",        $room_data->{$vnum}->{'Name'};
        printf DUMP "%s\n~\n",      join "\n", @{ $room_data->{$vnum}->{'Description'} };
        printf DUMP "%d %d %d",     $room_data->{$vnum}->{'Zone'},
                                    flags_value($room_data->{$vnum}->{'Flags'}, $rev_room_flags),
                                    $rev_sector_types->{$room_data->{$vnum}->{'Sector'}};
        if( $room_data->{$vnum}->{'Sector'} eq 'SECT_TELEPORT' ) {
            # TeleportTime, TeleportTo, TeleportLook, TeleportSector
            printf DUMP " %d %d %d %d",     $room_data->{$vnum}->{'TeleportTime'},
                                            $room_data->{$vnum}->{'TeleportTo'},
                                            $room_data->{$vnum}->{'TeleportLook'},
                                            $rev_sector_types->{$room_data->{$vnum}->{'TeleportSector'}};

        } elsif( $room_data->{$vnum}->{'Sector'} eq 'SECT_WATER_NOSWIM' ) {
            # NOSWIM means you don't need swimming or a boat to enter, a river or stream...
            # RiverSpeed, RiverDirection 
            printf DUMP " %d %d",           $room_data->{$vnum}->{'RiverSpeed'},
                                            $rev_exit_directions->{$room_data->{$vnum}->{'RiverDirection'}};
        }
        printf DUMP "\n";

        if( exists $room_data->{$vnum}->{'SoundOne'} ) {
            my $sound = join "\n", @{ $room_data->{$vnum}->{'SoundOne'} };
            $sound .= "\n" if length $sound > 0;
            printf DUMP "%s~\n", $sound;
        }
        if( exists $room_data->{$vnum}->{'SoundTwo'} ) {
            my $sound = join "\n", @{ $room_data->{$vnum}->{'SoundTwo'} };
            $sound .= "\n" if length $sound > 0;
            printf DUMP "%s~\n", $sound;
        }

        if( exists $room_data->{$vnum}->{'Exits'} ) {
            my $exits = $room_data->{$vnum}->{'Exits'};
            foreach my $exit (sort { $exits->{$a}->{'Number'} <=> $exits->{$b}->{'Number'} } (keys %{ $exits })) {
                printf DUMP "D%d\n",        $rev_exit_directions->{ $exits->{$exit}->{'Direction'} };
                printf DUMP "%s~\n",        $exits->{$exit}->{'Description'};
                printf DUMP "%s~\n",        $exits->{$exit}->{'Keywords'};
                printf DUMP "%d %d %d\n",   $rev_exit_types->{ $exits->{$exit}->{'ExitType'} },
                                            $exits->{$exit}->{'KeyNumber'},
                                            $exits->{$exit}->{'DestinationVNum'};
            }
        }
        if( exists $room_data->{$vnum}->{'ExtraDesc'} ) {
            my $xtra = $room_data->{$vnum}->{'ExtraDesc'};
            #foreach my $extra (keys %{ $xtra }) {
            foreach my $extra (sort { $xtra->{$a}->{'Number'} <=> $xtra->{$b}->{'Number'} } (keys %{ $xtra })) {
                printf DUMP "E\n";
                printf DUMP "%s~\n",        $xtra->{$extra}->{'Keywords'};
                printf DUMP "%s~\n",        $xtra->{$extra}->{'Description'};
            }
        }
        printf DUMP "S\n";
    }

    printf DUMP "\$~\n";
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

