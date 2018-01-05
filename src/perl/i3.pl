#!/usr/bin/perl -w

use strict;
use utf8;
use Encode qw(encode_utf8);
no warnings 'utf8';

use POE;
use POE::Wheel::ReadLine;           # for the console interface.
use POE::Component::Server::TCP;    # for the telnet interface.
use POE::Filter::Stream;
use POE::Filter::Line;
use POE::Component::Client::TCP;    # to connect to our i3 server.
#use POE::Component::SimpleDBI;      # Async access to SQL database.
use DBI;
use Regexp::Grammars;
use Date::Manip::Date;
use Data::Dumper;
use Digest::MD5 qw(md5_hex);

my $DB_FILE = './i3.db';
my $db = undef;
my $result = undef;

# I3 coloring data

my $i3_normals = {
    '%^BLACK%^%^BOLD%^'         => '%^BOLD%^%^BLACK%^',
    '%^BLACK%^BOLD%^'           => '%^BOLD%^%^BLACK%^',
    '%^BOLD%^BLACK%^'           => '%^BOLD%^%^BLACK%^',
    '%^DARKGREY%^'              => '%^BOLD%^%^BLACK%^',

    '%^RED%^%^BOLD%^'           => '%^BOLD%^%^RED%^',
    '%^RED%^BOLD%^'             => '%^BOLD%^%^RED%^',
    '%^BOLD%^RED%^'             => '%^BOLD%^%^RED%^',
    '%^PINK%^'                  => '%^BOLD%^%^RED%^',
    '%^LIGHTRED%^'              => '%^BOLD%^%^RED%^',

    '%^GREEN%^%^BOLD%^'         => '%^BOLD%^%^GREEN%^',
    '%^GREEN%^BOLD%^'           => '%^BOLD%^%^GREEN%^',
    '%^BOLD%^GREEN%^'           => '%^BOLD%^%^GREEN%^',
    '%^LIGHTGREEN%^'            => '%^BOLD%^%^GREEN%^',

    '%^YELLOW%^%^BOLD%^'        => '%^YELLOW%^',
    '%^YELLOW%^BOLD%^'          => '%^YELLOW%^',
    '%^BOLD%^YELLOW%^'          => '%^YELLOW%^',

    '%^ORANGE%^%^BOLD%^'        => '%^YELLOW%^',
    '%^ORANGE%^BOLD%^'          => '%^YELLOW%^',
    '%^BOLD%^ORANGE%^'          => '%^YELLOW%^',
    '%^BOLD%^%^ORANGE%^'        => '%^YELLOW%^',

    '%^BLUE%^%^BOLD%^'          => '%^BOLD%^%^BLUE%^',
    '%^BLUE%^BOLD%^'            => '%^BOLD%^%^BLUE%^',
    '%^BOLD%^BLUE%^'            => '%^BOLD%^%^BLUE%^',
    '%^LIGHTBLUE%^'             => '%^BOLD%^%^BLUE%^',

    '%^MAGENTA%^%^BOLD%^'       => '%^BOLD%^%^MAGENTA%^',
    '%^MAGENTA%^BOLD%^'         => '%^BOLD%^%^MAGENTA%^',
    '%^BOLD%^MAGENTA%^'         => '%^BOLD%^%^MAGENTA%^',
    '%^PURPLE%^%^BOLD%^'        => '%^BOLD%^%^MAGENTA%^',
    '%^PURPLE%^BOLD%^'          => '%^BOLD%^%^MAGENTA%^',
    '%^BOLD%^PURPLE%^'          => '%^BOLD%^%^MAGENTA%^',
    '%^BOLD%^%^PURPLE%^'        => '%^BOLD%^%^MAGENTA%^',
    '%^LIGHTMAGENTA%^'          => '%^BOLD%^%^MAGENTA%^',
    '%^LIGHTPURPLE%^'           => '%^BOLD%^%^MAGENTA%^',

    '%^CYAN%^%^BOLD%^'          => '%^BOLD%^%^CYAN%^',
    '%^CYAN%^BOLD%^'            => '%^BOLD%^%^CYAN%^',
    '%^BOLD%^CYAN%^'            => '%^BOLD%^%^CYAN%^',
    '%^LIGHTCYAN%^'             => '%^BOLD%^%^CYAN%^',

    '%^WHITE%^%^BOLD%^'         => '%^BOLD%^%^WHITE%^',
    '%^WHITE%^BOLD%^'           => '%^BOLD%^%^WHITE%^',
    '%^BOLD%^WHITE%^'           => '%^BOLD%^%^WHITE%^',
    '%^BRIGHTWHITE%^'           => '%^BOLD%^%^WHITE%^',

    '%^GREY%^'                  => '%^WHITE%^',

    # Background colors (cannot be bold)

    '%^B_BLACK%^%^BOLD%^'       => '%^B_BLACK%^',
    '%^B_BLACK%^BOLD%^'         => '%^B_BLACK%^',
    '%^BOLD%^B_BLACK%^'         => '%^B_BLACK%^',
    '%^BOLD%^%^B_BLACK%^'       => '%^B_BLACK%^',
    '%^B_DARKGREY%^'            => '%^B_BLACK%^',

    '%^RED%^%^BOLD%^'           => '%^B_RED%^',
    '%^RED%^BOLD%^'             => '%^B_RED%^',
    '%^BOLD%^RED%^'             => '%^B_RED%^',
    '%^BOLD%^%^RED%^'           => '%^B_RED%^',
    '%^B_PINK%^'                => '%^B_RED%^',
    '%^LIGHTRED%^'              => '%^B_RED%^',

    '%^GREEN%^%^BOLD%^'         => '%^B_GREEN%^',
    '%^GREEN%^BOLD%^'           => '%^B_GREEN%^',
    '%^BOLD%^GREEN%^'           => '%^B_GREEN%^',
    '%^BOLD%^%^GREEN%^'         => '%^B_GREEN%^',
    '%^LIGHTGREEN%^'            => '%^B_GREEN%^',

    '%^B_YELLOW%^%^BOLD%^'      => '%^B_YELLOW%^',
    '%^B_YELLOW%^BOLD%^'        => '%^B_YELLOW%^',
    '%^BOLD%^B_YELLOW%^'        => '%^B_YELLOW%^',
    '%^BOLD%^%^B_YELLOW%^'      => '%^B_YELLOW%^',

    '%^B_ORANGE%^%^BOLD%^'      => '%^B_ORANGE%^',
    '%^B_ORANGE%^BOLD%^'        => '%^B_ORANGE%^',
    '%^BOLD%^B_ORANGE%^'        => '%^B_ORANGE%^',
    '%^BOLD%^%^B_ORANGE%^'      => '%^B_ORANGE%^',

    '%^B_BLUE%^%^BOLD%^'        => '%^B_BLUE%^',
    '%^B_BLUE%^BOLD%^'          => '%^B_BLUE%^',
    '%^BOLD%^B_BLUE%^'          => '%^B_BLUE%^',
    '%^BOLD%^%^B_BLUE%^'        => '%^B_BLUE%^',
    '%^B_LIGHTBLUE%^'           => '%^B_BLUE%^',

    '%^B_MAGENTA%^%^BOLD%^'     => '%^B_MAGENTA%^',
    '%^B_MAGENTA%^BOLD%^'       => '%^B_MAGENTA%^',
    '%^BOLD%^B_MAGENTA%^'       => '%^B_MAGENTA%^',
    '%^BOLD%^%^B_MAGENTA%^'     => '%^B_MAGENTA%^',
    '%^B_PURPLE%^%^BOLD%^'      => '%^B_MAGENTA%^',
    '%^B_PURPLE%^BOLD%^'        => '%^B_MAGENTA%^',
    '%^BOLD%^B_PURPLE%^'        => '%^B_MAGENTA%^',
    '%^BOLD%^%^B_PURPLE%^'      => '%^B_MAGENTA%^',
    '%^B_LIGHTMAGENTA%^'        => '%^B_MAGENTA%^',
    '%^B_LIGHTPURPLE%^'         => '%^B_MAGENTA%^',

    '%^B_CYAN%^%^BOLD%^'        => '%^B_CYAN%^',
    '%^B_CYAN%^BOLD%^'          => '%^B_CYAN%^',
    '%^BOLD%^B_CYAN%^'          => '%^B_CYAN%^',
    '%^BOLD%^%^B_CYAN%^'        => '%^B_CYAN%^',
    '%^B_LIGHTCYAN%^'           => '%^B_CYAN%^',

    '%^B_WHITE%^%^BOLD%^'       => '%^B_WHITE%^',
    '%^B_WHITE%^BOLD%^'         => '%^B_WHITE%^',
    '%^BOLD%^B_WHITE%^'         => '%^B_WHITE%^',
    '%^BOLD%^%^B_WHITE%^'       => '%^B_WHITE%^',
    '%^B_BRIGHTWHITE%^'         => '%^B_WHITE%^',

    '%^B_GREY%^'                => '%^B_WHITE%^',
};

