#!/usr/bin/perl -w

use strict;
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

if(scalar @ARGV != 1) {
    print "Usage:  $0 < build | verify >\n";
    exit 0;
}

my $get_details = 0;

$HTML::FromANSI::Options{fill_cols} = 1;
$HTML::FromANSI::Options{linewrap} = 0;
$HTML::FromANSI::Options{html_entitiy} = 1;
$HTML::FromANSI::Options{cols} = 80;
#$HTML::FromANSI::Options{rows} = 24;
$HTML::FromANSI::Options{lf_to_crlf} = 1;
$HTML::FromANSI::Options{show_cursor} = 0;
$HTML::FromANSI::Options{style} = "line-height: 1.0; letter-spacing: 0; font-size: 12pt";
 
my $dbc = DBI->connect("DBI:Pg:dbname=mudlist;host=localhost;port=5432;sslmode=prefer", "quixadhal", "tardis69", { AutoCommit => 0, PrintError => 0, });

my $key_field = 'mud_id';
my $count = $dbc->selectrow_arrayref(qq!

  SELECT count(*)
    FROM mudlist
   WHERE live

!)->[0];
my $last_verified;

if( $count > 0 ) {
    $last_verified = $dbc->selectrow_arrayref(qq!

      SELECT to_char(last_verified, 'HH:MI:SS am on FMDay, DD FMMonth, YYYY TZ') 
        FROM mudlist
       WHERE live
    ORDER BY last_verified DESC
       LIMIT 1

    !)->[0];
} else {
    $last_verified = $dbc->selectrow_arrayref(qq!

      SELECT to_char(now() - interval '30 years', 'HH:MI:SS am on FMDay, DD FMMonth, YYYY TZ') 

    !)->[0];
}

#print STDERR "There were $count MUDs listed at $last_verified\n";

my $sth_live = $dbc->prepare( qq!
    UPDATE mudlist SET last_verified = now(), live = ?
    WHERE mud_id = ?
!);
my $sth_update = $dbc->prepare( qq!
    UPDATE mudlist SET last_verified = now(), name = ?, live = ?, ansi_login = ?, html_login = ?, png_login = ?
    WHERE mud_id = ?
!);

if($ARGV[0] eq 'build') {
    do_build();
} elsif($ARGV[0] eq 'verify') {
    do_verify_all();
} else {
    print "Usage:  $0 < build | verify >\n";
    exit 0;
}

exit 1;

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

  $screen  = ($connect and (scalar @tlines) > 0)  ? join("", @tlines) : undef;
  eval {
    $html    = ($connect and (defined $screen) and (length $screen) > 0)  ? ansi2html($screen) : undef;
  };
  eval {
    $png     = ($connect and (defined $screen) and (length $screen) > 0)  ? ansi2png($screen) : undef;
  };

  return ($connect, $screen, $html, $png);
}


sub do_verify_all {
    my $key_field = 'mud_id';
#        WHERE ansi_login IS NULL AND live
    my $big_list = $dbc->selectall_hashref(qq!
        SELECT mud_id, name, site, port, live
        FROM mudlist
    !, $key_field);

    my $pm = Parallel::ForkManager->new(20);

    foreach my $id (sort { $a <=> $b } keys %$big_list) {
        $pm->start() and next;
        do_verify($big_list->{$id});
        $pm->finish(0);
    }
    $pm->wait_all_children;
}

sub do_verify {
    my $data = shift;
    return if !defined $data;

    $dbc = undef;
    sleep 0.5;
    $dbc = DBI->connect("DBI:Pg:dbname=mudlist;host=localhost;port=5432;sslmode=prefer", "quixadhal", "tardis69", { AutoCommit => 0, PrintError => 0, });
    my $id = $data->{'mud_id'};
    my $name = $data->{'name'};
    my $site = $data->{'site'};
    my $port = $data->{'port'};
    my ($connect, $screen, $html, $png) = fetch_login_screen($site, $port);
    if ($connect) {
        my $b64ansi = $screen ? encode_base64($screen) : encode_base64('');
        my $b64html = $html ? encode_base64($html) : undef;
        my $b64png  = $png ? encode_base64($png) : undef;

        if ($screen) {
            my $rv_update = $sth_update->execute($name, 't', $b64ansi, $b64html, $b64png, $id);
            if($rv_update) {
                $dbc->commit;
                print " UPDATE: $id\t$site\t$port online with login\n";
            } else {
                $dbc->rollback;
                print "   FAIL: $id\t$site\t$port online with login\n";
            }
        } else {
            my $rv_live = $sth_live->execute('t', $id);
            if($rv_live) {
                $dbc->commit;
                print " UPDATE: $id\t$site\t$port online\n";
            } else {
                $dbc->rollback;
                print "   FAIL: $id\t$site\t$port online\n";
            }
        }
    } else {
        my $rv_live = $sth_live->execute('f', $id);
        if($rv_live) {
            $dbc->commit;
            print " UPDATE: $id\t$site\t$port offline\n";
        } else {
            $dbc->rollback;
            print "   FAIL: $id\t$site\t$port offline\n";
        }
    }
    $dbc = undef;
}

