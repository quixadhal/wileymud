#!/usr/bin/perl -w
# Created by Quixadhal, 2010.05.01

# Fed a URL, this script will fetch it and return information about the actual
# destination URL and, in the case of youtube, the video in question.
# A second argument is for the mud channel the URL came from, if any.
#
# <link rel="canonical" href="/watch?v=IleiqUDYpFQ">
# <link rel="alternate" media="handheld" href="http://m.youtube.com/watch?desktop_uri=%2Fwatch%3Fv%3DIleiqUDYpFQ&amp;v=IleiqUDYpFQ&amp;gl=US">
# <meta name="title" content="Ronald Reagan: First Inaugural Address (1 of 3)">
# <meta name="description" content="Senator Hatfield, Mr. Chief Justice, Mr. President, Vice President Bush, Vice President Mondale, Senator Baker, Speaker O\'Neill, Reverend Moomaw, and my fell...">
# <meta name="keywords" content="president, ronald, reagan, first, inaugural, inauguration, address, washington, 80s, reaganomics, ronny, gipper, carter, jfk, kennedy, bush, clinton, republi...">
# <meta itemprop="duration" content="PT8M26S">
#
# Here are the triggers I used in Tinyfugue, to feed it.
#
# 00:45 <intergossip> Kalinash@Fire and Ice: http://www.youtube.com/watch?v=IleiqUDYpFQ
#
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://www.youtube.com/watch\?.*?v=[^&\?\.\s]+)" check_youtube_chan = /if (%P1 !~ "url") /quote -0 url !~/bin/untiny.pl '%P2' '%P1'%; /endif
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://tinyurl.com/[^&\?\.\s]+)" check_tiny_chan = /quote -0 url !~/bin/untiny.pl '%P2' '%P1'
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://bit.ly/[^&\?\.\s]+)" check_bitly_chan = /quote -0 url !~/bin/untiny.pl '%P2' '%P1'
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://goo.gl/[^&\?\.\s]+)" check_googl_chan = /quote -0 url !~/bin/untiny.pl '%P2' '%P1'
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://mcaf.ee/[^&\?\.\s]+)" check_mcafee_chan = /quote -0 url !~/bin/untiny.pl '%P2' '%P1'
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://migre.me/[^&\?\.\s]+)" check_migreme_chan = /quote -0 url !~/bin/untiny.pl '%P2' '%P1'
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://durl.me/[^&\?\.\s]+)" check_durlme_chan = /quote -0 url !~/bin/untiny.pl '%P2' '%P1'
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://is.gd/[^&\?\.\s]+)" check_isgd_chan = /quote -0 url !~/bin/untiny.pl '%P2' '%P1'
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://dailym.ai/[^&\?\.\s]+)" check_daily_chan = /quote -0 url !~/bin/untiny.pl '%P2' '%P1'
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://ebay.to/[^&\?\.\s]+)" check_ebay_chan = /quote -0 url !~/bin/untiny.pl '%P2' '%P1'
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://youtu.be/[^&\?\.\s]+)" check_yout_chan = /quote -0 url !~/bin/untiny.pl '%P2' '%P1'
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://onforb.es/[^&\?\.\s]+)" check_forbs_chan = /quote -0 url !~/bin/untiny.pl '%P2' '%P1'
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://imgur.com/[^&\>\.\s]+)" check_imgur_chan = /quote -0 url !~/bin/untiny.pl '%P2' '%P1'
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://amzn.to/[^&\?\.\s]+)" check_amzon_chan = /quote -0 url !~/bin/untiny.pl '%P2' '%P1'
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://on.fb.me/[^&\?\.\ ]+)" check_fbme_chan = /quote -0 url !~/bin/untiny.pl '%P2' '%P1'
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://qr.ae/[^&\?\.\ ]+)" check_qrae_chan = /quote -0 url !~/bin/untiny.pl '%P2' '%P1'
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://imdb.to/[^&\?\.\s]+)" check_imdb_chan = /quote -0 url !~/bin/untiny.pl '%P2' '%P1'
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://.*?imdb.com/title/[^&\?\.\s]+)" check_imdbfull_chan = /if (%P1 !~ "url") /quote -0 url !~/bin/untiny.pl '%P2' '%P1'%; /endif
#/def -mregexp -p2 -t"\[([\w-]+)\].*(https?://store.steampowered.com/app/[^&\?\.\s]+)" check_steamfull_chan = /if (%P1 !~ "url") /quote -0 url !~/bin/untiny.pl '%P2' '%P1'%; /endif
#
#Just use this one now...
#
#/def -mregexp -p2 -t"\[([\w-]+)\].* (https?\:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,3}(?::[a-zA-Z0-9]*)?\/?(?:[a-zA-Z0-9\-\._\?\,\'\/\\\+&amp;%\$#\=~])*)" check_generic_url = /if (%P1 !~ "url") /quote -0 url !~/bin/untiny.pl '%P2' '%P1'%; /endif