my $i3_conversion = {
    "ansi"  => {
        '%^RESET%^'                 => "\033[0;0m",

        '%^BOLD%^'                  => "\033[1m",
        '%^FLASH%^'                 => "\033[5m",

        '%^BLACK%^'                 => "\033[30m",
        '%^RED%^'                   => "\033[31m",
        '%^GREEN%^'                 => "\033[32m",
        '%^ORANGE%^'                => "\033[33m",
        '%^BLUE%^'                  => "\033[34m",
        '%^MAGENTA%^'               => "\033[35m",
        '%^CYAN%^'                  => "\033[36m",
        '%^WHITE%^'                 => "\033[37m",

        '%^BOLD%^%^BLACK%^'         => "\033[1;30m",
        '%^BOLD%^%^RED%^'           => "\033[1;31m",
        '%^BOLD%^%^GREEN%^'         => "\033[1;32m",
        '%^YELLOW%^'                => "\033[1;33m",
        '%^BOLD%^%^BLUE%^'          => "\033[1;34m",
        '%^BOLD%^%^MAGENTA%^'       => "\033[1;35m",
        '%^BOLD%^%^CYAN%^'          => "\033[1;36m",
        '%^BOLD%^%^WHITE%^'         => "\033[1;37m",

        '%^B_BLACK%^'               => "\033[40m",
        '%^B_RED%^'                 => "\033[41m",
        '%^B_GREEN%^'               => "\033[42m",
        '%^B_ORANGE%^'              => "\033[43m",
        '%^B_BLUE%^'                => "\033[44m",
        '%^B_MAGENTA%^'             => "\033[45m",
        '%^B_CYAN%^'                => "\033[46m",
        '%^B_WHITE%^'               => "\033[47m",

        # Bold backgrounds are not supported by normal ANSI
        #'%^B_GREY%^'                => "\033[40m",
        #'%^B_PINK%^'                => "\033[41m",
        #'%^B_LIGHTRED%^'            => "\033[41m",
        #'%^B_LIGHTGREEN%^'          => "\033[42m",
        '%^B_YELLOW%^'              => "\033[43m",
        #'%^B_LIGHTBLUE%^'           => "\033[44m",
        #'%^B_LIGHTMAGENTA%^'        => "\033[45m",
        #'%^B_LIGHTCYAN%^'           => "\033[46m",
        #'%^B_WHITE%^'               => "\033[47m",
    },
    "html" => {
        '%^RESET%^'                 => '</SPAN>',

        '%^BOLD%^'                  => '<SPAN style="bold;">',
        '%^FLASH%^'                 => '<SPAN class="blink;">',

        '%^BLACK%^'                 => '<SPAN style="color: #555555">',
        '%^RED%^'                   => '<SPAN style="color: #ff5555">',
        '%^GREEN%^'                 => '<SPAN style="color: #55ff55">',
        '%^ORANGE%^'                => '<SPAN style="color: #ffaa55">',
        '%^BLUE%^'                  => '<SPAN style="color: #5555ff">',
        '%^MAGENTA%^'               => '<SPAN style="color: #ff55ff">',
        '%^CYAN%^'                  => '<SPAN style="color: #55ffff">',
        '%^WHITE%^'                 => '<SPAN style="color: #aaaaaa">',

        '%^BOLD%^%^BLACK%^'         => '<SPAN style="color: #aaaaaa">',
        '%^BOLD%^%^RED%^'           => '<SPAN style="color: #ffaaaa">',
        '%^BOLD%^%^GREEN%^'         => '<SPAN style="color: #aaffaa">',
        '%^YELLOW%^'                => '<SPAN style="color: #ffff55">',
        '%^BOLD%^%^BLUE%^'          => '<SPAN style="color: #aaaaff">',
        '%^BOLD%^%^MAGENTA%^'       => '<SPAN style="color: #ffaaff">',
        '%^BOLD%^%^CYAN%^'          => '<SPAN style="color: #aaffff">',
        '%^BOLD%^%^WHITE%^'         => '<SPAN style="color: #ffffff">',

        '%^B_BLACK%^'               => '<SPAN style="background-color: #000000">',
        '%^B_RED%^'                 => '<SPAN style="background-color: #ff0000">',
        '%^B_GREEN%^'               => '<SPAN style="background-color: #00ff00">',
        '%^B_ORANGE%^'              => '<SPAN style="background-color: #ffaa00">',
        '%^B_BLUE%^'                => '<SPAN style="background-color: #0000ff">',
        '%^B_MAGENTA%^'             => '<SPAN style="background-color: #ff00ff">',
        '%^B_CYAN%^'                => '<SPAN style="background-color: #00ffff">',
        #'%^B_DARKGREY%^'            => '<SPAN style="background-color: #555555">',

        #'%^B_GREY%^'                => '<SPAN style="background-color: #aaaaaa">',
        #'%^B_PINK%^'                => '<SPAN style="background-color: #ffaaaa">',
        #'%^B_LIGHTRED%^'            => '<SPAN style="background-color: #ffaaaa">',
        #'%^B_LIGHTGREEN%^'          => '<SPAN style="background-color: #aaffaa">',
        '%^B_YELLOW%^'              => '<SPAN style="background-color: #ffff55">',
        #'%^B_LIGHTBLUE%^'           => '<SPAN style="background-color: #aaaaff">',
        #'%^B_LIGHTMAGENTA%^'        => '<SPAN style="background-color: #ffaaff">',
        #'%^B_LIGHTCYAN%^'           => '<SPAN style="background-color: #aaffff">',
        '%^B_WHITE%^'               => '<SPAN style="background-color: #ffffff">',
    },
};

my $i3_channel_colors = {
    "intermud"      => "%^WHITE%^",
    "muds"          => "%^WHITE%^",
    "connections"   => "%^BOLD%^%^WHITE%^",
    "death"         => "%^BOLD%^%^RED%^",
    "cre"           => "%^BOLD%^%^GREEN%^",
    "admin"         => "%^BOLD%^%^MAGENTA%^",
    "newbie"        => "%^B_YELLOW%^%^BLACK%^",
    "gossip"        => "%^B_BLUE%^%^YELLOW%^",

    "wiley"         => "%^BOLD%^%^YELLOW%^",
    "ds"            => "%^BOLD%^%^YELLOW%^",
    "dchat"         => "%^CYAN%^",
    "intergossip"   => "%^GREEN%^",
    "intercre"      => "%^ORANGE%^",
    "pyom"          => "%^FLASH%^%^BOLD%^%^GREEN%^",
    "free_speech"   => "%^BOLD%^%^RED%^",
    "url"           => "%^BOLD%^%^WHITE%^",
    "discord"       => "%^BOLD%^%^MAGENTA%^",

    "ibuild"        => "%^B_RED%^%^YELLOW%^",
    "ichat"         => "%^B_RED%^%^GREEN%^",
    "mbchat"        => "%^B_RED%^%^GREEN%^",
    "pchat"         => "%^B_RED%^%^BOLD%^%^GREEN%^",
    "i2game"        => "%^B_BLUE%^",
    "i2chat"        => "%^B_GREEN%^",
    "i3chat"        => "%^B_RED%^",
    "i2code"        => "%^B_YELLOW%^%^RED%^",
    "i2news"        => "%^B_YELLOW%^%^BLUE%^",
    "imudnews"      => "%^B_YELLOW%^%^CYAN%^",
    "irc"           => "%^B_BLUE%^%^GREEN%^",
    "ifree"         => "%^B_BLUE%^%^GREEN%^",

    "default"       => "%^BOLD%^%^BLUE%^",
    "default-IMC2"  => "%^B_BLUE%^%^BOLD%^%^WHITE%^"
};

