#!/usr/bin/perl -w

use strict;

my $repositories = {
    'dgd'       => {
        'dgd'                   => { 'type' => 'git', 'url' => 'git@github.com:dworkin/dgd' },
        'lpc-doc'               => { 'type' => 'git', 'url' => 'git@github.com:dworkin/lpc-doc.git' },
        'lpc-ext'               => { 'type' => 'git', 'url' => 'git@github.com:dworkin/lpc-ext' },
        'kernellib'             => { 'type' => 'git', 'url' => 'git@github.com:dworkin/kernellib.git' },
        'dgd-httpd'             => { 'type' => 'git', 'url' => 'git@github.com:dworkin/jorinde.git' },
        'gurbalib'              => { 'type' => 'git', 'url' => 'git@github.com:dworkin/gurbalib' },
        'kernellib-shentino'    => { 'type' => 'git', 'url' => 'git@github.com:shentino/kernellib.git' },
        'gurbalib-sirdude'      => { 'type' => 'git', 'url' => 'git@github.com:sirdude/gurbalib.git' },
        'cloud'                 => { 'type' => 'git', 'url' => 'git@github.com:dworkin/cloud-server.git' },
    },
    'fluffos'   => {
        'quix_fluffos'          => { 'type' => 'git', 'url' => 'git@github.com:quixadhal/fluffos.git' },
        'sunyc_fluffos'         => { 'type' => 'git', 'url' => 'git@github.com:fluffos/fluffos.git' },
        'ds'                    => { 'type' => 'git', 'url' => 'git@github.com:quixadhal/deadsouls.git' },
        'lima'                  => { 'type' => 'git', 'url' => 'git@github.com:quixadhal/lima.git' },
        'diku2ds'               => { 'type' => 'git', 'url' => 'git@github.com:LashMUD/DikuMud-to-Dead-Souls-Port.git' },
    },
    'ldmud'     => {
#        'ldmud-3.3'             => { 'type' => 'svn', 'url' => 'svn://svn.bearnip.com/ldmud/3.3/trunk' },
        'ldmud'                 => { 'type' => 'git', 'url' => 'git@github.com:ldmud/ldmud.git' },
        'ldlib'                 => { 'type' => 'git', 'url' => 'git@github.com:ldmud/ldlib.git' },
        'ldmud-extensions'      => { 'type' => 'git', 'url' => 'git@github.com:ldmud/ldmud-extensions.git' },
        'proftpd-mod_mud'       => { 'type' => 'git', 'url' => 'git@github.com:ldmud/proftpd-mod_mud.git' },
#        'proftpd-cvs'           => { 'type' => 'cvs', 'url' => ':pserver:anonymous@proftp.cvs.sourceforge.net:2401/cvsroot/proftp', 'co' => 'proftpd' },
#        'proftpd-userguide-cvs' => { 'type' => 'cvs', 'url' => ':pserver:anonymous@proftp.cvs.sourceforge.net:2401/cvsroot/proftp', 'co' => 'Userguide' },
        'simud'                 => { 'type' => 'git', 'url' => 'git://github.com/shentino/simud' },
    },
    'diku'      => {
        'quix_smaug'            => { 'type' => 'git', 'url' => 'git@github.com:quixadhal/SmaugFUSS.git' },
        'smaug'                 => { 'type' => 'git', 'url' => 'git@github.com:InfiniteAxis/SmaugFUSS.git' },
        'samson_smaug'          => { 'type' => 'git', 'url' => 'git@github.com:Arthmoor/SmaugFUSS.git' },
        'ackfuss'               => { 'type' => 'git', 'url' => 'git@github.com:Kline-/ackfuss.git' },
#        'roh'                   => { 'type' => 'svn', 'url' => 'http://svn.rohonline.net/roh' },
        'brokendreams'          => { 'type' => 'git', 'url' => 'git@github.com:syn2083/BrokenDreams-RoT-2014.git' },
        'wileymud'              => { 'type' => 'git', 'url' => 'git@github.com:quixadhal/WileyMUD.git' },
        'afkmud'                => { 'type' => 'git', 'url' => 'git@github.com:Arthmoor/AFKMud.git' },
        'tbamud'                => { 'type' => 'git', 'url' => 'git@github.com:tbamud/tbamud.git' },
    },
    'other'      => {
        'plaintext'             => { 'type' => 'git', 'url' => 'git@github.com:arendjr/PlainText.git' },
        'mudcore'               => { 'type' => 'git', 'url' => 'git@github.com:endgame/MudCore.git' },
        'pyom'                  => { 'type' => 'git', 'url' => 'git@bitbucket.org:mudbytes/pyom.git' },
        'miniboa'               => { 'type' => 'git', 'url' => 'git@github.com:marlboromoo/miniboa.git' },
        'havoc'                 => { 'type' => 'git', 'url' => 'git@github.com:plamzi/Havoc.git' },
        'arkmud'                => { 'type' => 'svn', 'url' => 'https://rottenkomquat.dyndns.org/svn/mud/' },
        'fuzzballmuck'          => { 'type' => 'git', 'url' => 'git@github.com:fuzzball-muck/fuzzball.git' },
    },
    'clients'      => {
        'lyntin'                => { 'type' => 'cvs', 'url' => ':pserver:anonymous@lyntin.cvs.sourceforge.net:/cvsroot/lyntin', 'co' => 'lyntin' },
        'tf-utf8'               => { 'type' => 'git', 'url' => 'git@github.com:kruton/tinyfugue.git' },
        'putty-tray'            => { 'type' => 'git', 'url' => 'git@github.com:FauxFaux/PuTTYTray.git' },
        'mudrammer'             => { 'type' => 'git', 'url' => 'git@github.com:splinesoft/MUDRammer.git' },
    },
};

sub fetch_repository {
    my $style = shift or return 0;
    my $repo = shift or return 0;
    my $data = shift or return 0;
    my $type = $data->{'type'} or return 0;
    my $url = $data->{'url'} or return 0;

    mkdir $style if ! -d $style;
    chdir $style;
    if( $type eq 'git' ) {
        if( -d $repo ) {
            chdir $repo;
            print "==> Updating $style/$repo from $url\n";
            system("git pull");
            chdir '..';
        } else {
            print "==> Fetching $style/$repo from $url\n";
            system("git clone $url $repo");
        }
    } elsif( $type eq 'svn' ) {
        if( -d $repo ) {
            chdir $repo;
            print "==> Updating $style/$repo from $url\n";
            system("svn update");
            chdir '..';
        } else {
            print "==> Fetching $style/$repo from $url\n";
            system("svn checkout $url $repo");
        }
    } elsif( $type eq 'cvs' ) {
        if( -d $repo ) {
            chdir $repo;
            print "==> Updating $style/$repo from $url\n";
            system("cvs update");
            chdir '..';
        } else {
            my $co = $data->{'co'};
            print "==> Fetching $style/$repo from $url\n";
            system("cvs -d $url -z3 co $co");
            rename $co, $repo;
        }
    }
    chdir '..';
    print "\n";
    return 1;
}

foreach my $style (sort keys %$repositories) {
    foreach my $repo (sort keys %{ $repositories->{$style} }) {
        fetch_repository($style, $repo, $repositories->{$style}->{$repo});
    }
}