my @global_tmp = ();
my @result_set = ();

sub get_tmc {
    my $pm = shift;

    my $timeout = 90;
    my $lwp = LWP::UserAgent->new();
       $lwp->timeout($timeout/2);
    my @url_list = qw(
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=DIGIT
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=a
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=b
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=c
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=d
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=e
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=f
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=g
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=h
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=i
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=j
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=k
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=l
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=m
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=n
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=o
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=p
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=q
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=r
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=s
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=t
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=u
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=v
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=w
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=x
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=y
        http://mudconnect.com/cgi-bin/mud_list.cgi?letter=z
    );

    foreach my $url (@url_list) {
        print STDERR "\nFetching $url ...";
        my $page = get_url( $url );
        print STDERR "done, processing ...";
        sleep (0.25 + rand() * 2.0);
        next if !defined $page;

        push @result_set, process_list_page($page);
        print STDERR "done. (" . (scalar @result_set) . " results so far)";
    }
    print STDERR "\n";

    # Now, we need to fork so this doesn't take so long...

    my $set_size = scalar @result_set;
    for(my $i = 0; $i < $set_size; $i++) {
        $pm->start($i) and next;

        my $result = $result_set[$i];

        $result->{'index'} = $i;

        sleep (0.25 + rand() * 2.0);
        my $page = get_url( $result->{'url'} );
        process_result_page($result, $page) if defined $page;
        process_login($result) if defined $result->{'site'} and defined $result->{'port'};
        printf STDERR "Got details and login screen from %s (%03d to go)\n", $result->{'url'}, $set_size - $i;

        $pm->finish(0, \$result);
    }
    $pm->wait_all_children;

    # Now, grab the I3 mudlist from my own MUD

    @result_set = @global_tmp;
    @global_tmp = ();
}

sub get_i3 {
    my $pm = shift;

    my $url = "http://192.168.1.11:5001/cgi/mudlist.c";
    print STDERR "\nFetching $url ...";
    my $page = get_url( $url );
    print STDERR "done, processing ...";
    if(defined $page) {
        my $set_size = scalar @result_set;
        my @mudlist_results = process_mudlist_page($page, \@result_set);
        my $mudlist_set_size = scalar @mudlist_results;
        for(my $i = 0; $i < $mudlist_set_size; $i++) {
            $pm->start($i) and next;

            my $result = $mudlist_results[$i];
            $result->{'index'} = $i + $set_size;
            process_login($result) if defined $result->{'site'} and defined $result->{'port'};
            printf STDERR "Got login screen from %s (%03d to go)\n", $result->{'url'}, $mudlist_set_size - $i;

            $pm->finish(0, \$result);
        }
        $pm->wait_all_children;

        push @result_set, @global_tmp;
        @global_tmp = ();
    }
}

sub get_json {
    my $pm = shift;

    my $url = "http://tintin.sourceforge.net/mssp/mudlist.json";
    print STDERR "\nFetching $url ...";
    my $page = get_url( $url );
    print STDERR "done, processing ...";
    if(defined $page) {
        my $set_size = scalar @result_set;
        my @mudlist_results = process_json_page($page, \@result_set);
        my $mudlist_set_size = scalar @mudlist_results;

        push @result_set, @mudlist_results;
        return;

        for(my $i = 0; $i < $mudlist_set_size; $i++) {
            $pm->start($i) and next;

            my $result = $mudlist_results[$i];
            $result->{'index'} = $i + $set_size;
            process_login($result) if defined $result->{'site'} and defined $result->{'port'};
            printf STDERR "Got login screen from %s (%03d to go)\n", $result->{'url'}, $mudlist_set_size - $i;

            $pm->finish(0, \$result);
        }
        $pm->wait_all_children;

        push @result_set, @global_tmp;
        @global_tmp = ();
    }
}