use strict;
use English;
use Data::Dumper;
use HTTP::Request::Common qw(POST);
use HTML::Entities;
use LWP::UserAgent;
use URI;
use WWW::Shorten::TinyURL qw(makeashorterlink);

sub channel_color {
    my $channel = shift;
    my $method = shift;
    my $colors = {
        "bloodlines" => {
            "intermud"      => "%^B_BLACK%^%^GREY%^",
            "muds"          => "%^B_BLACK%^%^GREY%^",
            "connections"   => "%^B_BLACK%^%^WHITE%^",
            "death"         => "%^LIGHTRED%^",
            "cre"           => "%^LIGHTGREEN%^",
            "admin"         => "%^LIGHTMAGENTA%^",
            "newbie"        => "%^B_YELLOW%^%^BLACK%^",
            "gossip"        => "%^B_BLUE%^%^YELLOW%^",

            "wiley"         => "%^YELLOW%^",
            "ds"            => "%^YELLOW%^",
            "dchat"         => "%^CYAN%^",
            "intergossip"   => "%^GREEN%^",
            "intercre"      => "%^ORANGE%^",
            "pyom"          => "%^FLASH%^%^LIGHTGREEN%^",
            "free_speech"   => "%^PINK%^",
            "url"           => "%^WHITE%^",

            "ibuild"        => "%^B_RED%^%^YELLOW%^",
            "ichat"         => "%^B_RED%^%^GREEN%^",
            "mbchat"        => "%^B_RED%^%^GREEN%^",
            "pchat"         => "%^B_RED%^%^LIGHTGREEN%^",
            "i2game"        => "%^B_BLUE%^",
            "i2chat"        => "%^B_GREEN%^",
            "i3chat"        => "%^B_RED%^",
            "i2code"        => "%^B_YELLOW%^%^RED%^",
            "i2news"        => "%^B_YELLOW%^%^BLUE%^",
            "imudnews"      => "%^B_YELLOW%^%^CYAN%^",
            "irc"           => "%^B_BLUE%^%^GREEN%^",
            "ifree"         => "%^B_BLUE%^%^GREEN%^",

            "default"       => "%^LIGHTBLUE%^",
            "default-IMC2"  => "%^B_BLUE%^%^WHITE%^"
        },
        "wiley" => {
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
        },
        "html" => {
            "intermud"      => '<SPAN style="color: #bbbbbb">',
            "muds"          => '<SPAN style="color: #bbbbbb">',
            "connections"   => '<SPAN style="color: #ffffff">',
            "death"         => '<SPAN style="color: #ff5555">',
            "cre"           => '<SPAN style="color: #55ff55">',
            "admin"         => '<SPAN style="color: #ff55ff">',
            "newbie"        => '<SPAN style="background-color: #ffff55; color: #000000">',
            "gossip"        => '<SPAN style="background-color: #0000bb; color: #ffff55">',

            "wiley"         => '<SPAN style="color: #ffff55">',
            "ds"            => '<SPAN style="color: #ffff55">',
            "dchat"	    => '<SPAN style="color: #00bbbb">',
            "intergossip"   => '<SPAN style="color: #00bb00">',
            "intercre"      => '<SPAN style="color: #bbbb00">',
            "pyom"          => '<SPAN style="color: #55ff55">',
            "free_speech"   => '<SPAN style="color: #ff55ff">',
            "url"           => '<SPAN style="color: #ffffff">',

            "ibuild"        => '<SPAN style="background-color: #bb0000; color: #ffff55">',
            "ichat"         => '<SPAN style="background-color: #00bb00; color: #000000">',
            "mbchat"        => '<SPAN style="background-color: #00bb00; color: #000000">',
            "pchat"         => '<SPAN style="background-color: #bb0000; color: #00ff00">',
            "i2game"        => '<SPAN style="background-color: #0000bb; color: #ffffff">',
            "i2chat"        => '<SPAN style="background-color: #00bb00; color: #ffffff">',
            "i3chat"        => '<SPAN style="background-color: #bb0000; color: #ffffff">',
            "i2code"        => '<SPAN style="background-color: #bb0000; color: #ffff55">',
            "i2news"        => '<SPAN style="background-color: #0000bb; color: #ffff55">',
            "imudnews"      => '<SPAN style="background-color: #00bbbb; color: #0000bb">',
            "irc"           => '<SPAN style="background-color: #bb00bb; color: #000000">',
            "ifree"         => '<SPAN style="background-color: #bb00bb; color: #000000">',

            "default"       => '<SPAN style="color: #5555ff">',
            "default-IMC2"  => '<SPAN style="background-color: #0000bb; color: #ffffff">'
        },
        "ansi" => {
            "intermud"      => "\033[40m\033[1;30m",
            "muds"          => "\033[40m\033[1;30m",
            "connections"   => "\033[40m\033[1;37m",
            "death"         => "\033[1;31m",
            "cre"           => "\033[1;32m",
            "admin"         => "\033[1;35m",
            "newbie"        => "\033[43m\033[30m",
            "gossip"        => "\033[44m\033[1;33m",

            "wiley"         => "\033[1;33m",
            "ds"            => "\033[1;33m",
            "dchat"         => "\033[36m",
            "intergossip"   => "\033[32m",
            "intercre"      => "\033[33m",
            "pyom"          => "\033[5m\033[1;32m",
            "free_speech"   => "\033[1;31m",
            "url"           => "\033[1;37m",

            "ibuild"        => "\033[41m\033[1;33m",
            "ichat"         => "\033[41m\033[32m",
            "mbchat"        => "\033[41m\033[32m",
            "pchat"         => "\033[41m\033[1;32m",
            "i2game"        => "\033[44m",
            "i2chat"        => "\033[42m",
            "i3chat"        => "\033[41m",
            "i2code"        => "\033[43m\033[31m",
            "i2news"        => "\033[43m\033[34m",
            "imudnews"      => "\033[43m\033[36m",
            "irc"           => "\033[44m\033[32m",
            "ifree"         => "\033[44m\033[32m",

            "default"       => "\033[34m",
            "default-IMC2"  => "\033[44m\033[1;37m",
        },
    };

    $method = "wiley" if !defined $method;

    return $colors->{$method}->{default} if !defined $channel;
    return $colors->{$method}->{$channel} if exists $colors->{$method}->{$channel};
    return $colors->{$method}->{default};
}

