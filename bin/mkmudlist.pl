#!/usr/bin/perl -w
#/usr/bin/perl -w -CSAD

use strict;
use utf8;
#binmode(STDOUT, ":utf8");
#binmode(STDERR, ":utf8");
use English qw( −no_match_vars );
use Data::Dumper;

use Time::HiRes qw(sleep time alarm);
use Net::Telnet qw(TELNET_IAC TELNET_SB TELNET_SE TELOPT_TTYPE);
use DBI;
use CGI;
use POSIX qw(strftime);
use HTML::Entities;
use HTML::FromANSI; # Not used internally now, but used by stored proc.
use Term::ANSIColor;
use Getopt::Long;
use URI;
use HTTP::Request::Common qw(POST);
use LWP::UserAgent;
use HTML::TokeParser;
use HTML::TableExtract;
use MIME::Base64;
use Image::ANSI;
use Encode;
use Parallel::ForkManager 0.7.6;
use JSON qw(encode_json decode_json);
use Encode qw( encode_utf8 );
use Digest::SHA qw(sha256_hex);
use Digest::MD5 qw(md5_hex);

$HTML::FromANSI::Options{fill_cols} = 1;
$HTML::FromANSI::Options{linewrap} = 0;
$HTML::FromANSI::Options{html_entitiy} = 1;
$HTML::FromANSI::Options{cols} = 80;
#$HTML::FromANSI::Options{rows} = 24;
$HTML::FromANSI::Options{lf_to_crlf} = 1;
$HTML::FromANSI::Options{show_cursor} = 0;
$HTML::FromANSI::Options{style} = "line-height: 1.0; letter-spacing: 0; font-size: 12pt";

my $mssp_url = 'https://mudhalla.net/tintin/protocols/mssp/mudlist.html';
my $mssp_page = "/home/www/log/data/mssp_mudlist.html";
my $filename = "/home/www/log/data/mudlist.json";
my $mssp_filename = "/home/www/log/data/mssp_mudlist.json";
my $imagedir = "/home/www/log/gfx/mud";
my $thread_count = 60;
my @global_tmp = ();
my @result_set = ();
our $default_ttype = "ansi";

#do_build();
do_mssp();
exit 0;

sub ansi2png {
    my $ansi = shift;

    my $img = Image::ANSI->new( string => $ansi );
    my %opts = ( mode => "full", font => undef, palette => undef );
    my $png = $img->as_png( %opts );

    return $png;
}

sub subopt_callback {
    my ($biteme, $option, $parameters) = @_;
    my $telcmd;

    if ($option == TELOPT_TTYPE) {
        $telcmd = pack("C4 A* C2", TELNET_IAC, TELNET_SB, TELOPT_TTYPE, 0,
                       $default_ttype, TELNET_IAC, TELNET_SE);
        $biteme->put(String => $telcmd, Telnetmode => 0);
        printf STDERR "                Replying to TTYPE with %s\n", $default_ttype;
    }

    1;
}

sub fetch_login_screen {
    my $site = shift;
    my $port = shift;

    return undef if !defined $site or !defined $port;

    my $telnet = Net::Telnet->new( Timeout => 30 );
    my $connect = 0;
    my @tlines = ();

    $telnet->option_callback(sub {});
    $telnet->suboption_callback(\&subopt_callback);
    $telnet->option_accept(Do => TELOPT_TTYPE);

    eval { $connect = ($telnet->open( Host => $site, Port => $port )); };
    if ( $EVAL_ERROR and $EVAL_ERROR =~ /unknown remote host/i ) {
        eval { $connect = ($telnet->open( Host => $site, Port => $port )); };
    }

    if ( $connect ) {
        my $tline = undef;
        $telnet->timeout(30);
        $telnet->errmode("return");
        push @tlines, $tline while( $tline = $telnet->getline());
        #my $first_line = $telnet->getline( Timeout => 10 );
        #if( defined $first_line ) {
        #    my @other_lines = $telnet->getlines( Timeout => 20, All => 0 );
        #    @tlines = ($first_line, @other_lines);
        #}
        #@tlines= $telnet->getlines( Timeout => 20, All => 0 );
        #chomp foreach (@tlines);
        $telnet->close();
        $telnet->timeout(30);
        $telnet->errmode("return");
    }

    my ($screen, $html, $png);

    $screen = ($connect and (scalar @tlines) > 0)  ? join("", @tlines) : undef;
    eval {
        $html = ($connect and (defined $screen) and (length $screen) > 0)  ? ansi2html($screen) : undef;
    };
    eval {
        $png  = ($connect and (defined $screen) and (length $screen) > 0)  ? ansi2png($screen) : undef;
    };

    #print STDERR "No ANSI data from $site:$port\n" if !defined $screen;
    #print STDERR "No HTML data from $site:$port\n" if !defined $html;
    #print STDERR "No PNG data from $site:$port\n" if !defined $png;
    return ($connect, $screen, $html, $png);
}