my $i3_hour_colors = {
    "00" => "%^BOLD%^%^BLACK%^",
    "01" => "%^BOLD%^%^BLACK%^",
    "02" => "%^BOLD%^%^BLACK%^",
    "03" => "%^BOLD%^%^BLACK%^",
    "04" => "%^RED%^",
    "05" => "%^RED%^",
    "06" => "%^ORANGE%^",
    "07" => "%^ORANGE%^",
    "08" => "%^YELLOW%^",
    "09" => "%^YELLOW%^",
    "10" => "%^GREEN%^",
    "11" => "%^GREEN%^",
    "12" => "%^BOLD%^%^GREEN%^",
    "13" => "%^BOLD%^%^GREEN%^",
    "14" => "%^WHITE%^",
    "15" => "%^WHITE%^",
    "16" => "%^BOLD%^%^CYAN%^",
    "17" => "%^BOLD%^%^CYAN%^",
    "18" => "%^CYAN%^",
    "19" => "%^CYAN%^",
    "20" => "%^BOLD%^%^BLUE%^",
    "21" => "%^BOLD%^%^BLUE%^",
    "22" => "%^BLUE%^",
    "23" => "%^BLUE%^"
};

sub get_color_time {
    my $gm = shift;

    $gm = gmtime() unless defined $gm; #Thu Oct 13 04:54:34 1994
    my $hour = substr $gm, 11, 2;
    my $minute = substr $gm, 14, 2;
    return $i3_hour_colors->{$hour} . "$hour:$minute%^RESET%^";
}

sub channel_color {
    my $channel = shift;

    return $i3_channel_colors->{default} if !defined $channel;
    return $i3_channel_colors->{$channel} if exists $i3_channel_colors->{$channel};
    return $i3_channel_colors->{default};
}

sub normalize_pinkfish {
    my $string = shift;
    return undef unless defined $string;

    foreach my $k ( sort { length $b <=> length $a } keys( %$i3_normals ) ) {
        my $v = $i3_normals->{$k};
        $string =~ s/\Q$k\E/$v/gsmx;
    }

    return $string;
}

sub pinkfish_to {
    my $string = shift;
    my $style = shift;
    $style = "ansi" if !defined $style;

    return undef unless defined $string;
    return $string unless $style eq 'ansi' or $style eq 'html';

    foreach my $k ( keys( %{ $i3_conversion->{$style} } ) ) {
        my $v = $i3_conversion->{$style}{$k};
        $string =~ s/\Q$k\E/$v/gsmx;
    }
    return $string;
}

sub bug {
    my $db = shift;
    my $level = shift || "INFO";
    my $message = shift || "";

    my $msg = pinkfish_to( get_color_time() . " " . $message . "\n", "ansi");
    print STDERR $msg;

    #$db->trace(2);

    my $sth = $db->prepare( qq!
        INSERT INTO log (logtype, message) VALUES (?,?);
       !);
    my $rv = $sth->execute( $level, $message );
    if($rv) {
        $db->commit;
    } else {
        print STDERR $DBI::errstr."\n";
        $db->rollback;
        return undef;
    }
    return 1;
}

# Database setup code

sub do_table_setup {
    my $db = shift;
    my $table_sql = shift;
    my $truncate_sql = shift;
    my $insert_sql = shift;
    my $data = shift;

    die "Invalid database handle!" unless defined $db;
    die "Invalid table sql!" unless defined $table_sql;
    #$db->trace(2);

    my $sth = $db->prepare( $table_sql );
    my $rv = $sth->execute();
    if($rv) {
        $db->commit;
    } else {
        print STDERR $DBI::errstr."\n";
        $db->rollback;
        return undef;
    }

    if( defined $truncate_sql ) {
        $sth = $db->prepare( $truncate_sql );
        $rv = $sth->execute();
        if($rv) {
            $db->commit;
        } else {
            print STDERR $DBI::errstr."\n";
            $db->rollback;
            return undef;
        }
    }

    if( defined $data) {
        if( defined $insert_sql and ref $insert_sql eq 'CODE' ) {
            $insert_sql->( $db, $data );
        } else {
            $sth = undef;
            $sth = $db->prepare( $insert_sql );
            foreach my $k ( sort { length $b <=> length $a } keys( %$data ) ) {
                my $v = $data->{$k};
                $rv = $sth->execute($k,$v);
                if( $rv ) {
                    #$db->commit;
                } else {
                    print STDERR $DBI::errstr."\n" if $DBI::errstr !~ /^UNIQUE constraint failed/;
                    $db->rollback;
                    return undef;
                }
            }
            $db->commit if $rv;
        }
    }

    return 1;
}

sub setup_i3log_table {
    my $db = shift;
    die "Invalid database handle!" unless defined $db;

    return do_table_setup( $db, qq!
        CREATE TABLE IF NOT EXISTS i3log ( 
            created     DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW','utc')), 
            is_emote    INTEGER, 
            is_url      INTEGER, 
            is_bot      INTEGER, 
            channel     TEXT, 
            speaker     TEXT, 
            mud         TEXT, 
            message     TEXT 
        );
        !
    );
}

sub setup_urls_table {
    my $db = shift;
    die "Invalid database handle!" unless defined $db;

    return do_table_setup( $db, qq!
        CREATE TABLE IF NOT EXISTS urls ( 
            created         DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW','utc')), 
            processed       INTEGER,
            channel         TEXT,
            speaker         TEXT,
            mud             TEXT,
            url             TEXT,
            message         TEXT,
            message_text    TEXT,
            message_ansi    TEXT,
            message_html    TEXT
        );
        !
    );
}

sub setup_log_table {
    my $db = shift;
    die "Invalid database handle!" unless defined $db;

    return do_table_setup( $db, qq!
        CREATE TABLE IF NOT EXISTS log ( 
            created     DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW','utc')), 
            logtype     TEXT DEFAULT 'INFO', 
            filename    TEXT, 
            function    TEXT, 
            line        INTEGER, 
            message     TEXT 
        );
        !
    );
}

sub setup_pinkfish_normals_table {
    my $db = shift;
    die "Invalid database handle!" unless defined $db;

    return do_table_setup( $db, qq!
        CREATE TABLE IF NOT EXISTS pinkfish_normals ( 
            source  TEXT PRIMARY KEY NOT NULL,
            result  TEXT
        );
        !,
        qq!
        DELETE FROM pinkfish_normals;
        !,
        qq!
        INSERT INTO pinkfish_normals (source, result) VALUES (?,?);
        !,
        $i3_normals
    );
}

sub setup_pinkfish_to_table {
    my $db = shift;
    die "Invalid database handle!" unless defined $db;

    return unless do_table_setup( $db, qq!
        CREATE TABLE IF NOT EXISTS pinkfish_to ( 
            source  TEXT PRIMARY KEY NOT NULL REFERENCES pinkfish_normals (source),
            ansi    TEXT,
            html    TEXT
        );
        !,
        qq!
        DELETE FROM pinkfish_to;
        !
    );
    my $insert_sql = $db->prepare( qq!
        INSERT INTO pinkfish_to (source, ansi, html) VALUES (?,?,?);
        !);
    my $rv;
    foreach my $k ( sort { length $b <=> length $a } keys( %{ $i3_conversion->{'ansi'} } ) ) {
        my $va = $i3_conversion->{'ansi'}{$k};
        my $vh = $i3_conversion->{'html'}{$k};
        $rv = $insert_sql->execute($k,$va,$vh);
        if( $rv ) {
            #$db->commit;
        } else {
            print STDERR $DBI::errstr."\n" if $DBI::errstr !~ /^UNIQUE constraint failed/;
            $db->rollback;
            return undef;
        }
    }
    $db->commit if $rv;
    return 1;
}

