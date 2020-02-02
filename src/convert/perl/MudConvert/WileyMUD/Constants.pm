#!/usr/bin/perl -w

package MudConvert::WileyMUD::Constants;

use strict;
use English;
use base 'Exporter';

our @EXPORT_OK =  qw(   $zone_reset_flags $rev_zone_reset_flags
                        $equip_positions $rev_equip_positions
                        $door_states $rev_door_states
                        $hate_types $rev_hate_types
                        $fear_types $rev_fear_types
                        $sector_types $rev_sector_types
                        $room_flags $rev_room_flags
                        $zone_commands
                        $exit_directions $rev_exit_directions
                        $exit_types $rev_exit_types
                        $exit_flags $rev_exit_flags
                        $shop_sell_item_count $shop_buy_item_count $shop_message_count
                        $shop_attitudes $rev_shop_attitudes
                        $shop_immortal_flags $rev_shop_immortal_flags
                        $shop_message_names
                        $act_flags $rev_act_flags
                        $class_types $rev_class_types
);

our $zone_reset_flags = {
    0   => 'RESET_NEVER',
    1   => 'RESET_PC',
    2   => 'RESET_ALWAYS',
};
our $rev_zone_reset_flags = { reverse %{ $zone_reset_flags } };
$rev_zone_reset_flags->{$_} = int($rev_zone_reset_flags->{$_}) foreach (keys %{ $rev_zone_reset_flags });

our $equip_positions = {
    0   => 'WEAR_LIGHT',
    1   => 'WEAR_FINGER_R',
    2   => 'WEAR_FINGER_L',
    3   => 'WEAR_NECK_1',
    4   => 'WEAR_NECK_2',
    5   => 'WEAR_BODY',
    6   => 'WEAR_HEAD',
    7   => 'WEAR_LEGS',
    8   => 'WEAR_FEET',
    9   => 'WEAR_HANDS',
    10  => 'WEAR_ARMS',
    11  => 'WEAR_SHIELD',
    12  => 'WEAR_ABOUT',
    13  => 'WEAR_WAISTE',
    14  => 'WEAR_WRIST_R',
    15  => 'WEAR_WRIST_L',
    16  => 'WIELD',
    17  => 'HOLD',
    18  => 'WIELD_TWOH',
};
our $rev_equip_positions = { reverse %{ $equip_positions } };
$rev_equip_positions->{$_} = int($rev_equip_positions->{$_}) foreach (keys %{ $rev_equip_positions });

our $door_states = {
    0   => 'DOOR_OPEN',
    1   => 'DOOR_CLOSED',
    2   => 'DOOR_LOCKED',
};
our $rev_door_states = { reverse %{ $door_states } };
$rev_door_states->{$_} = int($rev_door_states->{$_}) foreach (keys %{ $rev_door_states });

our $exit_directions = {
    -1  => 'EXIT_NONE',
    0   => 'EXIT_NORTH',
    1   => 'EXIT_EAST',
    2   => 'EXIT_SOUTH',
    3   => 'EXIT_WEST',
    4   => 'EXIT_UP',
    5   => 'EXIT_DOWN',
};
our $rev_exit_directions = { reverse %{ $exit_directions } };
$rev_exit_directions->{$_} = int($rev_exit_directions->{$_}) foreach (keys %{ $rev_exit_directions });

our $exit_types = {
    -1  => 'EXIT_INVALID',
    0   => 'EXIT_OPEN',
    1   => 'EXIT_DOOR',
    2   => 'EXIT_NOPICK',
    3   => 'EXIT_SECRET',
    4   => 'EXIT_SECRET_NOPICK',
    5   => 'EXIT_OPEN_ALIAS',
    6   => 'EXIT_DOOR_ALIAS',
    7   => 'EXIT_NOPICK_ALIAS',
    8   => 'EXIT_SECRET_ALIAS',
    9   => 'EXIT_SECRET_NOPICK_ALIAS',
};
our $rev_exit_types = { reverse %{ $exit_types } };
$rev_exit_types->{$_} = int($rev_exit_types->{$_}) foreach (keys %{ $rev_exit_types });