sub do_build {
    $dbc = undef;

    my $pm = Parallel::ForkManager->new(20);

    $pm->run_on_finish(
        sub {
            my ($pid, $exit_code, $ident, $exit_signal, $core_dump, $data_reference) = @_;
            print STDERR "PID $pid, EXIT $exit_code, IDENT $ident, SIG $exit_signal, CORE $core_dump\n";
            if( defined $data_reference ) {
                my $result = ${ $data_reference };
                push @global_tmp, $result;
            } else {
                print STDERR "NO DATA\n";
            }
        }
    );

    #get_tmc($pm);
    #get_i3($pm);
    get_json($pm);

    #sql_update(@result_set);
    print "\nFINAL:\n" . Dumper(\@result_set);
}

sub process_json_page {
    my $page = shift;
    my $result_set = shift;
    my @results;

    return undef if !defined $page;
    my $json = decode_json($page);

    foreach my $mud (sort keys %$json) {
        my @bits = split /:/, $mud;

        my $host = (exists $json->{$mud}->{'LAST HOST'}) ? $json->{$mud}->{'LAST HOST'} : $bits[0];
        my $port = (exists $json->{$mud}->{'LAST PORT'}) ? $json->{$mud}->{'LAST PORT'} : $bits[1];
        my $name = (exists $json->{$mud}->{'NAME'}) ? $json->{$mud}->{'NAME'} : $mud;
        my $ipaddr = (exists $json->{$mud}->{'IP'}) ? $json->{$mud}->{'IP'} : undef;
        my $type = (exists $json->{$mud}->{'CODEBASE'}) ? $json->{$mud}->{'CODEBASE'} : undef;
        my $url = (exists $json->{$mud}->{'WEBSITE'}) ? $json->{$mud}->{'WEBSITE'} : undef;

        my %data = (
            'site'      => $host,
            'port'      => $port,
            'ipaddr'    => $ipaddr,
            'url'       => $url,
            'name'      => $name,
            'type'      => $type,
            'updated'   => strftime("%B %e, %Y", localtime(time)),
        );

        push @results, \%data if !(grep {$_->{'name'} eq $name} @$result_set);
    }

    print STDERR "Got " . (scalar @results) . " results...\n";
    #print STDERR "\nMUDLIST:\n" . Dumper(\@results);
    return @results;
}

sub process_mudlist_page {
    my $page = shift;
    my $result_set = shift;
    my @results;

    return undef if !defined $page;
    my @rows = grep /<tr\s+class\=\"entry\"/, (split /\n/, $page);
    print STDERR "Got " . (scalar @rows) . " rows...\n";
    foreach my $row (@rows) {
        $row =~ s/^<tr\s*[^>]*?><td\s*[^>]*?>//;
        $row =~ s/<\/td><\/tr>$//;
        $row =~ s/\&nbsp;/ /gmix;
        my @cols = split /<\/td><td\s*[^>]*?>/, $row;
        $cols[0] =~ s/[\x00-\x1F\x7F-\xFF]//gmix;
        $cols[0] =~ s/<\/?a[^>]*?>//gmix;
        $cols[4] =~ s/<\/?a[^>]*?>//gmix;
        $cols[5] = 0 + $cols[5];
        next if length $cols[0] < 1;
        my %data = (
            'site'      => $cols[4],
            'port'      => $cols[5],
            'ipaddr'    => $cols[4],
            'url'       => "http://" . $cols[4] . "/",
            'name'      => $cols[0],
            'type'      => $cols[1],
            'detail'    => $cols[3],
            'updated'   => strftime("%B %e, %Y", localtime(time)),
        );

        push @results, \%data if !(grep {$_->{'name'} eq $cols[0]} @$result_set);
    }

    print STDERR "Got " . (scalar @results) . " results...\n";
    #print STDERR "\nMUDLIST:\n" . Dumper(\@results);
    return @results;
}