sub setup_channel_colors_table {
    my $db = shift;
    die "Invalid database handle!" unless defined $db;

    return do_table_setup( $db, qq!
        CREATE TABLE IF NOT EXISTS channel_colors ( 
            channel TEXT PRIMARY KEY NOT NULL,
            color   TEXT
        );
        !,
        qq!
        DELETE FROM channel_colors;
        !,
        qq!
        INSERT INTO channel_colors (channel, color) VALUES (?,?);
        !,
        $i3_channel_colors
    );
}

sub setup_hour_colors_table {
    my $db = shift;
    die "Invalid database handle!" unless defined $db;

    return do_table_setup( $db, qq!
        CREATE TABLE IF NOT EXISTS hour_colors ( 
            hour    INTEGER PRIMARY KEY NOT NULL,
            color   TEXT
        );
        !,
        qq!
        DELETE FROM hour_colors;
        !,
        qq!
        INSERT INTO hour_colors (hour, color) VALUES (?,?);
        !,
        $i3_hour_colors
    );
}

sub setup_users_table {
    my $db = shift;
    die "Invalid database handle!" unless defined $db;

    return do_table_setup( $db, qq!
        CREATE TABLE IF NOT EXISTS users ( 
            name        TEXT PRIMARY KEY NOT NULL,
            passwd      TEXT,
            created     DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW','utc')), 
            last_login  DATETIME,
            last_logout DATETIME,
            email       TEXT,
            auth        INTEGER
        );
        !
    );
}

sub setup_i3_table {
    my $db = shift;
    die "Invalid database handle!" unless defined $db;

    return do_table_setup( $db, qq!
        CREATE TABLE IF NOT EXISTS i3 ( 
            router_name         TEXT PRIMARY KEY NOT NULL,
            router_address      TEXT NOT NULL,
            router_port         INTEGER NOT NULL,
            router_password     INTEGER DEFAULT 0,
            mudlist_id          INTEGER DEFAULT 0,
            chanlist_id         INTEGE DEFAULT 0R
            mud_name            TEXT,
            telnet_port         INTEGER,
            mudlib              TEXT,
            base_mudlib         TEXT,
            driver              TEXT,
            mud_type            TEXT,
            open_status         TEXT,
            admin_email         TEXT
        );
        !,
        qq!
        DELETE FROM i3;
        !,
        qq!
        INSERT INTO i3 (router_name, router_address, router_port,
                        mud_name, telnet_port, mudlib, base_mudlib,
                        driver, mud_type, open_status, admin_email) VALUES (?,?);
        !,
        $i3_data
    );
}

sub setup_i3_chanlist_table {
    my $db = shift;
    die "Invalid database handle!" unless defined $db;

    return do_table_setup( $db, qq!
        CREATE TABLE IF NOT EXISTS i3_chanlist ( 
            router_name         TEXT PRIMARY KEY NOT NULL REFERENCES i3(router_name),
            channel_name        TEXT,
            channel_host        TEXT,
            channel_type        INTEGER,
            is_deleted          INTEGER
        );
        !
    );
}

sub setup_database {
    $db = DBI->connect( "DBI:SQLite:dbname=$DB_FILE", '', '', 
        { AutoCommit => 0, PrintError => 0, } );

    setup_log_table( $db );
    bug( $db, "BOOT", "Setting up database..." );
    setup_i3log_table( $db );
    setup_urls_table( $db );
    setup_pinkfish_normals_table( $db );
    setup_pinkfish_to_table( $db );
    setup_channel_colors_table( $db );
    setup_hour_colors_table( $db );
    setup_users_table( $db );
    bug( $db, "BOOT", "Done." );
}

# User Logins

# users is the set of all users, logged in or not, indexed by name
my $users = {};

# logins is the set of current sessions, indexed by session_id
my $logins = {};

sub trim {
    my $str = shift;

    return undef unless defined $str;
    $str =~ s/^\s+//;
    $str =~ s/\s+$//;
    return $str;
}

sub load_users_table {
    my $db = shift;
    die "Invalid database handle!" unless defined $db;

    my $rv = $db->selectall_hashref(qq!
        SELECT * FROM users;
    !, 'name');

    if(defined $rv) {
    #    $db->commit;
        return $rv;
    } else {
        print STDERR $DBI::errstr."\n";
    #    $db->rollback;
    }
    return {};
}

# Does a user exist?
sub user_exists {
    my $db = shift;
    my $username = shift;
    die "Invalid database handle!" unless defined $db;

    my $rv = $db->selectall_arrayref(qq!
        SELECT * FROM users WHERE lower(trim(name)) = lower(trim(?));
    !, { Slice => {} }, $username);

    if(defined $rv) {
    #    $db->commit;
        return $rv->[0];
    } else {
        print STDERR $DBI::errstr."\n";
    #    $db->rollback;
    }
    return undef;
}

# Create a new user.
sub user_create {
    my $db = shift;
    my $username = shift;
    die "Invalid database handle!" unless defined $db;
    die "Invalid username!" unless defined $username;

    my $rv = $db->do(qq!
        INSERT INTO users (name) VALUES (trim(?));
    !, undef, $username);

    if(defined $rv) {
        $db->commit;
        return user_exists( $db, $username );
    } else {
        print STDERR $DBI::errstr."\n";
        $db->rollback;
    }
    return undef;
}

# Does a user have a password?
sub user_has_password {
    my $db = shift;
    my $username = shift;
    die "Invalid database handle!" unless defined $db;
    die "Invalid username!" unless defined $username;

    my $rv = $db->selectrow_arrayref(qq!
        SELECT passwd FROM users WHERE lower(trim(name)) = lower(trim(?));
    !, undef, $username);

    if(defined $rv) {
    #    $db->commit;
        return $rv->[0];
    } else {
        print STDERR $DBI::errstr."\n";
    #    $db->rollback;
    }
    return undef;
}

# Create a new password (or reset a password) for a user.
sub user_set_password {
    my $db = shift;
    my $username = shift;
    my $password = shift;
    die "Invalid database handle!" unless defined $db;
    die "Invalid username!" unless defined $username;

    my $rv = $db->do(qq!
        UPDATE users SET passwd = ? WHERE lower(trim(name)) = lower(trim(?));
    !, undef, $password, $username);
    if(defined $rv) {
        $db->commit;
        return user_has_password( $db, $username );
    } else {
        print STDERR $DBI::errstr."\n";
        $db->rollback;
    }
    return undef;
}

# Does the password of the user match the one given?
sub user_check_password {
    my $db = shift;
    my $username = shift;
    my $password = shift;
    die "Invalid database handle!" unless defined $db;
    die "Invalid username!" unless defined $username;

    #local $db->{TraceLevel} = "4|SQL";

    my $rv = $db->selectrow_arrayref(qq!
        SELECT 1 FROM users WHERE lower(trim(name)) = lower(trim(?)) AND passwd = ?;
    !, undef, $username, $password);

    if(defined $rv) {
    #    $db->commit;
        return $rv->[0];
    } else {
        print STDERR $DBI::errstr."\n";
    #    $db->rollback;
    }
    return undef;
}

# Does a user have an email address?
sub user_has_email {
    my $db = shift;
    my $username = shift;
    die "Invalid database handle!" unless defined $db;
    die "Invalid username!" unless defined $username;

    my $rv = $db->selectrow_arrayref(qq!
        SELECT email FROM users WHERE lower(trim(name)) = lower(trim(?));
    !, undef, $username);

    if(defined $rv) {
    #    $db->commit;
        return $rv->[0];
    } else {
        print STDERR $DBI::errstr."\n";
    #    $db->rollback;
    }
    return undef;
}