sub pinkfish_to {
    my $string = shift;
    my $style = shift;
    $style = "wiley" if !defined $style;

    my $conversion = {
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
            #'%^WHITE%^'                 => "\033[37m",

            '%^GREY%^'                  => "\033[1;30m",
            '%^PINK%^'                  => "\033[1;31m",
            '%^LIGHTRED%^'              => "\033[1;31m",
            '%^LIGHTGREEN%^'            => "\033[1;32m",
            '%^YELLOW%^'                => "\033[1;33m",
            '%^LIGHTBLUE%^'             => "\033[1;34m",
            '%^LIGHTMAGENTA%^'          => "\033[1;35m",
            '%^LIGHTCYAN%^'             => "\033[1;36m",
            '%^WHITE%^'                 => "\033[1;37m",
        },
        "wiley" => {
        },
        "html" => {
        },
    };
    foreach my $k (keys(%${ $conversion->{$style} })) {
        my $v = $conversion->{$style}{$k};
        $string =~ s/\Q$k\E/$v/gsmx;
    }
    return $string;
}

sub pinkfish_to_ansi {
    my $string = shift;
    my %conversion = (
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
        #'%^WHITE%^'                 => "\033[37m",

        '%^GREY%^'                  => "\033[1;30m",
        '%^PINK%^'                  => "\033[1;31m",
        '%^LIGHTRED%^'              => "\033[1;31m",
        '%^LIGHTGREEN%^'            => "\033[1;32m",
        '%^YELLOW%^'                => "\033[1;33m",
        '%^LIGHTBLUE%^'             => "\033[1;34m",
        '%^LIGHTMAGENTA%^'          => "\033[1;35m",
        '%^LIGHTCYAN%^'             => "\033[1;36m",
        '%^WHITE%^'                 => "\033[1;37m",
    );
    foreach my $k (keys(%conversion)) {
        my $v = $conversion{$k};
        $string =~ s/\Q$k\E/$v/gsmx;
    }
    return $string;
}