sub process_result_page {
    my $result = shift;
    my $page = shift;

    return if !defined $page;
    $page =~ /site:\s+(.*?)\s+(\d+)\s+\[((?:\d+\.\d+\.\d+\.\d+)|(?:[\w\.?]+))\]\s*<br\s*\/>/gsmix;
    my ($site, $port, $ipaddr) = ($1, $2, $3);
    #print "$site $port $ipaddr\n";
    return if !defined $site;
    return if !defined $port;
    $result->{'site'} = $site;
    $result->{'port'} = $port;
    $result->{'ipaddr'} = $ipaddr if defined $ipaddr;
}

sub process_login {
    my $result = shift;

    return if !defined $result;
    return if !defined $result->{'site'};
    return if !defined $result->{'port'};
    my $mud = $result->{'name'};
    my $site = $result->{'site'};
    my $port = $result->{'port'};
    #print STDERR "Trying $mud $site $port\n";
    my ($connect, $screen, $html, $png) = fetch_login_screen($site, $port);
    if( $connect and defined $screen) {
        my $b64ansi = $screen ? encode_base64($screen) : encode_base64('');
        my $b64html = $html ? encode_base64($html) : undef;
        my $b64png  = $png ? encode_base64($png) : undef;

        $result->{'ansi'} = $b64ansi;
        $result->{'html'} = $b64html;
        $result->{'png'} = $b64png;
        $result->{'connect'} = 1;
    } else {
        $result->{'ansi'} = undef;
        $result->{'html'} = undef;
        $result->{'png'} = undef;
        $result->{'connect'} = 0;
    }
}

sub sql_update {
    my @result_set = @_;

    $dbc = undef;
    sleep 0.5;
    $dbc = DBI->connect("DBI:Pg:dbname=mudlist;host=localhost;port=5432;sslmode=prefer", "quixadhal", "tardis69", { AutoCommit => 0, PrintError => 0, });
    my $sth = $dbc->prepare( qq!
        INSERT INTO mudlist (name, site, port, ansi_login, html_login, png_login, live, codebase)
        VALUES (?,?,?,?,?,?,?,?)
    !);
    my $sth2 = $dbc->prepare( qq!
        UPDATE mudlist SET last_verified = now(), live = ?, ansi_login = ?, html_login = ?, png_login = ?, codebase = ?
        WHERE name = ?
    !);

    foreach my $result (@result_set) {
        next if !exists $result->{'connect'};
        my $mud = $result->{'name'};
        my $site = $result->{'site'};
        my $port = $result->{'port'};
        my $ansi = $result->{'ansi'};
        my $html = $result->{'html'};
        my $png = $result->{'png'};
        my $code_base = $result->{'type'};

        if($result->{'connect'}) {
            my $rv = undef;
            $rv = $sth->execute($mud, $site, $port, $ansi, $html, $png, 't', $code_base);
            if($rv) {
                $dbc->commit;
                print " INSERT: $mud\t$site\t$port online\n";
            } else {
                if ($DBI::errstr =~ /duplicate key/) {
                    $dbc->rollback;
                    my $rv2 = $sth2->execute('t', $ansi, $html, $png, $mud, $code_base);
                    if($rv2) {
                        $dbc->commit;
                        print " UPDATE: $mud\t$site\t$port online\n";
                    } else {
                        $dbc->rollback;
                        print "   FAIL: $mud\t$site\t$port online\n";
                    }
                } else {
                    print STDERR $DBI::errstr."\n";
                    $dbc->rollback;
                    print "   FAIL: $mud\t$site\t$port online\n";
                }
            }
        } else {
            my $rv = undef;
            $rv = $sth->execute($mud, $site, $port, undef, undef, undef, 'f', $code_base);
            if($rv) {
                $dbc->commit;
                print " INSERT: $mud\t$site\t$port offline\n";
            } else {
                if ($DBI::errstr =~ /duplicate key/) {
                    $dbc->rollback;
                    my $rv2 = $sth2->execute('f', undef, undef, undef, $mud, $code_base);
                    if($rv2) {
                        $dbc->commit;
                        print " UPDATE: $mud\t$site\t$port offline\n";
                    } else {
                        $dbc->rollback;
                        print "   FAIL: $mud\t$site\t$port offline\n";
                    }
                } else {
                    print STDERR $DBI::errstr."\n";
                    $dbc->rollback;
                    print "   FAIL: $mud\t$site\t$port offline\n";
                }
            }
        }
    }
}