# Set email address for user.
sub user_set_email {
    my $db = shift;
    my $username = shift;
    my $email = shift;
    die "Invalid database handle!" unless defined $db;
    die "Invalid username!" unless defined $username;

    my $rv = $db->do(qq!
        UPDATE users SET email = ? WHERE lower(trim(name)) = lower(trim(?));
    !, undef, $email, $username);
    if(defined $rv) {
        $db->commit;
        return user_has_email( $db, $username );
    } else {
        print STDERR $DBI::errstr."\n";
        $db->rollback;
    }
    return undef;
}

# Does a user have an email address?
sub user_auth {
    my $db = shift;
    my $username = shift;
    die "Invalid database handle!" unless defined $db;
    die "Invalid username!" unless defined $username;

    my $rv = $db->selectrow_arrayref(qq!
        SELECT auth FROM users WHERE lower(trim(name)) = lower(trim(?));
    !, undef, $username);

    if(defined $rv) {
    #    $db->commit;
        return $rv->[0];
    } else {
        print STDERR $DBI::errstr."\n";
    #    $db->rollback;
    }
    return undef;
}

# Set email address for user.
sub user_set_auth {
    my $db = shift;
    my $username = shift;
    my $auth = shift;
    die "Invalid database handle!" unless defined $db;
    die "Invalid username!" unless defined $username;

    my $rv = $db->do(qq!
        UPDATE users SET auth = ? WHERE lower(trim(name)) = lower(trim(?));
    !, undef, $auth, $username);
    if(defined $rv) {
        $db->commit;
        return user_auth( $db, $username );
    } else {
        print STDERR $DBI::errstr."\n";
        $db->rollback;
    }
    return undef;
}

# Update their last_login time to now, clear last_logout time.
sub user_login {
    my $db = shift;
    my $username = shift;
    die "Invalid database handle!" unless defined $db;
    die "Invalid username!" unless defined $username;

    my $rv = $db->do(qq!
        UPDATE users SET last_login = STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW','utc'),
                         last_logout = NULL
                     WHERE lower(trim(name)) = lower(trim(?));
    !, undef, $username);
    if(defined $rv) {
        $db->commit;
        return user_exists( $db, $username );
    } else {
        print STDERR $DBI::errstr."\n";
        $db->rollback;
    }
    return undef;
}

# Update their last_logout time.
sub user_logout {
    my $db = shift;
    my $username = shift;
    die "Invalid database handle!" unless defined $db;
    warn "Invalid username!" unless defined $username;

    my $rv = $db->do(qq!
        UPDATE users SET last_logout = STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW','utc')
                     WHERE lower(trim(name)) = lower(trim(?));
    !, undef, $username);
    if(defined $rv) {
        $db->commit;
        return user_exists( $db, $username );
    } else {
        print STDERR $DBI::errstr."\n";
        $db->rollback;
    }
    return undef;
}

setup_database();

$users = load_users_table( $db );
$logins = {};

my $default_prompt = "Prompt: ";
my $console_session_id = undef;
my $chat_server_session_id = undef;
my $i3_client_session_id = undef;
my %session_registry = ();

sub register {
    my ( $session_id, $type, $handle ) = @_;
    $session_registry{$session_id} = {
        id          => $session_id,
        type        => $type,
        handle      => $handle,
        username    => undef,
    };
}

sub unregister {
    my ( $session_id ) = @_;
    delete $session_registry{$session_id};
}

sub format_message {
    my ($session_id, $message) = @_;

    return sprintf( "%15.15s %s", "$session_id", "$message");
}

sub system_message {
    my $session_id = shift; # Session to send message TO
    my $username = shift;   # Name of sender
    my $message = shift;    # Message
    my $prompt = shift;     # Prompt to issue afterwards

    my $msg = format_message( $username, $message );
    $poe_kernel->post($session_id, 'send', $msg, $prompt );
}

sub broadcast_message {
    my $session_id = shift; # Session that SENT the message
    my $username = shift;   # Name of sender
    my $message = shift;    # Message
    my $prompt = shift;     # Promot to issue afterwards

    $username = "$session_id" if !defined $username;
    my $msg = format_message( $username, $message );
    foreach (keys %$logins) {
        if( $_ == $session_id and $username ne 'SYSTEM' ) {
            system_message( $_, 'You', $message, $prompt );
        } else {
            system_message( $_, $username, $message, $prompt );
        }
    }
    #if( defined $console_session_id and $session_id != $console_session_id ) {
    #    $poe_kernel->post( $console_session_id, 'message_for_console', $session_id, $message, $default_prompt );
    #}
}

sub handle_who_command {
    my $db = shift;
    my $session_id = shift;
    die "Invalid database handle!" unless defined $db;
    die "Invalid session!" unless defined $session_id;

    foreach (sort values %$logins) {
        next unless defined $users->{$_};
        my $line = sprintf( "%15.15s %19.19s %19.19s",
            $users->{$_}{name} || '',
            $users->{$_}{last_login} || '',
            $users->{$_}{last_logout} || '',
        );
        system_message( $session_id, '', $line, $default_prompt );
    }
}

POE::Session->create(
    inline_states   => {
        _start              => \&setup_console,
        send                => \&handle_console_send,
        console_input       => \&handle_console_input,
        message_for_console => \&handle_message_for_console,
        _stop               => \&teardown_console,
    },
);

sub setup_console {
    my $session_id = $_[SESSION]->ID;

    $_[HEAP]{console} = POE::Wheel::ReadLine->new(
        InputEvent          => 'console_input',
    );

    my $console = $_[HEAP]{console};
    register($session_id, 'console', $console);
    $console_session_id = $session_id;
    # If the user doesn't have a .inputrc with this, arrow keys usually don't work
    $console->bind_key("\e[D", 'backward-char');
    $console->bind_key("\e[C", 'forward-char');
    $console->bind_key("\e[A", 'previous-history');
    $console->bind_key("\e[B", 'next-history');
    #$console->read_history("./test_history");
    $console->clear();
    $console->put(
        "   Welcome to I3!    ",
        "---------------------",
        "Type /quit to logout."
    );
    $console->get("Username: ");
}

sub teardown_console {
    my $session_id = $_[SESSION]->ID;
    foreach (keys %$logins) {
        user_logout( $db, $logins->{$_} ) if defined $logins->{$_};
        $logins->{$_} = undef;
    }
    unregister( $session_id );
    $console_session_id = undef;
}

sub handle_console_send {
    my ($heap, $message, $prompt) = @_[HEAP, ARG0, ARG1];
    $heap->{console}->put($message);
    $heap->{console}->get($prompt);
}

sub handle_console_input {
    my ($message, $exception) = @_[ARG0, ARG1];
    my $session_id = $_[SESSION]->ID;
    my $console = $_[HEAP]{console};

    my $username = $session_id;
    $username = $logins->{$session_id} if exists $logins->{$session_id};

    unless (defined $message) {
        foreach (keys %$logins) {
            user_logout( $db, $logins->{$_} ) if defined $logins->{$_};
            $logins->{$_} = undef;
        }
        $console->put("$exception caught.  B'bye!");
        $_[KERNEL]->signal($_[KERNEL], "UIDESTROY");
        #$console->write_history("./test_history");
        return;
    }

    if( !handle_login( $db, $session_id, $console, $default_prompt, $message ) ) {
        if( $message =~ /^\/who$/i ) {
            handle_who_command( $db, $session_id );
            #system_message( $session_id, 'SYSTEM', "Soon...", $default_prompt );
            return;
        } elsif( $message =~ /^\/quit$/i ) {
            system_message( $session_id, 'SYSTEM', "You can't /quit the console, use /shutdown.", $default_prompt );
            return;
        } elsif( $message =~ /^\/shutdown$/i ) {
            foreach (keys %$logins) {
                user_logout( $db, $logins->{$_} ) if defined $logins->{$_};
                $logins->{$_} = undef;
            }
            $_[KERNEL]->signal($_[KERNEL], "UIDESTROY");
            return;
        }

        #if( defined $chat_server_session_id and $session_id != $chat_server_session_id ) {
        #    $_[KERNEL]->post( $chat_server_session_id, 'message_for_chat_server', $session_id, $message );
        #}
        $console->addhistory($message);
        #system_message( $session_id, 'You', $message, $default_prompt );
        broadcast_message( $session_id, $username, $message, $default_prompt );
    }
}

