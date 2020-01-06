#!/usr/bin/perl -w

package MudConvert::WileyMUD::Input;

use strict;
use English;
use Data::Dumper;
use MudConvert::Utils qw( float vnum_index_file report_error );
use MudConvert::WileyMUD::Constants qw( $sector_types $rev_sector_types
                                        $room_flags $rev_room_flags
                                        $zone_commands
                                        $exit_directions $rev_exit_directions
                                        $zone_reset_flags $rev_zone_reset_flags
                                        $door_states $equip_positions
                                        $exit_types $exit_flags
                                        $shop_sell_item_count $shop_buy_item_count
                                        $shop_message_count
                                        $shop_attitudes $rev_shop_attitudes
                                        $shop_immortal_flags $rev_shop_immortal_flags
                                        $shop_message_names
                                      );
use MudConvert::API qw( exit_flag_list );

use base 'Exporter';

our @EXPORT_OK = qw( load_game load_zones load_rooms load_mobs load_objects load_shops );

sub load_game {
    my $cfg = shift;
    my $input_data = {};

    $input_data->{'Zones'} = load_zones($cfg);
    $input_data->{'Rooms'} = load_rooms($cfg);
    $input_data->{'Mobs'}  = load_mobs($cfg);
    $input_data->{'Objects'} = load_objects($cfg);
    $input_data->{'Shops'} = load_shops($cfg);

    return $input_data;
}