sub process_login {
    my $result = shift;

    return if !defined $result;
    return if !defined $result->{'ipaddress'};
    return if !defined $result->{'port'};
    my $mud = $result->{'name'};
    my $site = $result->{'ipaddress'};
    my $port = $result->{'port'};
    print STDERR "        Fetching $mud from $site $port\n";
    my ($connect, $screen, $html, $png) = fetch_login_screen($site, $port);
    if( $connect and defined $screen) {
        my $b64ansi = $screen ? encode_base64($screen) : encode_base64('');
        my $b64html = $html ? encode_base64($html) : undef;
        my $b64png  = $png ? encode_base64($png) : undef;

        $result->{'ansi'} = $b64ansi;
        $result->{'html'} = $b64html;
        $result->{'png'} = $b64png;
        $result->{'login'} = 1;
    } else {
        $result->{'ansi'} = undef;
        $result->{'html'} = undef;
        $result->{'png'} = undef;
        $result->{'login'} = 0;
    }
}

sub dump_png {
    my $result = shift;
    return if !defined $result;
    return if !defined $result->{'png'};
    return if !defined $result->{'name'};

    printf STDERR "            Saving login screen for %s from %s %d\n", $result->{'name'}, $result->{'ipaddress'}, $result->{'port'};
    my $mudname = $result->{'name'};
    $mudname = md5_hex($mudname);
    open FP, ">", "$imagedir/$mudname.png" or return;
    print FP decode_base64($result->{'png'});
    close FP;
}

sub geoip_locate {
    my $result = shift;
    return if !defined $result;
    return if !defined $result->{'ipaddress'};
    return if !defined $result->{'name'};

    printf STDERR "            Fetching geographic location for %s\n", $result->{'ipaddress'};
    my $output = {};
    my $mudname = $result->{'name'};
    $mudname = md5_hex($mudname);

    # curl 'https://freegeoip.app/json/104.156.100.167'
    #my $url = "https://freegeoip.app/json/" . $result->{'ipaddress'};
    my $url = "https://freegeoip.live/json/" . $result->{'ipaddress'};
    my $json = undef;
    my $data = undef;
    open(FP, "-|", "/usr/bin/curl", "-s", $url) or die "Can't open /usr/bin/curl $!";
    {
        local $/ = undef;
        $json = <FP>;
    }
    close FP;
    $data = decode_json($json) if defined $json;

    open FP, ">", "$imagedir/$mudname.json" or return;
    $output->{'ipaddress'} = $result->{'ipaddress'};
    $output->{'name'} = $result->{'name'};
    $output->{'geoip'} = $data;
    $output->{'login'} = $result->{'login'};
    # curl 'https://freegeoip.app/json/104.156.100.167'
    print FP encode_json($output);
    close FP;
}