sub handle_message_for_console {
    my $console = $_[HEAP]{console};
    my $session_id = $_[ARG0];
    my $message = $_[ARG1];
    $console->put( format_message( $session_id, $message ));
}

# This handles the login logic for both the console and the sockets
# It returns 1 if login processesing happened, 0 if the message should be
# handled externally.
sub handle_login {
    my $db = shift;
    my $session_id = shift;
    my $device = shift;
    my $device_prompt = shift;
    my $message = shift;
    die "Invalid database handle!" unless defined $db;
    die "Invalid session!" unless defined $session_id;
    die "Invalid device!" unless defined $device;

    my $username;
    my $password;

    if( !exists $logins->{$session_id} ) {
        # If there's no data for this session, it must not be logged in, so
        # the input is indeed a username.
        
        $username = $message;
        $username = trim( $username ) if defined $username;
        $username = lc( $username ) if defined $username;

        if( grep { /^$username$/i } (values %$logins) ) {
            # If we find this username as the value of another login session,
            # we reject the new attempt.
            system_message( $session_id, 'SYSTEM', "$username is already logged in.", "Username: " );
            return 1;
        }

        # This username isn't logged in anywhere, so go ahead and get started.
        if( exists $users->{$username} ) {
            # The user exists and is not fully logged in on another session.
            $users->{$username}{session} = $session_id;
            $users->{$username}{state} = 'old password';
            $logins->{$session_id} = $username;
            system_message( $session_id, 'SYSTEM', "Welcome back $username!", "Password: " );
            return 1;
        }

        # The user is brand new, or still trying to log in elsewhere?
        my $new_user = undef;
        if( $users->{$username} = user_create( $db, $username ) ) {
            # We added the user, so now get a new password from them.
            $users->{$username}{session} = $session_id;
            $users->{$username}{state} = 'new password';
            $logins->{$session_id} = $username;
            system_message( $session_id, 'SYSTEM', "Welcome $username!", "Password: " );
            return 1;
        } else {
            # If this failed, another session is trying RIGHT NOW?
            delete $users->{$username};
            system_message( $session_id, 'SYSTEM', "$username is already logging in elsewhere.  Try again.", "Username: " );
            return 1;
        }
    }

    # If our session exists, we must have gotten a username and at least created
    # a new user entry.
    $username = $logins->{$session_id};

    # If the user has no state information, at this point, they are done with login
    return undef unless defined $users->{$username}{state};
    
    if( $users->{$username}{state} eq 'old password' ) {
        # This is an old user, so check that their password is correct.
        $password = $message;
        $password = trim( $password ) if defined $password;
        $password = md5_hex( $password ) if defined $password;

        if( user_check_password( $db, $username, $password ) ) {
            # OK, let them in!
            $users->{$username} = user_login( $db, $username );
            $users->{$username}{session} = $session_id;
            system_message( $session_id, 'SYSTEM', "Nice to see you again, $username.", $device_prompt );
            broadcast_message( $session_id, 'SYSTEM', "$username has returned!", $device_prompt );
            return 1;
        } else {
            system_message( $session_id, 'SYSTEM', "Password mismatch.  Try again.", "Password: " );
            return 1;
        }
    } elsif( $users->{$username}{state} eq 'new password' ) {
        # This is a new user, so get their new password.
        $password = $message;
        $password = trim( $password ) if defined $password;
        $password = md5_hex( $password ) if defined $password;

        $users->{$username}{new_passwd} = $password;
        $users->{$username}{state} = 'verify password';
        system_message( $session_id, 'SYSTEM', "Fantastic!  Please enter the same password again.", "Password: " );
        return 1;
    } elsif( $users->{$username}{state} eq 'verify password' ) {
        # This is a new user, so if their second entry matches, save it.
        $password = $message;
        $password = trim( $password ) if defined $password;
        $password = md5_hex( $password ) if defined $password;

        if( $password eq $users->{$username}{new_passwd} ) {
            # OK, let them in and save their new password.
            if( user_set_password( $db, $username, $password ) ) {
                # Hooray!
                $users->{$username} = user_login( $db, $username );
                $users->{$username}{session} = $session_id;
                system_message( $session_id, 'SYSTEM', "Welcome aboard, $username!", $device_prompt );
                broadcast_message( $session_id, 'SYSTEM', "A wild $username appears!", $device_prompt );
                return 1;
            } else {
                # Oops?  This should never happen.
                delete $users->{$username};
                delete $logins->{$session_id};
                system_message( $session_id, 'SYSTEM', "Could not set password for $username???", "Username: " );
                return 1;
            }
        } else {
            $users->{$username}{state} = 'new password';
            system_message( $session_id, 'SYSTEM', "Password mismatch.  Try again.", "Password: " );
            return 1;
        }
    } else {
        # Oops?  This should never happen.
        delete $users->{$username};
        delete $logins->{$session_id};
        system_message( $session_id, 'SYSTEM', "Unknown state for $username???", "Username: " );
        return 1;
    }

    return undef;
}

#POE::Kernel->run();
#exit 0;

POE::Session->create(
    inline_states   => {
        _start                      => \&setup_chat_server,
        message_for_chat_server     => \&handle_message_for_chat_server,
        _stop                       => \&teardown_chat_server,
    },
);

sub setup_chat_server {
    my $session_id = $_[SESSION]->ID;
    $_[HEAP]{chat_server} = POE::Component::Server::TCP->new(
        Port                => 32081,
        InlineStates        => {
            send                => \&handle_chat_send,
        },
        ClientConnected     => \&handle_chat_connect,
        ClientError         => \&handle_chat_error,
        ClientDisconnected  => \&handle_chat_disconnect,
        ClientInput         => \&handle_chat_input,
        ClientInputFilter   => [ 'POE::Filter::Line', Literal => "\r\n" ],
        ClientOutputFilter  => 'POE::Filter::Stream',
    );
    my $chat_server = $_[HEAP]{chat_server};
    register($session_id, 'chat_server', $chat_server);
    $chat_server_session_id = $session_id;
}

sub teardown_chat_server {
    my $session_id = $_[SESSION]->ID;
    foreach (keys %$logins) {
        user_logout( $db, $logins->{$_} ) if defined $logins->{$_};
        $logins->{$_} = undef;
    }
    unregister( $session_id );
    $chat_server_session_id = undef;
}

sub handle_chat_send {
    my ($heap, $message, $prompt) = @_[HEAP, ARG0, ARG1];
    $heap->{client}->put("\r$message\r\n$prompt") if defined $heap->{client};
}

sub handle_chat_connect {
    my $session_id = $_[SESSION]->ID;
    my $client = $_[HEAP]{client};

    register($session_id, 'client', $client);
    system_message( $session_id, 'SYSTEM', "   Welcome to I3!    \r\n---------------------\r\nType quit to logout.", "Username: " );
}

sub handle_chat_disconnect {
    my $session_id = $_[SESSION]->ID;
    my $username = $session_id;
    $username = $logins->{$session_id} if exists $logins->{$session_id};

    user_logout( $db, $username );
    unregister($session_id);
    broadcast_message( $session_id, 'SYSTEM', $username . " has run away!", $default_prompt );
}

sub handle_chat_error {
    my $session_id = $_[SESSION]->ID;
    my $username = $session_id;
    $username = $logins->{$session_id} if exists $logins->{$session_id};

    unregister($session_id);
    broadcast_message( $session_id, 'SYSTEM', $username . " has crashed and burned!", $default_prompt );
    $_[KERNEL]->yield("shutdown");
}