sub get_url {
    my $url = shift;

    return undef if !defined $url;
    my $timeout = 90;
    my $lwp = LWP::UserAgent->new();
       $lwp->timeout($timeout/2);
       #$lwp->agent("User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.5) Gecko/20031007 Firebird/0.7");
       $lwp->agent('User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36');
       $URI::ABS_ALLOW_RELATIVE_SCHEME = 1;
       $URI::ABS_REMOTE_LEADING_DOTS = 1; 
    my $request = HTTP::Request->new(GET => $url);
    my $response = undef;

    my $given_uri = URI->new($url);
    my $given_host = $given_uri->host;
    my $origin_uri = undef;

    #print STDERR "DEBUG: given URL:  $given_uri\n";
    #print STDERR "DEBUG: given HOST: $given_host\n";

    if($request) {
        eval {
            local $SIG{ALRM} = sub { die "Exceeded Timeout of $timeout for $url\n" };
            alarm $timeout;
            $response = $lwp->request($request);
            alarm 0;
        };
        warn "Timeout" if($EVAL_ERROR and ($EVAL_ERROR =~ /^Exceeded Timeout/));

        if( (defined $response) and $response->is_success ) {
            my $origin = undef;
            for( my $prev = $response; defined $prev; $prev = $prev->previous ) {
                $origin = $prev->header("location");
                #print STDERR "DEBUG: $origin\n";
                #print STDERR "DEBUG: " . Dumper($prev) . "\n";
                last if defined $origin;
            }
            #print STDERR "DEBUG: origin URL: $origin\n" if defined $origin;
            $origin_uri = (defined $origin) ? URI->new($origin) : $given_uri->clone;
        }
        return ($origin_uri, $response->content);
    }
    return undef;
}

sub get_youtube_id {
    my $page = shift;

    $page =~ /<link\s+rel=\"canonical\"\s+href=\".*?\/watch\?v=([^\"\&]*)\">/i;
    my ($id) =  ($1);
    return $id;
}

sub get_youtube_title {
    my $page = shift;

    $page =~ /<meta\s+name=\"title\"\s+content=\"([^\"]*)\">/i;
    my ($title) =  ($1);
    $title = decode_entities($title) if defined $title;
    return $title;
}

