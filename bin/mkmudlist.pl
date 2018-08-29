#!/usr/bin/perl -w

use strict;
use utf8;
use English;
use Data::Dumper;

use Time::HiRes qw(sleep time alarm);
use Net::Telnet;
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
use JSON qw(decode_json);
use Encode qw( encode_utf8 );

$HTML::FromANSI::Options{fill_cols} = 1;
$HTML::FromANSI::Options{linewrap} = 0;
$HTML::FromANSI::Options{html_entitiy} = 1;
$HTML::FromANSI::Options{cols} = 80;
#$HTML::FromANSI::Options{rows} = 24;
$HTML::FromANSI::Options{lf_to_crlf} = 1;
$HTML::FromANSI::Options{show_cursor} = 0;
$HTML::FromANSI::Options{style} = "line-height: 1.0; letter-spacing: 0; font-size: 12pt";

my $filename = "/home/wiley/public_html/mudlist.json";
my $imagedir = "/home/wiley/public_html/gfx/mud";
my $thread_count = 20;
my @global_tmp = ();
my @result_set = ();

do_build();
exit 0;

sub ansi2png {
    my $ansi = shift;

    my $img = Image::ANSI->new( string => $ansi );
    my %opts = ( mode => "full", font => undef, palette => undef );
    my $png = $img->as_png( %opts );

    return $png;
}

sub fetch_login_screen {
    my $site = shift;
    my $port = shift;

    return undef if !defined $site or !defined $port;

    my $telnet = Net::Telnet->new( Timeout => 10 );
    my $connect = 0;
    my @tlines = ();

    eval { $connect = ($telnet->open( Host => $site, Port => $port )); };
    if ( $EVAL_ERROR and $EVAL_ERROR =~ /unknown remote host/i ) {
        eval { $connect = ($telnet->open( Host => $site, Port => $port )); };
    }

    if ( $connect ) {
        my $tline = undef;
        $telnet->timeout(2);
        $telnet->errmode("return");
        push @tlines, $tline while( $tline = $telnet->getline());
        $telnet->close();
        $telnet->timeout(10);
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
    open FP, ">", "$imagedir/$mudname.png" or return;
    print FP decode_base64($result->{'png'});
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
    if(defined $page) {
        print STDERR "done, processing ...";
        my @mudlist = @{ $json->{mudlist} };
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