sub handle_chat_input {
    my $session_id = $_[SESSION]->ID;
    my $message = $_[ARG0];
    my $client = $_[HEAP]{client};
    my $username = $session_id;
    $username = $logins->{$session_id} if exists $logins->{$session_id};

    if( !handle_login( $db, $session_id, $client, $default_prompt, $message ) ) {
        if( $message =~ /^\/who$/i ) {
            handle_who_command( $db, $session_id );
            #system_message( $session_id, 'SYSTEM', "Soon...", $default_prompt );
            return;
        } elsif( $message =~ /^\/quit$/i ) {
            $_[KERNEL]->yield('shutdown');
            return;
        } elsif( $message =~ /^\/shutdown$/i ) {
            system_message( $session_id, 'SYSTEM', "You can't do a /shutdown remotely.  Try /quit.", $default_prompt );
            return;
        }
        broadcast_message( $session_id, $username, $message, $default_prompt );
    }
}

sub handle_message_for_chat_server {
    my $session_id = $_[ARG0];
    my $message = $_[ARG1];
    my $username = $session_id;
    $username = $logins->{$session_id} if exists $logins->{$session_id};

    broadcast_message( $session_id, $username, $message, $default_prompt );
}

POE::Kernel->run();
exit 0;

my $i3_data = [
    {
        router_name     => '*KellyTest',
        router_address  => '144.139.132.92',
        router_port     => 9000,
        router_password => 0, # This will be given by the router on connect.
        mudlist_id      => 0, # This will be given by the router on connect.
        chanlist_id     => 0, # This will be given by the router on connect.
        mud_name        => 'PerlToy',
        telnet_port     => 3456,
        mudlib          => 'Under Par',
        base_mudlib     => 'Par 5',
        driver          => '9 Iron',
        mud_type        => 'Golf Clap',
        open_status     => 'LOL',
        admin_email     => 'crow@biteme.com',
    },
    {
        router_name     => '*dalet',
        router_address  => '97.107.133.86',
        router_port     => 8787,
        router_password => 0, # This will be given by the router on connect.
        mudlist_id      => 0, # This will be given by the router on connect.
        chanlist_id     => 0, # This will be given by the router on connect.
        mud_name        => 'PerlToy',
        telnet_port     => 3456,
        mudlib          => 'Under Par',
        base_mudlib     => 'Par 5',
        driver          => '9 Iron',
        mud_type        => 'Golf Clap',
        open_status     => 'LOL',
        admin_email     => 'crow@biteme.com',
    },
    {
        router_name     => '*i4',
        router_address  => '204.209.44.3',
        router_port     => 8080,
        router_password => 0, # This will be given by the router on connect.
        mudlist_id      => 0, # This will be given by the router on connect.
        chanlist_id     => 0, # This will be given by the router on connect.
        mud_name        => 'PerlToy',
        telnet_port     => 3456,
        mudlib          => 'Under Par',
        base_mudlib     => 'Par 5',
        driver          => '9 Iron',
        mud_type        => 'Golf Clap',
        open_status     => 'LOL',
        admin_email     => 'crow@biteme.com',
    },
];

my $current_router = '*KellyTest';

POE::Session->create(
    inline_states   => {
        _start          => \&setup_i3,
    },
);

# Public events:  connect, reconnect, shutdown

my $i3_buffer = '';

sub setup_i3_client {
    my $session_id = $_[SESSION]->ID;
    $_[HEAP]{i3_client} = POE::Component::Client::TCP->new(
        Alias           => "i3_client",
        RemoteAddress   => $i3_data->[$current_router]->{router_address},
        RemotePort      => $i3_data->[$current_router]->{router_port},
        #Domain          => AF_INET,
        Filter          => 'POE::Filter::Stream',
        Connected       => \&handle_i3_connect,
        Disconnected    => \&handle_i3_disconnect,
        ServerInput     => \&handle_i3_recv,
        InlineStates        => {
            send                => \&handle_i3_send,
        },
    );
    my $i3_client = $_[HEAP]{i3_client};
    register($session_id, 'i3_client', $i3_client);
    $i3_client_session_id = $session_id;

    #setsockopt( $socket, SOL_SOCKET, SO_KEEPALIVE, 1);
    #my $flags = fcntl($socket, F_GETFL, 0) or die "Can't get flags for socket: $!\n";
    #            fcntl($socket, F_SETFL, $flags | O_NONBLOCK) or die "Can't make socket nonblocking: $!\n";
    #my $peerHost = $socket->peerhost();
    #my $peerPort = $socket->peerport();
}

sub teardown_i3_client {
    my $session_id = $_[SESSION]->ID;
    unregister( $session_id );
    $i3_client_session_id = undef;
}

sub handle_i3_send {
    return unless $_[HEAP]{connected};
    return if $_[HEAP]{shutdown};
    my $packet = $_[ARG0];
    $_[HEAP]{server}->put( $packet );
}

sub handle_i3_recv {
    my $packet = $_[ARG0];
    my @args = @_;

    $i3_buffer .= $packet;
    my $packet_length = get_packet_length( $i3_buffer );
    return unless length $i3_buffer >= $packet_length + 4;

    substr( $i3_buffer, 0, 4, '' );                           # remove length prefix
    my $data = substr( $i3_buffer, 0, $packet_length, '' );   # remove content
    my $result = parse_packet( $data );
    $_[HEAP]{packet} = $result;
    handle_packet( @args );
}

sub handle_packet {
    return unless defined $_[HEAP]{packet};
    return unless ref $_[HEAP]{packet} eq 'ARRAY';
    my $packet = $_[HEAP]{packet};
    my $type = $packet->[0];
    return unless defined $type and $type ne '';
    my $heap = $_[HEAP];

    if( $type eq 'startup-reply' ) {
        handle_startup_reply( $packet, $heap );
    } elsif( $type eq 'chanlist-reply' ) {
        handle_chanlist_reply( $packet, $heap );
    } elsif( $type eq 'channel-m' ) {
        handle_channel_m( $packet, $heap );
    } elsif( $type eq 'channel-e' ) {
        handle_channel_e( $packet, $heap );
    } elsif( $type eq 'channel-t' ) {
        handle_channel_t( $packet, $heap );
    }
}

sub handle_i3_connect {
    my @args = @_;

    send_startup_packet( @args );
}

#### Packet stuff

sub escape_string {
    my $str = shift;

    $str =~ s/\\/\\\\/gi;
    $str =~ s/"/\\"/gi;
    $str =~ s/\r/\\r/gi;
    $str =~ s/\n/\\n/gi;
    $str =~ s/\x00/\\x00/gi;

    $str = '"' . $str . '"';
    
    return $str;
}

sub unescape_string {
    my $str = shift;

    $str = substr($str, 1, -1); # Remove quotes

    $str =~ s/\\x00/\x00/gi;
    $str =~ s/\\n/\n/gi;
    $str =~ s/\\r/\r/gi;
    $str =~ s/\\"/"/gi;
    $str =~ s/\\\\/\\/gi;

    return $str;
}

# Decode packets