our $exit_flags = {
    (1 << 0)    => 'EXITFLAG_DOOR',
    (1 << 1)    => 'EXITFLAG_NOPICK',
    (1 << 2)    => 'EXITFLAG_SECRET',
    (1 << 3)    => 'EXITFLAG_ALIAS',
};
our $rev_exit_flags = { reverse %{ $exit_flags } };
$rev_exit_flags->{$_} = int($rev_exit_flags->{$_}) foreach (keys %{ $rev_exit_flags });

our $room_flags = {
    (1 << 0)    => 'ROOM_DARK',
    (1 << 1)    => 'ROOM_DEATH',
    (1 << 2)    => 'ROOM_NOMOB',
    (1 << 3)    => 'ROOM_INDOORS',
    (1 << 4)    => 'ROOM_NOATACK',
    (1 << 5)    => 'ROOM_NOSTEAL',
    (1 << 6)    => 'ROOM_NOSUMMON',
    (1 << 7)    => 'ROOM_NOMAGIC',
    (1 << 8)    => 'ROOM_UNUSED',
    (1 << 9)    => 'ROOM_PRIVATE',
    (1 << 10)   => 'ROOM_SOUND',
};
our $rev_room_flags = { reverse %{ $room_flags } };
$rev_room_flags->{$_} = int($rev_room_flags->{$_}) foreach (keys %{ $rev_room_flags });

our $sector_types = {
    -1  => 'SECT_TELEPORT',
    0   => 'SECT_INDOORS',
    1   => 'SECT_CITY',
    2   => 'SECT_FIELD',
    3   => 'SECT_FOREST',
    4   => 'SECT_HILLS',
    5   => 'SECT_MOUNTAIN',
    6   => 'SECT_WATER_SWIM',
    7   => 'SECT_WATER_NOSWIM',
    8   => 'SECT_AIR',
    9   => 'SECT_UNDERWATER',
};
our $rev_sector_types = { reverse %{ $sector_types } };
$rev_sector_types->{$_} = int($rev_sector_types->{$_}) foreach (keys %{ $rev_sector_types });

our $hate_types = {
    1       => 'HATE_SEX',
    2       => 'HATE_RACE',
    4       => 'HATE_CHAR',
    8       => 'HATE_CLASS',
    16      => 'HATE_EVIL',
    32      => 'HATE_GOOD',
    64      => 'HATE_VNUM',
    128     => 'HATE_RICH',
};
our $rev_hate_types = { reverse %{ $hate_types } };
$rev_hate_types->{$_} = int($rev_hate_types->{$_}) foreach (keys %{ $rev_hate_types });

our $fear_types = {
    1       => 'FEAR_SEX',
    2       => 'FEAR_RACE',
    4       => 'FEAR_CHAR',
    8       => 'FEAR_CLASS',
    16      => 'FEAR_EVIL',
    32      => 'FEAR_GOOD',
    64      => 'FEAR_VNUM',
    128     => 'FEAR_RICH',
};
our $rev_fear_types = { reverse %{ $fear_types } };
$rev_fear_types->{$_} = int($rev_fear_types->{$_}) foreach (keys %{ $rev_fear_types });

our $zone_commands = {
    'M' => { 'Command' => 'M', 'Name' => 'MOBILE', 'Args' => [ 'IFF_FLAG', 'MOB_VNUM', 'MAX_COUNT', 'ROOM_VNUM' ] },
    'O' => { 'Command' => 'O', 'Name' => 'OBJECT', 'Args' => [ 'IFF_FLAG', 'OBJ_VNUM', 'MAX_COUNT', 'ROOM_VNUM' ] },
    'G' => { 'Command' => 'G', 'Name' => 'GIVE',   'Args' => [ 'IFF_FLAG', 'OBJ_VNUM', 'MAX_COUNT' ] },
    'E' => { 'Command' => 'E', 'Name' => 'EQUIP',  'Args' => [ 'IFF_FLAG', 'OBJ_VNUM', 'MAX_COUNT', 'EQUIP_POS' ] },
    'P' => { 'Command' => 'P', 'Name' => 'PUT',    'Args' => [ 'IFF_FLAG', 'SRC_OBJ', 'MAX_COUNT', 'DEST_OBJ' ] },
    'D' => { 'Command' => 'D', 'Name' => 'DOOR',   'Args' => [ 'IFF_FLAG', 'ROOM_VNUM', 'EXIT_DIR', 'DOOR_STATE' ] },
    'R' => { 'Command' => 'R', 'Name' => 'REMOVE', 'Args' => [ 'IFF_FLAG', 'ROOM_VNUM', 'OBJ_VNUM' ] },
    'L' => { 'Command' => 'L', 'Name' => 'LEAD',   'Args' => [ 'IFF_FLAG', 'MOB_VNUM', 'DO_GROUP' ] },
    'H' => { 'Command' => 'H', 'Name' => 'HATE',   'Args' => [ 'IFF_FLAG', 'HATE_TYPE', 'HATE_VALUE' ] },
    'S' => { 'Command' => 'S', 'Name' => 'END',    'Args' => [ ] },
};