sub get_mudlist {
    my $pm = shift;

    print STDERR "\nParsing $filename ...";
    my $page = do {
        local $/ = undef; # No record seperator
        open my $fp, "<", $filename or die "Cannot open $filename: $!";
        <$fp>;
    };
    my $json = decode_json(encode_utf8($page));
    print STDERR "done.";
    print STDERR "\nParsing $mssp_filename ...";
    my $mssp_page = do {
        local $/ = undef; # No record seperator
        open my $fp, "<", $mssp_filename or warn "Cannot open $mssp_filename: $!";
        <$fp>;
    };
    my $mssp_json = decode_json(encode_utf8($mssp_page)) if defined $mssp_page;

    if(defined $page or defined $mssp_page) {
        print STDERR "done, processing ...";
        my @mudlist = ();
        push @mudlist, @{ $json->{mudlist} } if defined $page;
        push @mudlist, @{ $mssp_json->{mudlist} } if defined $mssp_page;
        # Each entry should be key/value for name, type, mudlib, ipaddress, port, online.
        #print STDERR "MUDLIST: " . Dumper(\@mudlist) . "\n";
        #splice @mudlist, 10;
        #print STDERR "Trimming down to " . (scalar @mudlist) . " for testing.\n";
        #exit 1;

        print STDERR "found " . (scalar @mudlist) . " entries.\n";
        my $set_size = scalar @mudlist;
        for(my $i = 0; $i < $set_size; $i++) {
            $pm->start($i) and next;

            my $result = $mudlist[$i];
            $result->{'index'} = $i;
            printf STDERR "    Processing %s (%03d to go)\n", $result->{'name'}, $set_size - $i;
            process_login($result) if defined $result->{'ipaddress'} and defined $result->{'port'};
            dump_png($result) if defined $result->{'png'};
            geoip_locate($result) if defined $result->{'ipaddress'};

            $pm->finish(0, \$result);
        }
        $pm->wait_all_children;

        push @result_set, @global_tmp;
        @global_tmp = ();
    }
}

sub do_build {
    my $pm = Parallel::ForkManager->new($thread_count);

    # Define our callback for each thread, to collect the results.
    $pm->run_on_finish(
        sub {
            my ($pid, $exit_code, $ident, $exit_signal, $core_dump, $data_reference) = @_;
            #print STDERR "PID $pid, EXIT $exit_code, IDENT $ident, SIG $exit_signal, CORE $core_dump\n";
            if( defined $data_reference ) {
                my $result = ${ $data_reference };
                push @global_tmp, $result;
            } else {
                #print STDERR "NO DATA\n";
            }
        }
    );

    get_mudlist($pm);
    #print "\nFINAL:\n" . Dumper(\@result_set);
    print "\nDONE!\n";
}

sub get_url {
    my $url = shift;

    return undef if !defined $url;
    my $lwp = LWP::UserAgent->new();
       $lwp->timeout(30);
    my $request = HTTP::Request->new(GET => $url);
    my $response = undef;
    #print "Fetching... $url\n";
    if($request) {
        eval {
            local $SIG{ALRM} = sub { die "Exceeded Timeout of 30 seconds for $url\n" };
            alarm 30;
            $response = $lwp->request($request);
            alarm 0;
        };
        warn "Timeout" if($EVAL_ERROR and ($EVAL_ERROR =~ /^Exceeded Timeout/));
        return $response->content if((defined $response) and $response->is_success);
    }
    return undef;
}