sub load_zones {
    my $cfg = shift;

    my $zone_file = $cfg->{'source-dir'}.'/tinyworld.zon';
    my $zone_data = vnum_index_file($cfg, $zone_file);
    print "Parsing Zone file..." if !$cfg->{'quiet'};
    open FP, $zone_file;
    foreach my $vnum (keys %{ $zone_data }) {
        seek FP, $zone_data->{$vnum}->{'BytePos'}, 0;
        my @line_set = ();
        my $line = $zone_data->{$vnum}->{'Line'};
        while(<FP>) {
            chomp;
            push @line_set, $_;
            last if $_ =~ /^S$/;
            if( $_ =~ /^#\d+/ and $line != $zone_data->{$vnum}->{'Line'} ) {
                report_error($zone_data, $vnum, $_, pos($_), "WARNING", "Invalid ZONE data");
                last;
            }
            $line++;
        }
        $zone_data->{$vnum}->{'file_section'} = join("\n", @line_set);
    }
    close FP;

    # Done reading the section in, now let's pick at it!

    foreach my $vnum (sort { $a <=> $b } keys %{ $zone_data }) {
        $zone_data->{$vnum}->{'Source'} = 'WileyMUD';
        $zone_data->{$vnum}->{'Resets'} = [];
        my @line_set = split /\n/, $zone_data->{$vnum}->{'file_section'};

        # Already have vnum, so skip line 0.
        for( my $i = 1; $i < scalar(@line_set); $i++ ) {
            if( $i == 0 ) {  # VNUM line, skip since we already have it.
                next;
            } elsif( $i == 1 ) { # Name line.
                $line_set[$i] =~ /(^.*)~$/;
                $zone_data->{$vnum}->{'Name'} = $1;
                if( !(defined $zone_data->{$vnum}->{'Name'}) ) {
                    report_error($zone_data, $vnum, $line_set[$i], pos($line_set[$i]), "FATAL", "Missing ZONE NAME");
                    last;
                }
            } elsif( $i == 2 ) { # Flag line.
                $line_set[$i] =~ /^\s*(\d+)\s+(\d+)\s+(\d+)\s*$/;
                ( $zone_data->{$vnum}->{'Top'},
                    $zone_data->{$vnum}->{'Time'},
                    $zone_data->{$vnum}->{'Mode'} ) = ( $1, $2, $zone_reset_flags->{$3} );
                if( !(defined $zone_data->{$vnum}->{'Top'}) or
                    !(defined $zone_data->{$vnum}->{'Time'}) or
                    !(defined $zone_data->{$vnum}->{'Mode'})
                ) {
                    report_error($zone_data, $vnum, $line_set[$i], pos($line_set[$i]), "FATAL", "Invalid ZONE FLAGS");
                    print STDERR "Skipping zone $vnum!\n";
                    last;
                }
            } else {   # Reset lines.
                my $reset = {};
                $line_set[$i] =~ /^\s*(\w)/;
                my $cmd = $1;
                $reset->{'Command'} = $cmd;
                $reset->{'Number'} = $i - 2;
                if( !(defined $cmd) ) {
                    # Regex failure... not the right format line at all!
                    report_error($zone_data, $vnum, $line_set[$i], pos($line_set[$i]), "FATAL", "Missing ZONE COMMAND");
                    printf STDERR "Skipping zone %d reset %d.\n", $vnum, $reset->{'Number'};
                    next;
                } elsif (!(defined $zone_commands->{$cmd}) or !(defined $zone_commands->{$cmd}->{'Name'}) ) {
                    # We have a command, but it isn't a valid one...
                    report_error($zone_data, $vnum, $line_set[$i], pos($line_set[$i]), "FATAL", "Unrecognized ZONE COMMAND");
                    printf STDERR "Skipping zone %d reset %d.\n", $vnum, $reset->{'Number'};
                    next;
                } elsif( $zone_commands->{$cmd}->{'Name'} eq 'END' ) {
                    # End of the show, nothing more to see here.
                    last;
                } else {
                    # A live one!  Catch him quick!
                    $reset->{'Name'} = $zone_commands->{$cmd}->{'Name'};
                    my $found_error = 0;
                    my $argcnt = scalar(@{ $zone_commands->{$cmd}->{'Args'} });
                    my $tmp = $line_set[$i];
                    $reset->{'Args'} = [];
                    $tmp =~ s/^\s*\w//; # Strip the leading command so we can...
                    for( my $j = 0; ($j < $argcnt) and ($tmp =~ /\s+(\d+)/g); $j++ ) {
                        # Loop through the integers automagically.
                        $reset->{$zone_commands->{$cmd}->{'Args'}->[$j]} = $1;
                        push @{ $reset->{'Args'} }, $1;
                        if( !(defined $reset->{$zone_commands->{$cmd}->{'Args'}->[$j]}) ) {
                            report_error($zone_data, $vnum, $line_set[$i], pos($line_set[$i]), "FATAL", "Invalid ZONE COMMAND ARGUMENTS");
                            printf STDERR "Skipping zone %d reset %d.\n", $vnum, $reset->{'Number'};
                            $found_error = 1;
                        }
                    }
                    next if $found_error;
                    $reset->{'Comment'} = $1 if($tmp =~ /.*?\*\s*(.*?)\s*$/g);
                }
                # If we get here, it means we read in a valid format zone command.
                # Now we need to convert data to a more portable format...
                if( $reset->{'Name'} eq 'DOOR' ) {
                    $reset->{'EXIT_DIR'} = $exit_directions->{$reset->{'EXIT_DIR'}};
                    $reset->{'DOOR_STATE'} = $door_states->{$reset->{'DOOR_STATE'}};
                } elsif( $reset->{'Name'} eq 'EQUIP' ) {
                    $reset->{'EQUIP_POS'} = $equip_positions->{$reset->{'EQUIP_POS'}};
                }

                push @{ $zone_data->{$vnum}->{'Resets'} }, $reset;
            }
        }
    }

    printf("done\nLoaded %d zones.\n", scalar(keys %{ $zone_data })) if !$cfg->{'quiet'};

    return $zone_data;
}