our $shop_sell_item_count = 5;
our $shop_buy_item_count = 5;
our $shop_message_count = 7;

our $shop_attitudes = {
    0   => 'SHOP_RUDE',
    1   => 'SHOP_ALOOF',
};
our $rev_shop_attitudes = { reverse %{ $shop_attitudes } };
$rev_shop_attitudes->{$_} = int($rev_shop_attitudes->{$_}) foreach (keys %{ $rev_shop_attitudes });

our $shop_immortal_flags = {
    0   => 'SHOP_FIRSTSTRIKE',
    1   => 'SHOP_IMMORTAL',
};
our $rev_shop_immortal_flags = { reverse %{ $shop_immortal_flags } };
$rev_shop_immortal_flags->{$_} = int($rev_shop_immortal_flags->{$_}) foreach (keys %{ $rev_shop_immortal_flags });

our $shop_message_names = [
    'SHOP_MSG_NO_SUCH_ITEM_1',
    'SHOP_MSG_NO_SUCH_ITEM_2',
    'SHOP_MSG_DO_NOT_BUY',
    'SHOP_MSG_MISSING_CASH_1',
    'SHOP_MSG_MISSING_CASH_2',
    'SHOP_MSG_BUY',
    'SHOP_MSG_SELL',
];

our $act_flags = {
    (1 << 0)    => 'ACT_SPEC',
    (1 << 1)    => 'ACT_SENTINEL',
    (1 << 2)    => 'ACT_SCAVENGER',
    (1 << 3)    => 'ACT_ISNPC',
    (1 << 4)    => 'ACT_NICE_THIEF',
    (1 << 5)    => 'ACT_AGGRESSIVE',
    (1 << 6)    => 'ACT_STAY_ZONE',
    (1 << 7)    => 'ACT_WIMPY',
    (1 << 8)    => 'ACT_ANNOYING',
    (1 << 9)    => 'ACT_HATEFUL',
    (1 << 10)   => 'ACT_AFRAID',
    (1 << 11)   => 'ACT_IMMORTAL',
    (1 << 12)   => 'ACT_HUNTING',
    (1 << 13)   => 'ACT_DEADLY',
    (1 << 14)   => 'ACT_POLYSELF',
    (1 << 15)   => 'ACT_POLYOTHER',
    (1 << 16)   => 'ACT_GUARDIAN',
    (1 << 17)   => 'ACT_USE_ITEM',
    (1 << 18)   => 'ACT_FIGHTER_MOVES',
    (1 << 19)   => 'ACT_FOOD_PROVIDE',
    (1 << 20)   => 'ACT_PROTECTOR',
    (1 << 21)   => 'ACT_MOUNT',
    (1 << 22)   => 'ACT_SWITCH',
};
our $rev_act_flags = { reverse %{ $act_flags } };
$rev_act_flags->{$_} = int($rev_act_flags->{$_}) foreach (keys %{ $rev_act_flags });

our $class_types = {
    (1 << 0)    => 'CLASS_MAGE',
    (1 << 1)    => 'CLASS_CLERIC',
    (1 << 2)    => 'CLASS_WARRIOR',
    (1 << 3)    => 'CLASS_THIEF',
    (1 << 4)    => 'CLASS_RANGER',
    (1 << 5)    => 'CLASS_DRUID',
};
our $rev_class_types = { reverse %{ $class_types } };
$rev_class_types->{$_} = int($rev_class_types->{$_}) foreach (keys %{ $rev_class_types });

1;