sub process_list_page {
    my $page = shift;
    my @results;

    #<center>'Non-Letter': 7 listings were found</center>
    #<center><table width=90%><tr><td align=left><ol>
    #<li><a href="/mud-bin/adv_search.cgi?Mode=MUD&mud=2001's+Eternal+Darkness">2001's Eternal Darkness</a> [[Circlemud] Circle 3.0] <font size=-1>(February 23, 2008)</font></font>
    #<li><a href="/mud-bin/adv_search.cgi?Mode=MUD&mud=3-Kingdoms">3-Kingdoms</a> [[LP] LDMud] <font size=-1>(June 3, 2009)</font></font>
    #<li><a href="/mud-bin/adv_search.cgi?Mode=MUD&mud=3Scapes">3Scapes</a> [[LP] LDMud] <font size=-1>(June 3, 2009)</font></font>
    #<li><a href="/mud-bin/adv_search.cgi?Mode=MUD&mud=4+Dimensions">4 Dimensions</a> [[Circlemud] bpl15 modified] <font size=-1>(April 12, 2010)</font></font>
    #<li><a href="/mud-bin/adv_search.cgi?Mode=MUD&mud=6+Dragons">6 Dragons</a> [[Smaug] Heavily Modified] <font size=-1>(October 3, 2009)</font></font>
    #<li><a href="/mud-bin/adv_search.cgi?Mode=MUD&mud=7+Degrees+of+Freedom">7 Degrees of Freedom</a> [[Aber] Modified] <font size=-1>(September 11, 2007)</font></font>
    #<li><a href="/mud-bin/adv_search.cgi?Mode=MUD&mud=8bitMUSH">8bitMUSH</a> [[MUSH] TinyBit (PennMUSH Derived)] <font size=-1>(April 5, 2011)</font></font>
    #</ol></td></tr></table></center>


    #<li><a href="/mud-bin/adv_search.cgi?Mode=MUD&mud=Aliens+vs.+Predator">Aliens vs. Predator</a> [[Circlemud] LexiMUD] <font size=-1>(August 8, 2009)</font></font>
    #<li><a href="/mud-bin/adv_search.cgi?Mode=MUD&mud=Almaren">Almaren</a> [Diku/HnS 1.0] <font size=-1>(October 8, 1998)</font></font>
    #<li><a href="/mud-bin/adv_search.cgi?Mode=MUD&mud=Alter+Aeon">Alter Aeon</a> [[Custom] DentinMud 2.20] <font size=-1>(April 20, 2012)</font></font>


    return undef if !defined $page;
    my $url_base = 'http://mudconnect.com';
    $page =~ /<table\s+width=90%>\s*<tr><td\s+align=left>\s*<ol>\s*(.*?)\s*<\/ol>\s*<\/td>\s*<\/tr>\s*<\/table>/gsmix;
    my ($list) = ($1);
    return @results if !defined $list;
    while( $list =~ /\G<li>\s*<a\s+href="(.*?)">\s*(.*?)<\/a>\s*\[(?:\[(\w+)\]\s*)?(.*?)\]\s*<font\s+size=-1>\s*\(?(.*?)\)?\s*<\/font>(?:\s*<\/font>)?\s*/cgsmix )
    {
        my ($mud_url, $mud_name, $mud_type, $mud_detail, $last_updated) = ($1, $2, $3, $4, $5);
        next if !defined $mud_url;
        next if !defined $mud_name;
        #next if !defined $mud_type;
        #next if !defined $mud_detail;
        next if !defined $last_updated;

        $mud_type = "Custom" if defined $mud_detail and !defined $mud_type;
        next if !defined $mud_type;
        next if !defined $mud_detail;

        my %data = (
            'url'       => $url_base . $mud_url,
            'name'      => $mud_name,
            'type'      => $mud_type,
            'detail'    => $mud_detail,
            'updated'   => $last_updated,
        );
        push @results, \%data;
        #print STDERR "Found $mud_name\t$mud_type\t$mud_detail\t$last_updated\n";
    }
    return @results;
}