sub get_youtube_desc {
    my $page = shift;

    $page =~ /<meta\s+name=\"description\"\s+content=\"([^\"]*)\">/i;
    my ($desc) =  ($1);
    $desc = decode_entities($desc) if defined $desc;
    return $desc;
}

sub get_youtube_keywords {
    my $page = shift;

    $page =~ /<meta\s+name=\"keywords\"\s+content=\"([^\"]*)\">/i;
    my @keywords = (split /,\s+/, $1);
    return \@keywords;
}

sub get_youtube_duration {
    my $page = shift;

    $page =~ /<meta\s+itemprop=\"duration\"\s+content=\"([^\"]*)\">/i;
    my ($funky) = ($1);
    $funky =~ /.*?(\d+)M(\d+)S/;
    my ($minutes, $seconds) = ($1, $2);
    return sprintf "%d:%02d", $minutes, $seconds;
}

sub get_youtube_length {
    my $page = shift;

    $page =~ /<meta\s+itemprop=\"duration\"\s+content=\"([^\"]*)\">/i;
    my ($funky) = ($1);
    $funky =~ /.*?(\d+)M(\d+)S/;
    my ($minutes, $seconds) = ($1, $2);
    return ($minutes, $seconds);
}

sub get_page_title {
    my $page = shift;

    $page =~ /<title>\s*([^\<]*?)\s*<\/title>/i;
    my ($funky) = ($1);
    return $funky;
}

sub get_imdb_id {
    my $page = shift;

    $page =~ /<meta\s+property=\"pageId\"\s+content=\"(tt\d\d\d\d\d\d\d)\"\s+\/>/i;
    my ($id) =  ($1);
    return $id;
}

sub get_imdb_title {
    my $page = shift;

    $page =~ /<meta\s+name=\"title\"\s+content=\"([^\"]*)\"\s+\/>/i;
    my ($title) =  ($1);
    $title = decode_entities($title) if defined $title;
    return $title;
}

sub get_imdb_duration {
    my $page = shift;

    $page =~ /<time\s+itemprop=\"duration\"\s+datetime=\"PT(\d+)M\">/i;
    my ($minutes) = ($1);
    my $hours = int( $minutes / 60 );
    $minutes = $minutes % 60;
    return sprintf "%d:%02d", $hours, $minutes;
}

sub get_steam_id {
    my $page = shift;

    $page =~ /<link\s+rel=\"canonical\"\s+href=\".*?\/app\/([^\"\&]*)\/\">/i;
    my ($id) =  ($1);
    return $id;
}

sub get_steam_desc {
    my $page = shift;

    $page =~ /<meta\s+name=\"Description\"\s+content=\"([^\"]*)\">/i;
    my ($desc) =  ($1);
    $desc = decode_entities($desc) if defined $desc;
    return $desc;
}

my $prog = $0;
my $url = shift;
my $style = "wiley";

$style = "ansi" if $url eq '--ansi';
$style = "html" if $url eq '--html';
$url = shift if $style ne "wiley";

my $RESET = "%^RESET%^";
$RESET = pinkfish_to_ansi($RESET) if $style eq "ansi";
$RESET = "</SPAN>" if $style eq "html";

my $YELLOW = "%^YELLOW%^";
$YELLOW = pinkfish_to_ansi($YELLOW) if $style eq "ansi";
$YELLOW = '<SPAN style="color: #ffff55">' if $style eq "html";

my $RED = "%^RED%^";
$RED = pinkfish_to_ansi($RED) if $style eq "ansi";
$RED = '<SPAN style="color: #ff5555">' if $style eq "html";

my $GREEN = "%^GREEN%^";
$GREEN = pinkfish_to_ansi($GREEN) if $style eq "ansi";
$GREEN = '<SPAN style="color: #55ff55">' if $style eq "html";

my $CYAN= "%^CYAN%^";
$YELLOW = pinkfish_to_ansi($CYAN) if $style eq "ansi";
$YELLOW = '<SPAN style="color: #55ffff">' if $style eq "html";

