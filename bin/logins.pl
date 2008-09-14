#!/usr/bin/perl -w

use strict;
use English;

use File::Find;
use PerlIO::via::Bzip2 level => 9; # Maximum compression
use Time::HiRes qw(time);
use Date::Calc qw(Date_to_Time Time_to_Date Delta_YMDHMS);
use POSIX qw(strftime);

#<: 1080211.225840.323 : Quixadhal (adork@andropov.shadowlord.org) has connected.
#<: 1070509.115506.827 : Lost [c-71-230-249-144.hsd1.pa.comcast.net] new player.
#<: 1070107.202544.655 : Losing player: Nomad.
#<: 1080202.045640.329 : Closing all sockets.
#<: 1080201.050133.962 : Entering game loop.

my $log_dir = "/home/wiley/lib/log/07.12";

my $open_boot = undef;
my %log_entries = ();
my %open_characters = ();

find(\&f_logs, $log_dir);

#foreach (sort keys %$connections) {
#  next if $connections->{$_}->{mode} eq 'disconnect';
#  my $at = strftime("%A, %B %d, %Y", (reverse (Time_to_Date($_))));
#  printf("%s was on for %s at %s.\n",
#    $connections->{$_}->{name},
#    find_logout_time($_),
#    $at);
#}

sub length_of_time {
  my $start_time = shift;
  my $end_time = shift;

  print "$start_time, $end_time\n";
  my ($iy,$im,$id,$iH,$iM,$iS) = Time_to_Date(int $start_time);
  my ($oy,$om,$od,$oH,$oM,$oS) = Time_to_Date(int $end_time);
  my ($dy,$dm,$dd,$dH,$dM,$dS) = Delta_YMDHMS( $iy,$im,$id,$iH,$iM,$iS, $oy,$om,$od,$oH,$oM,$oS);

  my $answer  = undef;
     $answer .= sprintf("%d year", $dy)		if $dy > 0;
     $answer .= sprintf("s")			if $dy > 1;
     $answer .= sprintf(", ")			if $dy > 0;
     $answer .= sprintf("%d month", $dm)	if $dm > 0;
     $answer .= sprintf("s")			if $dm > 1;
     $answer .= sprintf(", ")			if $dm > 0;
     $answer .= sprintf("%d day", $dd)		if $dd > 0;
     $answer .= sprintf("s")			if $dd > 1;
     $answer .= sprintf(", ")			if $dd > 0;
     $answer .= sprintf("%d hour", $dH)		if $dH > 0;
     $answer .= sprintf("s")			if $dH > 1;
     $answer .= sprintf(", ")			if $dH > 0;
     $answer .= sprintf("%d minute", $dM)	if $dM > 0;
     $answer .= sprintf("s")			if $dM > 1;
     $answer .= sprintf(", and ")		if $dM > 0;
     $answer .= sprintf("%d second", $dS)	if $dS > 0;
     $answer .= sprintf("s")			if $dS > 1;
     $answer = "an unknown amount of time."	if !defined $answer;
  return $answer;
}

sub record_runtime {
  my $start_time = shift;
  my $end_time = shift;

  printf "WileyMUD was up for %s.\n", length_of_time( $start_time, $end_time );
}

sub record_playtime {
  my $character = shift;
  my $start_time = shift;
  my $end_time = shift;

  printf "%s was online for %s.\n", $character, length_of_time( $start_time, $end_time );
}

sub add_log_entry {
  my $mode = shift;
  my $filename = shift;
  my $line_number = shift;
  my $timestamp = shift;
  my $character = shift;
  my $address = shift;

  return undef if !defined $mode;
  return undef if !defined $filename;
  return undef if !defined $line_number;
  return undef if !defined $timestamp;

  $timestamp =~ s/^10/200/;
  $timestamp =~ /(\d{4})(\d{2})(\d{2})\.(\d{2})(\d{2})(\d{2})\.(\d+)/;
  my ($year, $month, $day, $hour, $minute, $second, $micro) = ($1, $2, $3, $4, $5, $6, $7);
  my $unixtime = Date_to_Time($year, $month, $day, $hour, $minute, $second);
  $unixtime = 0.0 + $unixtime + $micro / 1000;
  
  $log_entries{$unixtime}->{mode} = $mode;
  $log_entries{$unixtime}->{filename} = $filename;
  $log_entries{$unixtime}->{line_number} = $line_number;
  $log_entries{$unixtime}->{timestamp} = $timestamp;
  $log_entries{$unixtime}->{character} = $character;
  $log_entries{$unixtime}->{address} = $address;

  return $unixtime;
}