sub do_build_old {
  my $timeout = 90;
  my $lwp = LWP::UserAgent->new();
     $lwp->timeout($timeout/2);
  my $url = 'http://mudconnect.com/mud-bin/mud_biglist.cgi';
  my $counter = 0;
  my $batch = 0;
  my $start = 0;
  my $inc = 20;
  my $nextbatch = $start + $inc + 1;

  my $front_page = get_url( $url );
  if(defined $front_page) {
    ($batch, $counter) = process_page( $batch, $counter, $front_page );
    sleep rand() * 2.0;

    for(;;) {
      my $request = undef;
      $request = HTTP::Request->new(POST => $url);
      $request->content_type('application/x-www-form-urlencoded');
      $request->content("Mode=SPLIT&pred_start=$start&succ_start=$nextbatch&inc=$inc&navmode=Next");
      print STDERR "----- Mode=SPLIT&pred_start=$start&succ_start=$nextbatch&inc=$inc&Succ.x=16&Succ.y=17\n";
      print STDERR "----- Request: " . Dumper($request) . "\n";
      #POST /mud-bin/mud_biglist.cgi Mode=SPLIT&pred_start=42&succ_start=63&inc=20&Succ.x=28&Succ.y=16
      print STDERR "Obtaining mud list chunk: $start to $nextbatch by $inc\n";
      if($request) {
        my $response = undef;
        eval {
          local $SIG{ALRM} = sub { die "Exceeded Timeout of $timeout for $url\n" };
          alarm $timeout;
          #print STDERR "Pre-request\n";
          $response = $lwp->request($request);
          #print STDERR "Post-request\n";
          #print STDERR "----- Response: " . Dumper($response) . "\n";
          alarm 0;
        };
        die "Timeout" if($EVAL_ERROR and ($EVAL_ERROR =~ /^Exceeded Timeout/));
        $batch = 0;
        if((defined $response) and $response->is_success) {
          print STDERR "Got it!\n";
          my $web_page = $response->content;
  
          ($batch, $counter) = process_page( $batch, $counter, $web_page );
      
        } else {
          print STDERR (0 + $response->code())." Failed!\n".$response->message()."\n";
        }
      }
      last if $batch < 1;
      print "-------------------- BATCH [$start - " . ($nextbatch - 1) . "] --------------------\n";
      $start = $nextbatch;
      $nextbatch = $nextbatch + $inc + 1;
      sleep rand() * 2.0;
    }
  }

#<form method="POST" action="/mud-bin/mud_biglist.cgi">
#<input type="hidden" name="Mode" value="SPLIT">
#<input type="hidden" name="pred_start" value="0">
#<input type="hidden" name="succ_start" value="21">
#<table border=0 cellpadding=5 cellspacing=5>
#<tr><td>
#<font face="arial,helvetica">
#
#</td><td>
#<font face="arial,helvetica">
#<select name="inc">
#<option SELECTED value="20">Increment: 20<option  value="40">Increment: 40<option  value="60">Increment: 60<option   value="80">Increment: 80<option  value="100">Increment: 100<option  value="250">Increment: 250<option  value="ALL"> Jump To End 
#
#</select>
#</td><td>
#<font face="arial,helvetica">
#<input type="image" border=0 src="/images/right.gif" name="Succ">
#</table>
#</form>

  $dbc->disconnect();
  exit 1;
}