sub parse_room {
    my $cfg = shift;
    my $room_data = shift;
    my $vnum = shift;

    my ( $t1, $t2, $t3, $t4, $t5, $t6, $t7, $t8, $t9 ) = ( undef, undef, undef, undef, undef, undef, undef, undef, undef );

    my ($Name, $Description, $Zone, $Flags, $Sector) = (undef, [], undef, undef, undef);
    my ($TeleportTime, $TeleportTo, $TeleportLook, $TeleportSector) = (undef, undef, undef, undef);
    my ($RiverSpeed, $RiverDirection) = (undef, undef);
    my ($SoundOne, $SoundTwo) = ([], []);
    my ($Exits, $ExtraDesc) = ({}, {});

    my $room = $room_data->{$vnum}->{'file_section'};

    #  $room =~
    #    /^\#(?:[\d-]+)\s*\n      # VNum
    #      ([^~]*)~\n      # Room Name
    #      ([^~]*)~\n      # Description
    #      \s*([\d-]+)\s+([\d-]+)\s+([\d-]+)    # Zone, Flags, Sector
    #         (?:\s+([\d-]+))?(?:\s+([\d-]+))?   # Teleport or River
    #         (?:\s+([\d-]+))?(?:\s+([\d-]+))?\s*\n
    #    /cgmsx;
    #
    #  ( $Name, $t2,
    #    $Zone, $Flags, $Sector,
    #    $t6, $t7, $t8, $t9,
    #  ) = ( $1, $2, $3, $4, $5, $6, $7, $8, $9 );

    # Just eat the VNUM, we already have it...
    if( !($room =~ /^\#(?:[\d-]+)\s*\n/cgmsx) ) {   # VNum
        report_error($room_data, $vnum, $room, pos($room), "FATAL", "Missing ROOM VNUM");
        print STDERR "Skipping room $vnum!\n";
        return 0;
    }

    if( !($room =~ /\G([^~]*)~+\n/cgmsx) ) {   # Room Name
        report_error($room_data, $vnum, $room, pos($room), "FATAL", "Missing ROOM NAME");
        print STDERR "Skipping room $vnum!\n";
        return 0;
    } else {
        $Name = $1;
    }

    if( !($room =~ /\G([^~]*)~+\n/cgmsx) ) {   # Description
        report_error($room_data, $vnum, $room, pos($room), "FATAL", "Missing ROOM DESCRIPTION");
        print STDERR "Skipping room $vnum!\n";
        return 0;
    } else {
        push @{ $Description }, (split /\n/, $1);
    }

    if( !($room =~ /\G\s*([\d-]+)/cgmsx) ) {   # Zone
        report_error($room_data, $vnum, $room, pos($room), "FATAL", "Invalid ROOM ZONE NUMBER");
        print STDERR "Skipping room $vnum!\n";
        return 0;
    } else {
        $Zone = $1;
        if( $Zone != int($vnum / 100) ) {
            report_error($room_data, $vnum, $room, pos($room), "WARNING", "ROOM ZONE NUMBER mismatch");
        }
    }

    if( !($room =~ /\G\s+([\d-]+)/cgmsx) ) {   # Flags
        report_error($room_data, $vnum, $room, pos($room), "FATAL", "Invalid ROOM FLAGS");
        print STDERR "Skipping room $vnum!\n";
        return 0;
    } else {
        $Flags = $1;
        if( $Flags & int($rev_room_flags->{'ROOM_DEATH'}) ) {
            report_error($room_data, $vnum, $room, pos($room), "WARNING", "DEATH ROOM detected");
        }
    }

    if( !($room =~ /\G\s+([\d-]+)/cgmsx) ) {   # Sector
        report_error($room_data, $vnum, $room, pos($room), "FATAL", "Invalid ROOM SECTOR");
        print STDERR "Skipping room $vnum!\n";
        return 0;
    } else {
        $Sector = $sector_types->{$1};
        if( !(defined $Sector) ) {
            report_error($room_data, $vnum, $room, pos($room), "WARNING", "Unknown ROOM SECTOR TYPE", 1);
            print STDERR "Setting to ".$sector_types->{0}.".\n";
            $Sector = $sector_types->{0};
        }

        if( $Sector eq 'SECT_TELEPORT' ) {
            # Fine, teleport we must... but where?

            if( !($room =~ /\G\s+([\d-]+)/cgmsx) ) {   # Teleport Time
                report_error($room_data, $vnum, $room, pos($room), "FATAL", "Invalid ROOM TELEPORT TIME value");
                print STDERR "Skipping room $vnum!\n";
                return 0;
            } else {
                $TeleportTime = $1;
            }

            if( !($room =~ /\G\s+([\d-]+)/cgmsx) ) {   # Teleport To
                report_error($room_data, $vnum, $room, pos($room), "FATAL", "Invalid ROOM TELEPORT DESTINATION");
                print STDERR "Skipping room $vnum!\n";
                return 0;
            } else {
                $TeleportTo = $1;
            }

            if( !($room =~ /\G\s+([\d-]+)/cgmsx) ) {   # Teleport Look
                report_error($room_data, $vnum, $room, pos($room), "FATAL", "Invalid ROOM TELEPORT LOOK flag");
                print STDERR "Skipping room $vnum!\n";
                return 0;
            } else {
                $TeleportLook = $1;
            }

            if( !($room =~ /\G\s+([\d-]+)/cgmsx) ) {   # Teleport Sector
                report_error($room_data, $vnum, $room, pos($room), "FATAL", "Invalid ROOM TELEPORT SECTOR value");
                print STDERR "Skipping room $vnum!\n";
                return 0;
            } else {
                $TeleportSector = $sector_types->{$1};
            }

            if( !(defined $TeleportSector) ) {
                report_error($room_data, $vnum, $room, pos($room), "WARNING", "Unknown ROOM TELEPORT SECTOR TYPE", 1);
                print STDERR "Setting to ".$sector_types->{0}.".\n";
                $TeleportSector = $sector_types->{0};
            }
            #$room_data->{$vnum}->{'Sector'} = $rev_sector_types->{'SECT_FIELD'};
        } elsif( $Sector eq 'SECT_WATER_NOSWIM' ) {
            # Shallow Water (in Wiley) means river... which has to flow somewhere

            if( !($room =~ /\G\s+([\d-]+)/cgmsx) ) {   # River Speed
                report_error($room_data, $vnum, $room, pos($room), "FATAL", "Invalid ROOM RIVER SPEED value");
                print STDERR "Skipping room $vnum!\n";
                return 0;
            } else {
                $RiverSpeed = $1;
            }

            if( !($room =~ /\G\s+([\d-]+)/cgmsx) ) {   # River Direction
                report_error($room_data, $vnum, $room, pos($room), "FATAL", "Invalid ROOM RIVER DIRECTION");
                print STDERR "Skipping room $vnum!\n";
                return 0;
            } else {
                $RiverDirection = $1;
            }
        }

        # Read the trailing newline, regardless of sector type, to advance the pointer
        $room =~ /\G\s*\n/cgmsx;
    }


    if( $Flags & int($rev_room_flags->{'ROOM_SOUND'}) ) {
        $room =~ /\G([^~]*)~+\n([^~]*)~+\n/cgmsx;  # Two room sound lines
        ( $t1, $t2 ) = ( $1, $2 );
        if( !(defined $t1) or !(defined $t2) ) {
            report_error($room_data, $vnum, $room, pos($room), "FATAL", "Invalid ROOM SOUND DATA");
            print STDERR "Skipping room $vnum!\n";
            return 0;
        } else {
            if( $t1 !~ /^D\d+\s*\n/ and $t1 !~ /^E\s*\n/ and $t1 !~ /^S\s*\n/ ) {
                push @{ $SoundOne }, (split /\n/, $t1);
                push @{ $SoundTwo }, (split /\n/, $t2);
            } else { # We found doors or extras or the end instead of sounds!
                report_error($room_data, $vnum, $room, pos($room), "WARNING", "Missing ROOM SOUNDS");
                print STDERR "Setting SOUND bit to NULL for room $vnum!\n";
                $Flags ^= int($rev_room_flags->{'ROOM_SOUND'});
            }
        }
    }

    #print STDERR "DEBUG:  room == $room\n";

    my $j = 0;
    while(
        $room =~
        /\GD(\d+)\s*\n    # Exit number
        ([^~]*)~+\n    # Exit description
        ([^~]*)~+\n    # Exit keywords
        \s*([\d-]+)\s+([\d-]+)\s+([\d-]+)\s*\n # Type, Key Number, Target Room
        /cgmsx
    ) {
        ($t1,$t2,$t3,$t4,$t5,$t6) = ($1,$2,$3,$4,$5,$6);

        if( !(defined $t1) or !(defined $t2) or !(defined $t3) ) {
            report_error($room_data, $vnum, $room, pos($room), "FATAL", "Invalid EXIT DATA");
            print STDERR "Skipping room $vnum!\n";
            return 0;
        }
        $Exits->{$exit_directions->{$t1}} = {
            'Number'  => $j,
            'Direction'  => $exit_directions->{$t1},
            'Description'  => $t2,
            'Keywords'  => $t3,
            'ExitFlags'  => exit_flag_list($exit_types->{$t4}),
            'ExitType'  => $exit_types->{$t4},
            'KeyNumber'  => $t5,
            'DestinationVNum' => $t6,
        };
        $j++;
    }

    $j = 0;
    while(
        $room =~
        /\GE\s*\n     # Extra description!
        ([^~]*)~+\n    # Keyword list
        ([^~]*)~+\n    # Description
        /cgmsx
    ) {
        ($t1, $t2) = ($1, $2);
        if( !(defined $t1) or !(defined $t2) ) {
            report_error($room_data, $vnum, $room, pos($room), "FATAL", "Invalid EXTRA DESCRIPTION DATA");
            print STDERR "Skipping room $vnum!\n";
            return 0;
        }
        $ExtraDesc->{$t1} = {
            'Number'  => $j,
            'Keywords'  => $t1,
            'Description'  => $t2,
        };
        $j++;
    }

    # At this point we should be done... sanity checking should give us a lone S.

    #if( $room !~ /S\s*\n/ ) {
    #  printf STDERR "Terminating S tag not found for file %s, room %d, below line %d\n",
    #                $room_data->{$vnum}->{'Filename'},
    #                $vnum,
    #                $room_data->{$vnum}->{'Line'};
    #}

    $room_data->{$vnum}->{'Source'} = 'WileyMUD';
    $room_data->{$vnum}->{'Name'} = $Name;
    $room_data->{$vnum}->{'Description'} = $Description;
    $room_data->{$vnum}->{'Zone'} = $Zone;
    $room_data->{$vnum}->{'Flags'} = [];
    #$room_data->{$vnum}->{'Flags'}->{'Value'} = $Flags;
    for (my $i = 0; $i < 32; $i++) {
        push @{ $room_data->{$vnum}->{'Flags'} }, $room_flags->{1 << $i}
        if $Flags & (1 << $i);
    }
    $room_data->{$vnum}->{'Sector'} = $Sector;

    if( $Sector eq 'SECT_TELEPORT' ) {
        $room_data->{$vnum}->{'TeleportTime'} = $TeleportTime;
        $room_data->{$vnum}->{'TeleportTo'} = $TeleportTo;
        $room_data->{$vnum}->{'TeleportLook'} = $TeleportLook;
        $room_data->{$vnum}->{'TeleportSector'} = $TeleportSector;
    } elsif( $Sector eq 'SECT_WATER_NOSWIM' ) {
        $room_data->{$vnum}->{'RiverSpeed'} = $RiverSpeed;
        $room_data->{$vnum}->{'RiverDirection'} = $exit_directions->{$RiverDirection};
    }

    if( $Flags & int($rev_room_flags->{'ROOM_SOUND'}) ) {
        $room_data->{$vnum}->{'SoundOne'} = $SoundOne;
        $room_data->{$vnum}->{'SoundTwo'} = $SoundTwo;
    }

    if( scalar(keys %{ $Exits }) > 0 ) {
        $room_data->{$vnum}->{'Exits'} = $Exits
    } else {
        report_error($room_data, $vnum, $room, pos($room), "WARNING", "NO EXITS FOUND");
    }

    if( scalar(keys %{ $ExtraDesc }) > 0 ) {
        $room_data->{$vnum}->{'ExtraDesc'} = $ExtraDesc
    } else {
        #report_error($room_data, $vnum, $room, pos($room), "WARNING: NO EXTRA DESCRIPTIONS FOUND");
    }

    return 1;
}

sub load_rooms {
    my $cfg = shift;

    my $world_file = $cfg->{'source-dir'}.'/tinyworld.wld';
    my $room_data = vnum_index_file($cfg, $world_file);
    print "Parsing World file..." if !$cfg->{'quiet'};
    open FP, $world_file;
    foreach my $vnum (sort { $a <=> $b } keys %{ $room_data }) {
        seek FP, $room_data->{$vnum}->{'BytePos'}, 0;
        my @line_set = ();
        my $line = $room_data->{$vnum}->{'Line'};
        while(<FP>) {
            chomp;
            push @line_set, $_;
            last if $_ =~ /^S$/;
            if( $_ =~ /^#\d+/ and $line != $room_data->{$vnum}->{'Line'} ) {
                print STDERR "*** Invalid Room data at line $line!\n";
                print STDERR "    Attempting to continue...\n";
                last;
            }
            $line++;
        }
        $room_data->{$vnum}->{'file_section'} = join("\n", @line_set);
    }
    close FP;

    my $room_count = 0;
    foreach my $vnum (sort { $a <=> $b } keys %{ $room_data }) {
        if( !parse_room($cfg, $room_data, $vnum) ) {
            delete $room_data->{$vnum};
            print STDERR "FATAL: Skipping ROOM $vnum!\n";
        } else {
            $room_count++;
        }
    }

    print "done\nLoaded $room_count rooms.\n" if !$cfg->{'quiet'};

    return $room_data;
}

sub parse_mob {
    my $cfg = shift;
    my $mob_data = shift;
    my $vnum = shift;

    my ( $t1, $t2, $t3, $t4, $t5, $t6, $t7, $t8, $t9 ) = ( undef, undef, undef, undef, undef, undef, undef, undef, undef );

    my $mob = $mob_data->{$vnum}->{'file_section'};

    #  "#1\n
    #  Astral traveler~\n
    #  An astral traveler~\n
    #  ~\n~\n
    #  1 0 1000 S\n
    #  26 1 -1 26d8+26 1d8+3\n
    #  -1 0 15000 27\n
    #  8 8 2\n
    #  #2"

    #  $room =~
    #    /^\#(?:[\d-]+)\s*\n      # VNum
    #      ([^~]*)~\n      # Room Name
    #      ([^~]*)~\n      # Description
    #      \s*([\d-]+)\s+([\d-]+)\s+([\d-]+)    # Zone, Flags, Sector
    #         (?:\s+([\d-]+))?(?:\s+([\d-]+))?   # Teleport or River
    #         (?:\s+([\d-]+))?(?:\s+([\d-]+))?\s*\n
    #    /cgmsx;
    #
    #  ( $Name, $t2,
    #    $Zone, $Flags, $Sector,
    #    $t6, $t7, $t8, $t9,
    #  ) = ( $1, $2, $3, $4, $5, $6, $7, $8, $9 );
    return 1;
}

sub load_mobs {
    my $cfg = shift;

    my $mob_file = $cfg->{'source-dir'}.'/tinyworld.mob';
    my $mob_data = vnum_index_file($cfg, $mob_file);
    print "Parsing Mob file..." if !$cfg->{'quiet'};
    open FP, $mob_file;
    foreach my $vnum (sort { $a <=> $b } keys %{ $mob_data }) {
        seek FP, $mob_data->{$vnum}->{'BytePos'}, 0;
        my @line_set = ();
        my $line = $mob_data->{$vnum}->{'Line'};
        while(<FP>) {
            chomp;
            last if $_ =~ /^#\d+/ and $line != $mob_data->{$vnum}->{'Line'};
            push @line_set, $_;
            $line++;
        }
        $mob_data->{$vnum}->{'file_section'} = join("\n", @line_set);
    }
    close FP;

    my $mob_count = 0;
    foreach my $vnum (sort { $a <=> $b } keys %{ $mob_data }) {
        if( !parse_mob($cfg, $mob_data, $vnum) ) {
            delete $mob_data->{$vnum};
            print STDERR "FATAL: Skipping MOB $vnum!\n";
        } else {
            $mob_count++;
        }
    }

    print "done\nLoaded $mob_count mobs.\n" if !$cfg->{'quiet'};

    return $mob_data;
}

sub parse_obj {
    my $cfg = shift;
    my $obj_data = shift;
    my $vnum = shift;

    my ( $t1, $t2, $t3, $t4, $t5, $t6, $t7, $t8, $t9 ) = ( undef, undef, undef, undef, undef, undef, undef, undef, undef );

    my $obj = $obj_data->{$vnum}->{'file_section'};

    # "#4\n
    # meat hunk~\n
    # a hunk of meat~\n
    # A hunk of dead flesh sits here.~\n
    # ~\n
    # 19 131072 1\n
    # 6 0 0 0\n
    # 1 10 10\n
    # #5"

    return 1;
}

sub load_objects {
    my $cfg = shift;

    my $obj_file = $cfg->{'source-dir'}.'/tinyworld.obj';
    my $obj_data = vnum_index_file($cfg, $obj_file);
    print "Parsing World file..." if !$cfg->{'quiet'};
    open FP, $obj_file;
    foreach my $vnum (sort { $a <=> $b } keys %{ $obj_data }) {
        seek FP, $obj_data->{$vnum}->{'BytePos'}, 0;
        my @line_set = ();
        my $line = $obj_data->{$vnum}->{'Line'};
        while(<FP>) {
            chomp;
            last if $_ =~ /^#\d+/ and $line != $obj_data->{$vnum}->{'Line'};
            push @line_set, $_;
            $line++;
        }
        $obj_data->{$vnum}->{'file_section'} = join("\n", @line_set);
    }
    close FP;

    my $obj_count = 0;
    foreach my $vnum (sort { $a <=> $b } keys %{ $obj_data }) {
        if( !parse_obj($cfg, $obj_data, $vnum) ) {
            delete $obj_data->{$vnum};
            print STDERR "FATAL: Skipping OBJECT $vnum!\n";
        } else {
            $obj_count++;
        }
    }

    print "done\nLoaded $obj_count objects.\n" if !$cfg->{'quiet'};

    return $obj_data;
}

sub parse_shop {
    my $cfg = shift;
    my $shop_data = shift;
    my $vnum = shift;

    my ($SellItems, $SellProfit, $BuyProfit, $BuyItems) = ([], undef, undef, []);
    my ($Messages, $Attitude, $Immortal, $ShopkeeperVNum) = ({}, undef, undef, undef);
    my ($UnusedFlag, $RoomVNum, $OpenHour1, $CloseHour1, $OpenHour2, $CloseHour2) = (undef,undef,undef,undef,undef,undef);

    my $shop = $shop_data->{$vnum}->{'file_section'};

    # Just eat the VNUM, we already have it...
    if( !($shop =~ /^\#(?:[\d-]+)\s*\~?\s*\n/cgmsx) ) {  # VNum
        report_error($shop_data, $vnum, $shop, pos($shop), "FATAL", "Missing SHOP VNUM");
        return 0;
    }

    for( my $i = 1; $i <= $shop_sell_item_count; $i++) {
        if( !($shop =~ /\G\s*([\d-]+)\s*\n/cgmsx) ) {  # Goods for sale
            report_error($shop_data, $vnum, $shop, pos($shop), "FATAL", "Invalid SHOP SELL ITEM $i");
            return 0;
        } else {
            push @{ $SellItems }, $1;
        }
    }

    if( !($shop =~ /\G\s*([\d\.-]+)\s*\n/cgmsx) ) {  # Sell Profit
        report_error($shop_data, $vnum, $shop, pos($shop), "FATAL", "Invalid SHOP SELL PROFIT");
        return 0;
    } else {
        $SellProfit = $1;
    }

    if( !($shop =~ /\G\s*([\d\.-]+)\s*\n/cgmsx) ) {  # Buy Profit
        report_error($shop_data, $vnum, $shop, pos($shop), "FATAL", "Invalid SHOP BUY PROFIT");
        return 0;
    } else {
        $BuyProfit = $1;
    }

    for( my $i = 1; $i <= $shop_buy_item_count; $i++) {
        if( !($shop =~ /\G\s*([\d-]+)\s*\n/cgmsx) ) {  # Goods we purchase
            report_error($shop_data, $vnum, $shop, pos($shop), "FATAL", "Invalid SHOP BUY ITEM $i");
            return 0;
        } else {
            push @{ $BuyItems }, $1;
        }
    }

    for( my $i = 1; $i <= $shop_message_count; $i++) {
        if( !($shop =~ /\G([^~]*)~+\n/cgmsx) ) {   # Obnoxious messages 
            report_error($shop_data, $vnum, $shop, pos($shop), "FATAL", "Invalid SHOP MESSAGE $i");
            return 0;
        } else {
            $Messages->{$shop_message_names->[$i-1]} = $1;
        }
    }

    if( !($shop =~ /\G\s*([\d-]+)\s*\n/cgmsx) ) {   # Attitude
        report_error($shop_data, $vnum, $shop, pos($shop), "FATAL", "Invalid SHOP ATTITUDE ");
        return 0;
    } else {
        $Attitude = $1;
    }

    if( !($shop =~ /\G\s*([\d-]+)\s*\n/cgmsx) ) {   # Immortal Flag
        report_error($shop_data, $vnum, $shop, pos($shop), "FATAL", "Invalid SHOP IMMORTAL FLAG ");
        return 0;
    } else {
        $Immortal = $1;
    }

    if( !($shop =~ /\G\s*([\d-]+)\s*\n/cgmsx) ) {   # Shopkeeper VNum
        report_error($shop_data, $vnum, $shop, pos($shop), "FATAL", "Invalid SHOP SHOPKEEPER VNUM");
        return 0;
    } else {
        $ShopkeeperVNum = $1;
    }

    if( !($shop =~ /\G\s*([\d-]+)\s*\n/cgmsx) ) {   # Unused Flag
        report_error($shop_data, $vnum, $shop, pos($shop), "FATAL", "Invalid SHOP UNUSED FLAG");
        return 0;
    } else {
        $UnusedFlag = $1;
    }

    if( !($shop =~ /\G\s*([\d-]+)\s*\n/cgmsx) ) {   # Room VNum
        report_error($shop_data, $vnum, $shop, pos($shop), "FATAL", "Invalid SHOP ROOM VNUM");
        return 0;
    } else {
        $RoomVNum = $1;
    }

    if( !($shop =~ /\G\s*([\d-]+)\s*\n/cgmsx) ) {   # Open Hour
        report_error($shop_data, $vnum, $shop, pos($shop), "FATAL", "Invalid SHOP OPEN HOUR");
        return 0;
    } else {
        $OpenHour1 = $1;
    }

    if( !($shop =~ /\G\s*([\d-]+)\s*\n/cgmsx) ) {   # Close Hour
        report_error($shop_data, $vnum, $shop, pos($shop), "FATAL", "Invalid SHOP CLOSE HOUR");
        return 0;
    } else {
        $CloseHour1 = $1;
    }

    if( !($shop =~ /\G\s*([\d-]+)\s*\n/cgmsx) ) {   # Open Hour 2
        report_error($shop_data, $vnum, $shop, pos($shop), "FATAL", "Invalid SHOP OPEN HOUR 2");
        return 0;
    } else {
        $OpenHour2 = $1;
    }

    if( !($shop =~ /\G\s*([\d-]+)\s*/cgmsx) ) {   # Close Hour 2
        report_error($shop_data, $vnum, $shop, pos($shop), "FATAL", "Invalid SHOP CLOSE HOUR 2");
        return 0;
    } else {
        $CloseHour2 = $1;
    }

    $shop_data->{$vnum}->{'Source'} = 'WileyMUD';
    $shop_data->{$vnum}->{'Name'} = "Ye Olde Shoppe $vnum";   # Wiley shops don't have names
    $shop_data->{$vnum}->{'Description'} = "You see a shop full of stuff."; # No descriptions either
    $shop_data->{$vnum}->{'Zone'} = int($RoomVNum / 100); # Shops don't have zones, but their rooms do!
    $shop_data->{$vnum}->{'SellItems'} = $SellItems;
    $shop_data->{$vnum}->{'BuyItems'} = $BuyItems;
    $shop_data->{$vnum}->{'SellProfit'} = float($SellProfit);
    $shop_data->{$vnum}->{'BuyProfit'} = float($BuyProfit);
    $shop_data->{$vnum}->{'Messages'} = $Messages;
    $shop_data->{$vnum}->{'Attitude'} = $shop_attitudes->{$Attitude};
    $shop_data->{$vnum}->{'Immortal'} = $shop_immortal_flags->{$Immortal};
    $shop_data->{$vnum}->{'RoomVNum'} = $RoomVNum;
    $shop_data->{$vnum}->{'Hours'} = {  'Open' => [ $OpenHour1, $OpenHour2 ],
        'Close' => [ $CloseHour1, $CloseHour2 ], };

    return 1;
}

sub load_shops {
    my $cfg = shift;
    my $shop_file = $cfg->{'source-dir'}.'/tinyworld.shp';

    my $shop_data = vnum_index_file($cfg, $shop_file);
    print "Parsing Shop file..." if !$cfg->{'quiet'};
    open FP, $shop_file;
    foreach my $vnum (keys %{ $shop_data }) {
        seek FP, $shop_data->{$vnum}->{'BytePos'}, 0;
        my @line_set = ();
        my $line = $shop_data->{$vnum}->{'Line'};
        while(<FP>) {
            chomp;
            push @line_set, $_;
            if( $_ =~ /^\$\~/ or ($_ =~ /^#\d+/ and $line != $shop_data->{$vnum}->{'Line'}) ) {
                pop @line_set; # no record end marker, so toss the extra
                last;
            }
            $line++;
        }
        $shop_data->{$vnum}->{'file_section'} = join("\n", @line_set);
    }
    close FP;

    # Done reading the section in, now let's pick at it!

    my $shop_count = 0;
    foreach my $vnum (sort { $a <=> $b } keys %{ $shop_data }) {
        if( !parse_shop($cfg, $shop_data, $vnum) ) {
            delete $shop_data->{$vnum};
            print STDERR "FATAL: Skipping SHOP $vnum!\n";
        } else {
            $shop_count++;
        }
    }

    print "done\nLoaded $shop_count shops.\n" if !$cfg->{'quiet'};

    return $shop_data;
}

1;