sub do_mssp {
    print STDERR "\nFetching $mssp_url ...";
    #my $page = get_url($mssp_url);

    system "/usr/bin/wget -O '$mssp_page' '$mssp_url'";
    my $page = do {
        local $/ = undef; # No record seperator
        open my $fp, "<:encoding(UTF-8)", $mssp_page or die "Cannot open $mssp_page: $!";
        <$fp>;
    };

    # The fields show up in this order (currently)... with lots of
    # other fields we don't care about in between them.
    #
    # </span><span style='color:#FF5'>┌──
    #
    # <span styleblahblah>     PORT</span>              -- port
    # <span styelblahblah>     1234</span>
    #
    # <span styleblahblah>     CODEBASE</span>          -- mudlib
    # <span styelblahblah>     text</span>
    #
    # <span styleblahblah>     IP</span>                -- ipaddress
    # <span styelblahblah>     123.123.123.123</span>
    #
    # <span styleblahblah>     NAME</span>              -- name
    # <span styelblahblah>     Foo MUD</span>
    #
    # <span styleblahblah>     FAMILY</span>            -- type
    # <span styelblahblah>     text</span>
    #
    # ├────
    #
    # ──────┘
    # </span>
    #

    printf "Page is %d characters long\n", (length $page);
    my @entries = (
        #$page =~ /(<span\s+style='.*?'>\s*?[\u250F][\u2500\u2501].*?[\u2500\u2501][\u2508\u2509]\s*<\/span>)/gs
        #$page =~ /([\u250C\u250D\u250E\u250F][\u2500\u2501]+.*?[\u2500\u2501]+[\u2518\u2519\u251A\u251B])/gs
        #$page =~ /(┌.*?┘)/gs
        #$page =~ /<span style='color:#FF5'>┌─.*?─┘/gs
        #$page =~ /([\u250c].*?[\u2518])/gs
        $page =~ /([\x{250C}].*?[\x{2518}])/gs
    );
    printf "Got %d entries\n", (scalar @entries);
    my @mssp_list = ();
    foreach my $entry (@entries) {
        #printf "--------\n";
        #printf "%s\n", $entry;
        #printf "--------\n";

        $entry =~ /<span.*?>\s*?NAME\s*?<\/span>\s*<span.*?>\s*(.*?)\s*<\/span>/s;
        my $name = ($1);
        $entry =~ /<span.*?>\s*?IP\s*?<\/span>\s*<span.*?>\s*(.*?)\s*<\/span>/s;
        my $ipaddress = ($1);
        $entry =~ /<span.*?>\s*?PORT\s*?<\/span>\s*<span.*?>\s*(.*?)\s*<\/span>/s;
        my $port = ($1);
        $entry =~ /<span.*?>\s*?FAMILY\s*?<\/span>\s*<span.*?>\s*(.*?)\s*<\/span>/s;
        my $type = ($1);
        $entry =~ /<span.*?>\s*?CODEBASE\s*?<\/span>\s*<span.*?>\s*(.*?)\s*<\/span>/s;
        my $mudlib = ($1);

        next if !defined $name or -z $name;
        next if !defined $ipaddress or -z $ipaddress;
        next if !defined $port or -z $port;

        $port = 0 + $port; # Convert to integer
        $type = "---" if -z $type;
        $mudlib = "---" if -z $mudlib;

        my $data = {
            name        => $name,
            md5         => md5_hex($name),
            type        => $type,
            mudlib      => $mudlib,
            ipaddress   => $ipaddress,
            port        => $port,
            online      => 0,
            from_mssp   => 1,
        };
        push @mssp_list, $data;
        printf "%s  %s:%s  type: %s mudlib: %s\n", $name, $ipaddress, $port, $type, $mudlib;
    }

    #my $dump = { mudlist => \@mssp_list };
    #my $json_dump = JSON->new->utf8->allow_nonref->canonical->pretty->encode({ mudlist => \@mssp_list });
    #printf "--------\n";
    #printf "%s", $json_dump;
    #printf "--------\n";

    print STDERR "\nParsing $filename ...";
    my $mudlist_page = do {
        local $/ = undef; # No record seperator
        open my $fp, "<", $filename or die "Cannot open $filename: $!";
        <$fp>;
    };
    print STDERR "done.\n";
    my $mudlist_json = decode_json(encode_utf8($mudlist_page));
    my @mudlist = @{ $mudlist_json->{mudlist} };
    printf "Have %d mudlist entries in JSON file\n", (scalar @mudlist);

    my @new_mssp = ();
    foreach my $mssp (@mssp_list) {
        my @name_dup = grep  {lc $_->{name} eq lc $mssp->{name}} (@mudlist);
        my @ip_dup = grep { lc $_->{ipaddress} eq lc $mssp->{ipaddress} and lc $_->{port} eq lc $mssp->{port}} (@mudlist);
        if ((scalar @name_dup) > 0) {
            printf "Duplicate by name   %s %s:%s %s\n", $mssp->{name}, $mssp->{ipaddress}, $mssp->{port}, $mssp->{md5};
            next;
        }
        if ((scalar @ip_dup) > 0) {
            printf "Duplicate by ip:    %s %s:%s %s\n", $mssp->{name}, $mssp->{ipaddress}, $mssp->{port}, $mssp->{md5};
            next;
        }
        push @new_mssp, $mssp;
    }

    printf "Found %d new muds to process!\n", (scalar @new_mssp);
    my $dump = JSON->new->utf8->allow_nonref->canonical->pretty->encode({ mudlist => \@new_mssp });
    open FP, ">:encoding(UTF-8)", "$mssp_filename" or die "Cannot open MSSP JSON file for output: $!";
    print FP "$dump\n";
    close FP;

    # At this point, we need to do what do_build() and get_mudlist() do to process
    # all the muds NOT already done by those... or merge this into that so we
    # can do both at once from both files...

    do_build();
}