sub process_page {
  my $batch = shift;
  my $counter = shift;
  my $web_page = shift;
  my $detail_url = 'http://www.mudconnect.com/mud-bin/simple_search.cgi';
  my $timeout = 90;
  my $lwp = LWP::UserAgent->new();
     $lwp->timeout($timeout/2);

  my $output;
  #$output = table_get($web_page, "\t", { depth => 0, count => 2, keep_html => 1, headers => ["Check Mud", "Mud"], } );
  $output = table_get($web_page, "\t", { keep_html => 1, headers => ["Check Mud", "Mud"], } );
  my $sth = $dbc->prepare( qq!
    INSERT INTO mudlist (name, site, port, ansi_login, html_login, png_login, live, codebase)
    VALUES (?,?,?,?,?,?,?,?)
    !);
  my $sth2 = $dbc->prepare( qq!
    UPDATE mudlist SET last_verified = now(), live = ?, ansi_login = ?, html_login = ?, png_login = ?, codebase = ?
    WHERE name = ?
    !);
  foreach (@$output) {
    s/\n//g;
    $_ =~ /\?site\s*\=\s*([\w\.]+)\s*\&\s*port\s*\=\s*([\d]+).*?\&mud\=(.*?)\"/;
    my ($mud, $site, $port) = ($3, $1, $2);
    next if !(defined $mud) or !(defined $site) or !(defined $port);
    #$mud ||= '';
    #$site ||= '';
    #$port ||= 0;

    my $code_base = undef;
    my $date_created = undef;
    # This is the point where we should see if we can grab the mud connector detail url.
    if($get_details) {
      my $req2 = HTTP::Request->new(POST => $detail_url);
         $req2->content_type('application/x-www-form-urlencoded');
         $req2->content("Mode=MUD&mud=$mud");
      print STDERR "Fetching details for $mud\n";
      if($req2) {
        my $res2 = undef;
        eval {
          local $SIG{ALRM} = sub { die "Exceeded Timeout of $timeout for $detail_url\n" };
          alarm $timeout;
          $res2 = $lwp->request($req2);
          alarm 0;
        };
        die "Timeout" if($EVAL_ERROR and ($EVAL_ERROR =~ /^Exceeded Timeout/));
        if((defined $res2) and $res2->is_success) {
          print STDERR "Got it!\n";
          my $detail_page = $res2->content;
          $detail_page =~ /<b>Mud\s+Created:?<\/b>:?\s*(?:<.*?>)*([^<]+)/i;
          $date_created = $1;
          $detail_page =~ /<b>Code\s+Base:?<\/b>:?\s*(?:<.*?>)*([^<]+)/i;
          $code_base = $1;
          print STDERR "Date Created: $date_created\n";
          print STDERR "Code Base: $code_base\n";

          #TMC Ranking:</u></b></a> 407 <br>
          #<b>Last Updated:</b> <font color=#00FFFF>June 19, 2007</font><br>
          #<b>Mud Created:</b> <font color=#00FFFF>April, 2003</font><br> 
          #<b>Code Base</b>: <a href="http://ftp.game.org/cgi-bin/directory?/pub/mud/diku/merc/smaug" target="ftp.game.org"><font color=00ffff>Smaug</font></a>  Highly Modified, of course <br>


        }
      }
    }

    $mud =~ s/\+/ /g;
    print STDERR "Trying $mud $site $port\n";
    my ($connect, $screen, $html, $png) = fetch_login_screen($site, $port);

    my $b64ansi = $screen ? encode_base64($screen) : encode_base64('');
    my $b64html = $html ? encode_base64($html) : undef;
    my $b64png  = $png ? encode_base64($png) : undef;

    if($connect and defined $screen ) {
      my $rv = undef;
      $rv = $sth->execute($mud, $site, $port, $b64ansi, $b64html, $b64png, 't', $code_base);
      if($rv) {
        $dbc->commit;
        print " INSERT: $counter\t$mud\t$site\t$port online\n";
      } else {
        if ($DBI::errstr =~ /duplicate key/) {
          $dbc->rollback;
          my $rv2 = $sth2->execute('t', $b64ansi, $b64html, $b64png, $mud, $code_base);
          if($rv2) {
            $dbc->commit;
            print " UPDATE: $counter\t$mud\t$site\t$port online\n";
          } else {
            $dbc->rollback;
            print "   FAIL: $counter\t$mud\t$site\t$port online\n";
          }
        } else {
          print STDERR $DBI::errstr."\n";
          $dbc->rollback;
          print "   FAIL: $counter\t$mud\t$site\t$port online\n";
        }
      }
    } else {
      my $rv = undef;
      $rv = $sth->execute($mud, $site, $port, undef, undef, undef, 'f', $code_base);
      if($rv) {
        $dbc->commit;
        print " INSERT: $counter\t$mud\t$site\t$port offline\n";
      } else {
        if ($DBI::errstr =~ /duplicate key/) {
          $dbc->rollback;
          my $rv2 = $sth2->execute('f', undef, undef, undef, $mud, $code_base);
          if($rv2) {
            $dbc->commit;
            print " UPDATE: $counter\t$mud\t$site\t$port offline\n";
          } else {
            $dbc->rollback;
            print "   FAIL: $counter\t$mud\t$site\t$port offline\n";
          }
        } else {
          print STDERR $DBI::errstr."\n";
          $dbc->rollback;
          print "   FAIL: $counter\t$mud\t$site\t$port offline\n";
        }
      }
    }
    $batch++;
    $counter++;
  }
  return ($batch, $counter);
}