sub f_logs {
  my $filename = $File::Find::name;
  my $fp = undef;

  return unless -f $filename;
  return unless $filename =~ /runlog\..*/;

  if ($filename =~ /\.bz2$/) {
    open($fp, "<:via(Bzip2)", $filename);
  } else {
    open($fp, $filename);
  }
  return if !$fp;
  print "Processing... $filename\n";

  my $line_number = 0;
  my $last_timestamp = undef;

  while(my $line = <$fp>) {
    my ($timestamp, $character, $address, $unixtime) = (undef, undef, undef, undef);

    chomp;
    $line_number++;
    $line =~ /^<:\s*([\d\.]+)\s*:/;
    ($last_timestamp) = ($1);
    next if !defined $last_timestamp;
    $last_timestamp =~ s/^10/200/;
    $last_timestamp =~ /(\d{4})(\d{2})(\d{2})\.(\d{2})(\d{2})(\d{2})\.(\d+)/;
    my ($lyear, $lmonth, $lday, $lhour, $lminute, $lsecond, $lmicro) = ($1, $2, $3, $4, $5, $6, $7);
    $last_timestamp = Date_to_Time($lyear, $lmonth, $lday, $lhour, $lminute, $lsecond);
    $last_timestamp = 0.0 + $last_timestamp + $lmicro / 1000;

    if ($line =~ /has\s+connected/) {
      #<: 1080211.225840.323 : Quixadhal (adork@andropov.shadowlord.org) has connected.
      $line =~ /^<:\s*([\d\.]+)\s*:\s*(\w+)\s*\(\s*(.*?)\s*\)\s+has\s+connected\./;
      ($timestamp, $character, $address) = ($1, $2, $3);
      $unixtime = add_log_entry('connect', $filename, $line_number, $timestamp, $character, $address);
      $open_characters{$character} = $unixtime;
    } elsif ($line =~ /new\s+player/) {
      #<: 1070509.115506.827 : Lost [c-71-230-249-144.hsd1.pa.comcast.net] new player.
      $line =~ /^<:\s*([\d\.]+)\s*:\s*(\w+)\s*\[\s*(.*?)\s*\]\s+new\s+player\./;
      ($timestamp, $character, $address) = ($1, $2, $3);
      $unixtime = add_log_entry('new', $filename, $line_number, $timestamp, $character, $address);
      $open_characters{$character} = $unixtime;
    } elsif ($line =~ /Losing\s+player/) {
      #<: 1070107.202544.655 : Losing player: Nomad.
      $line =~ /^<:\s*([\d\.]+)\s*:\s*Losing player\s*:\s*(\w+)\./;
      ($timestamp, $character, $address) = ($1, $2, undef);
      $unixtime = add_log_entry('disconnect', $filename, $line_number, $timestamp, $character, $address);
      if(defined $open_characters{$character}) {
        record_playtime($character, $open_characters{$character}, $unixtime);
        delete $open_characters{$character};
      }
    } elsif ($line =~ /Entering\s+game\s+loop/) {
      #<: 1080201.050133.962 : Entering game loop.
      $line =~ /([\d\.]+)\s*:\s*Entering game loop\./;
      ($timestamp, $character, $address) = ($1, undef, undef);
      $unixtime = add_log_entry('startup', $filename, $line_number, $timestamp, $character, $address);
      $open_boot = $unixtime;
    } elsif ($line =~ /Closing\s+all\s+sockets/) {
      #<: 1080202.045640.329 : Closing all sockets.
      $line =~ /([\d\.]+)\s*:\s*Closing all sockets\./;
      ($timestamp, $character, $address) = ($1, undef, undef);
      $unixtime = add_log_entry('shutdown', $filename, $line_number, $timestamp, $character, $address);
      if(defined $open_boot) {
        foreach (keys %open_characters) {
          next if !defined $open_characters{$_};
          record_playtime($_, $open_characters{$_}, $unixtime);
          delete $open_characters{$_};
        }
        record_runtime($open_boot, $unixtime);
        $open_boot = undef;
      }
    } else {
      next;
    }
  }

  # This is to catch crashes, which won't have the shutdown log entries.
  foreach (keys %open_characters) {
    next if !defined $open_characters{$_};
    record_playtime($_, $open_characters{$_}, $last_timestamp);
    delete $open_characters{$_};
  }
  record_runtime($open_boot, $last_timestamp);

  close($fp);
}