sub parse_packet {
    my $raw_packet = shift;

    my $i3_packet_grammar = qr{
        # Start
        <nocontext:>
        <data=map> | <data=array>

        <rule: map>
            \(\[ <[entry]>+ % ( , ) \]\)
            (?{ my $tmp = {}; 
                my $arr = $MATCH{entry};
                foreach my $i (@$arr) {
                    my @keys = keys(%$i);
                    foreach my $k (@keys) {
                        $tmp->{$k} = $i->{$k};
                    };
                };
                $MATCH = $tmp;
                })

        <rule: entry>
            <key=string> \: <value>
            (?{ $MATCH = { $MATCH{key} => $MATCH{value} } })

        <rule: array>
            \(\{ <[value]>+ % ( , ) \}\)
            (?{ $MATCH = $MATCH{value} })

        <rule: value>
            <string> 
            (?{ $MATCH = $MATCH{string} })
            | <number> 
            (?{ $MATCH = $MATCH{number} })
            | <map> 
            (?{ $MATCH = $MATCH{map} })
            | <array>
            (?{ $MATCH = $MATCH{array} })

        <token: string>
            ( \".*?(?<!\\)\" )
            <MATCH= (?{ unescape_string( $CAPTURE ); })>

        <token: number>
            ( [+-]?[0-9]*\.?[0-9]+(?:[eE][+-]?[0-9]+)? )
            <MATCH= (?{ $CAPTURE + 0; })>

    }xms;

    if( $raw_packet =~ $i3_packet_grammar ) {
        my $result = \%/;
        return $result->{data};
    }
    return undef;
}

sub get_packet_length {
    my $buffer = shift;

    return undef if !defined $buffer;
    return undef unless length $buffer >= 4; # Must have 32-bit length word!
    my $packet_length = unpack "N", $buffer;
    return $packet_length;
}

# Create packets

sub dump_array {
    my $data = shift;
    return undef if !defined $data;
    return undef if !defined ref $data;
    return undef if ref $data ne 'ARRAY';

    my @tmp = ();
    foreach my $bit (@$data) {
        if( ref $bit eq 'ARRAY' ) {
            push @tmp, dump_array( $bit );
        } elsif( ref $bit eq 'HASH' ) {
            push @tmp, dump_hash( $bit );
        } elsif( $bit =~ /^([+-]?[0-9]*\.?[0-9]+(?:[eE][+-]?[0-9]+)?)$/xi ) {
            push @tmp, $bit;
        } else {
            push @tmp, escape_string( $bit );
        }
    }
    my $packet = "({" . join( ",", @tmp ) . "})";
    return $packet;
}

sub dump_hash {
    my $data = shift;
    return undef if !defined $data;
    return undef if !defined ref $data;
    return undef if ref $data ne 'HASH';

    my @tmp = ();
    foreach my $key (keys %$data) {
        my $bit = $data->{$key};
        my $result = escape_string($key) . ":";

        if( ref $bit eq 'ARRAY' ) {
            $result .= dump_array( $bit );
        } elsif( ref $bit eq 'HASH' ) {
            $result .= dump_hash( $bit );
        } elsif( $bit =~ /^([+-]?[0-9]*\.?[0-9]+(?:[eE][+-]?[0-9]+)?)$/xi ) {
            $result .= $bit;
        } else {
            $result .= escape_string( $bit );
        }
        push @tmp, $result;
    }
    my $packet = "([" . join( ",", @tmp ) . "])";
    return $packet;
}

sub form_packet {
    my $data = shift;
    return undef if !defined $data;
    return undef if !defined ref $data;
    return undef if ref $data ne 'ARRAY';

    my $packet = dump_array( $data );
    my $len = pack "N", length $packet;
    return $len . $packet;
}

# Individual packet handlers





my $MUD = {
    name                => 'Testes123',
    port                => 3456,
    driver              => '9 Iron',
    mudlib              => 'Under Par',
    base_mudlib         => 'Par 5',
    mud_type            => 'golf clap',
    open_status         => 'LOL',
    admin_email         => 'crow@biteme.com',
};

my $I3 = {
    current_router      => $current_router,
    password            => 0,
    mudlist_id          => 0,
    chanlist_id         => 0,
};

sub send_startup_packet {
    my $session_id = $_[SESSION]->ID;
    my @args = @_;

    my $gm = gmtime();
    my $packet = form_packet(
        [
            "startup-req-3",                                # Packet type
            5,                                              # Always 5
            $i3_data->[$current_router]->{mud_name},        # originator mud name or 0
            0,                                              # originator username or 0
            $current_router,                                # target mud name or 0
            0,                                              # target username or 0

            $i3_data->[$current_router]->{router_password}, # Our I3 password integer, or 0
            $i3_data->[$current_router]->{mudlist_id},      # Our last seen mudlist ID, or 0
            $i3_data->[$current_router]->{chanlist_id},     # Our last seen channel list ID, or 0
            $i3_data->[$current_router]->{telnet_port},     # Login port for our mud
            0,
            0,
            $i3_data->[$current_router]->{mudlib},
            $i3_data->[$current_router]->{base_mudlib},
            $i3_data->[$current_router]->{driver},
            $i3_data->[$current_router]->{mud_type},
            $i3_data->[$current_router]->{open_status},
            $i3_data->[$current_router]->{admin_email},
            {
                emoteto     => 0,   # boolean
                news        => 0,   # boolean
                ucache      => 0,   # boolean
                auth        => 0,   # boolean
                locate      => 0,   # boolean
                finger      => 0,   # boolean
                channel     => 1,   # boolean
                who         => 0,   # boolean
                tell        => 1,   # boolean
                beep        => 0,   # boolean
                mail        => 0,   # boolean
                file        => 0,   # boolean
                http        => 0,   # port number if used
                smtp        => 0,   # port number if used
                pop3        => 0,   # port number if used
                ftp         => 0,   # port number if used
                nntp        => 0,   # port number if used
                rcp         => 0,   # port number if used
                amrcp       => 0,   # port number if used
            },
            {
                #url         => 'web address url', # only include if active
                'time'      => $gm, # should be filled with the time the packet is sent, ctime format
            },
        ]
    );

    # Pass the packet to a send event and hope for the best!
    $poe_kernel->post( $session_id => send => $packet );
}

sub handle_startup_reply {
    my $packet = shift;
    my $heap = shift;

    return if !defined $packet;
    return if ref $packet ne 'ARRAY';

    my $origin = $packet->[2];      # should be the router
    my $target = $packet->[4];      # should be us!
    my $local_router_list = $packet->[6];
    my $password = $packet->[7];    # we should save this

    # router list will be an array of names and strings like "ip-address port"
    # At this point, we may want to ask for a channel list
    # typically, we'd send listen notifications to any channels we already know
    # about here as well.
}

sub handle_chanlist_reply {
    my $packet = shift;
    my $heap = shift;

    return if !defined $packet;
    return if ref $packet ne 'ARRAY';

    my $origin = $packet->[2];          # should be the router
    my $target = $packet->[4];          # should be us!
    my $chanlist_id = $packet->[6];     # we should save this
    my $channel_list = $packet->[7];    # we should save this

    # the chanlist_id is an integer telling us the update number
    # however, we may get the same number multiple times, as the channel list
    # is broken into segments for DikuMUD string limits.
    # the channel_list is a mapping of channel information

    # the channel list mapping has the channel name as a key
    # the value is either an integer 0, meaning the channel has been deleted,
    # or an array of two elements, element 0 is the host mud name, element 1 is the type
}

sub handle_channel_m {
    my $packet = shift;
    my $heap = shift;

    return if !defined $packet;
    return if ref $packet ne 'ARRAY';

    my $origin = $packet->[2];      # mud that spoke to us
    my $speaker = $packet->[3];     # player that spoke to us
    my $channel = $packet->[6];     # channel they spoke on
    my $visname = $packet->[7];     # name they want to look like
    my $message = $packet->[8];     # what was said
}

sub handle_channel_e {
    my $packet = shift;
    my $heap = shift;

    return if !defined $packet;
    return if ref $packet ne 'ARRAY';

    my $origin = $packet->[2];      # mud that spoke to us
    my $speaker = $packet->[3];     # player that spoke to us
    my $channel = $packet->[6];     # channel they spoke on
    my $visname = $packet->[7];     # name they want to look like
    my $message = $packet->[8];     # what was said
}

sub handle_channel_t {
    my $packet = shift;
    my $heap = shift;

    return if !defined $packet;
    return if ref $packet ne 'ARRAY';

    my $origin = $packet->[2];              # mud that spoke to us
    my $speaker = $packet->[3];             # player that spoke to us
    my $channel = $packet->[6];             # channel they spoke on
    my $target_mud = $packet->[7];          # mud they were aiming at
    my $target_player = $packet->[8];       # player they were aiming at
    my $message_others = $packet->[9];      # message for everyone else
    my $message_target = $packet->[10];     # message for the target player
    my $speaker_visname = $packet->[11];    # what the speaker wants to look like
    my $target_visname = $packet->[12];     # what the speaker calls the target
}



POE::Kernel->run();
exit 0;