sub get_url {
  my $url = shift;

  return undef if !defined $url;
  my $timeout = 90;
  my $lwp = LWP::UserAgent->new();
     $lwp->timeout($timeout/2);
  my $request = HTTP::Request->new(GET => $url);
  my $response = undef;
  #print "Fetching... $url\n";
  if($request) {
    eval {
      local $SIG{ALRM} = sub { die "Exceeded Timeout of $timeout for $url\n" };
      alarm $timeout;
      $response = $lwp->request($request);
      alarm 0;
    };
    warn "Timeout" if($EVAL_ERROR and ($EVAL_ERROR =~ /^Exceeded Timeout/));
    return $response->content if((defined $response) and $response->is_success);
  }
  return undef;
}

sub table_get {
  my $data = shift;
  my $seperator = shift || ',';
  my $conditions = shift;
  my @results;

  my $format = $seperator if $seperator =~ /%[\-]?[\d]*([\.][\d]+)?[sdx]/;

  if(defined $data) {
    my $parser = HTML::TableExtract->new( %$conditions );
    my $edata = decode_utf8($data);
    $parser->parse($edata);
    foreach my $table ($parser->table_states) {
      foreach my $row ($table->rows) {
        if(defined $format) {
          push @results, sprintf($format, @$row);
        } else {
          push @results, join($seperator, @$row);
        }
      }
    }
  }
  return \@results;
}

=head1 SQL

CREATE TABLE mudlist (
    mud_id serial NOT NULL,
    name text NOT NULL,
    site text NOT NULL,
    port integer NOT NULL,
    ansi_login text,
    last_verified timestamp without time zone DEFAULT now() NOT NULL,
    html_login text,
    added timestamp without time zone DEFAULT now() NOT NULL,
    visited integer,
    live boolean,
    png_login text
);

CREATE FUNCTION fn_b64encode(text) RETURNS text
    AS '
  use MIME::Base64;

  my $thing = $_[0];
  my $b64 = encode_base64($thing);

  return $b64;
'
    LANGUAGE plperlu;

CREATE FUNCTION fn_b64decode(text) RETURNS text
    AS '
  use MIME::Base64;

  my $b64 = $_[0];
  my $thing = decode_base64($b64);

  return $thing;
'
    LANGUAGE plperlu;

CREATE FUNCTION fn_ansi2html(text) RETURNS text
    AS '
  use HTML::FromANSI;
  use Term::ANSIColor;
  use MIME::Base64;

  $HTML::FromANSI::Options{fill_cols} = 1;
  $HTML::FromANSI::Options{linewrap} = 0;
  $HTML::FromANSI::Options{html_entitiy} = 1;
  $HTML::FromANSI::Options{cols} = 80;
  #$HTML::FromANSI::Options{rows} = 24;
  $HTML::FromANSI::Options{lf_to_crlf} = 1;
  $HTML::FromANSI::Options{show_cursor} = 0;
  $HTML::FromANSI::Options{style} = "line-height: 1.0; letter-spacing: 0; font-size: 12pt";
 
  my $login = decode_base64($_[0]);
  my $ansi = ansi2html($login);
  my $b64 = encode_base64($ansi);
  return $b64;
'
    LANGUAGE plperlu;

CREATE FUNCTION fn_ansi2png(text) RETURNS text
    AS '
  use Image::ANSI;
  use MIME::Base64;

  my $login = decode_base64($_[0]);
  return undef if ! $login;
  my $img = Image::ANSI->new( string => $login );
  return undef if ! $img;
  my %opts = ( mode => "full", font => undef, palette => undef );
  my $png = $img->as_png( %opts );
  return undef if ! $png;
  my $b64 = encode_base64($png);

  return $b64;
'
    LANGUAGE plperlu;

CREATE FUNCTION fn_update_login() RETURNS "trigger"
    AS '
BEGIN
  new.html_login = fn_ansi2html(fn_b64decode(new.ansi_login));
  new.png_login = fn_ansi2png(fn_b64decode(new.ansi_login));
  RETURN new;
END;'
    LANGUAGE plpgsql;


CREATE UNIQUE INDEX ix_mudname ON mudlist USING btree (name);
CREATE INDEX ix_live ON mudlist USING btree (live);
CREATE INDEX ix_verified ON mudlist USING btree (last_verified);
ALTER TABLE ONLY mudlist
    ADD CONSTRAINT mudlist_pkey PRIMARY KEY (mud_id);

CREATE TRIGGER trg_update_login
    BEFORE INSERT OR UPDATE ON mudlist
    FOR EACH ROW
    EXECUTE PROCEDURE fn_update_login();

=cut