my $FLASH = "%^FLASH%^";
$FLASH = pinkfish_to_ansi($FLASH) if $style eq "ansi";
$FLASH = '<SPAN class="blink;">' if $style eq "html";



my $given_uri = URI->new($url);
my $given_host = $given_uri->host;

my $channel = shift;
my ($origin, $page) = get_url($url);

# Give it a second try, because sometimes it fails from DNS stupidity.
if( !defined $page ) {
    sleep 0.5;
    ($origin, $page) = get_url($url);
}

#print STDERR "DEBUG: $page\n";

$origin = $given_uri if !defined $origin;
my $tinyurl = makeashorterlink($origin);


my $chan_color = channel_color($channel, $style) if defined $channel;
my $page_title = get_page_title($page) if defined $page;

my $youtube_id = get_youtube_id($page) if defined $page and $origin->host =~ /youtube/i;
my $youtube_title = get_youtube_title($page) if defined $youtube_id;
my $youtube_duration = get_youtube_duration($page) if defined $youtube_id;

my $imdb_id = get_imdb_id($page) if defined $page and $origin->host =~ /imdb/i;
my $imdb_title = get_imdb_title($page) if defined $imdb_id;
my $imdb_duration = get_imdb_duration($page) if defined $imdb_id;

my $steam_id = get_steam_id($page) if defined $page and $origin->host =~ /steam/i;
my $steam_title = $page_title;
#my $steam_desc = get_steam_desc($page) if defined $steam_id;

#my $youtube_desc = get_youtube_desc($page);
#my $youtube_keywords = get_youtube_keywords($page);

$channel = " from $chan_color<$channel>${RESET}" if defined $channel and defined $chan_color;
$channel = " from <$channel>" if defined $channel and !defined $chan_color;
$channel = "" if !defined $channel;

$youtube_id = "${YELLOW}[$youtube_id]${RESET}" if defined $youtube_id;
$youtube_duration = " ${RED}($youtube_duration)${RESET}" if defined $youtube_duration;

$imdb_id = "${YELLOW}[$imdb_id]${RESET}" if defined $imdb_id;
$imdb_duration = " ${RED}($imdb_duration)${RESET}" if defined $imdb_duration;

$steam_id = "${YELLOW}[$steam_id]${RESET}" if defined $steam_id;

my $output = "";

$output .= "${RESET}${GREEN}$tinyurl ::${RESET} " if defined $tinyurl;

if (defined $youtube_id and defined $youtube_title and defined $youtube_duration) {
    $output .= "${RESET}YouTube $youtube_id$channel is $youtube_title$youtube_duration\n";
} elsif (defined $youtube_id and defined $youtube_title) {
    $output .= "${RESET}YouTube $youtube_id$channel is $youtube_title\n";
} elsif (defined $imdb_id and defined $imdb_title and defined $imdb_duration) {
    $output .= "${RESET}IMDB $imdb_id$channel is $imdb_title$imdb_duration\n";
} elsif (defined $imdb_id and defined $imdb_title) {
    $output .= "${RESET}IMDB $imdb_id$channel is $imdb_title\n";
} elsif (defined $steam_id and defined $steam_title) {
    $output .= "${RESET}Steam $steam_id$channel is $steam_title\n";
} elsif (defined $origin) {
    my $origin_host = $origin->host;
    my $title_bit = "";
    $title_bit = " is ${YELLOW}$page_title${RESET}" if defined $page_title;
    my $given_bit = "URL";
    $given_bit = "$given_host URL" if $given_host ne $origin_host;
    my $from_bit = " goes to $origin_host";
    $from_bit = " from $origin_host" if defined $page_title;
    
    $output .= "${RESET}$given_bit$channel$title_bit$from_bit\n";
    #print STDERR "DEBUG: " . Dumper($origin) . "\n";
}

if ( $style eq "ansi" ) {
    print pinkfish_to_ansi($output);
} else {
    print $output;
}

