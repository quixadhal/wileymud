#!/usr/bin/perl -w

package MudConvert::Utils;

use strict;
use English;
use base 'Exporter';

our @EXPORT_OK = qw( vNull float center spin 
                  vnum_index_file
                  find_error_line
                  error_trace
                  report_error
                );

sub vNull {
  my $thing = shift;

  return (defined $thing)? $thing : 'NULL';
}

sub float {
  my $num = shift;

  return 0.0 + $num;
}

sub center {
  my $str = shift || '';
  my $width = shift || 78;

  return " "x$width if (length $str) < 1;
  return $str if (length $str) >= $width;
  my $len = (int ((length $str) / 2));
  $str =~ /(^.{$len})(.*?$)/;
  my ($left, $right) = ($1, $2);
  my $space_count = $width - (length $left) - (length $right);
  my $left_spaces = " "x(int ($space_count / 2));
  my $right_spaces = " "x($space_count - (length $left_spaces));
  my $result = $left_spaces.$left.$right.$right_spaces;
  return $result;
}

sub spin {
  my $count = shift || rand;

  return ($count%4 == 0) ? "|": ($count%4 == 1) ? "/" : ($count%4 == 2) ? "-" : "\\";
}

sub vnum_index_file {
  my $cfg = shift;
  my $filename = shift;
  my $vnum_map = {};

  die "No file specified!" if !(defined $filename);
  die "Cannot open file $filename!" if !(-r $filename);
  print "Indexing $filename..." if !$cfg->{'quiet'};
  open FP, "$filename";
  my $line = 1;
  my $count = 0;
  my $prev_pos = tell FP;
  while(<FP>) {
    chomp;
    if( $_ =~ /^\#(\d+)/ ) {
      my $vnum = $1;
      my $len = length "$vnum";
      printf "%d%s%s", $vnum, spin($count), "\b"x($len + 1) if $cfg->{'verbose'};
      report_error($vnum_map, $vnum, $_, pos($_), "WARNING", "Duplicate VNUM entry in $filename!")
        if (exists $vnum_map->{$vnum});
      $vnum_map->{$vnum}->{'VNum'} = $vnum;
      $vnum_map->{$vnum}->{'Filename'} = $filename;
      $vnum_map->{$vnum}->{'Number'} = $count++;
      $vnum_map->{$vnum}->{'Line'} = $line;
      $vnum_map->{$vnum}->{'BytePos'} = $prev_pos;
    }
    $line++;
    $prev_pos = tell FP;
  }
  close FP;
  print "done\n" if !$cfg->{'quiet'};
  return $vnum_map;
}

sub find_error_line {
  my $str = shift;
  my $pos = shift;
  my $line_num = shift || 1;
  
  my @lines = (split /\n/, $str);
  my $line = $line_num;
  my $spot = 0;
  foreach (@lines) {
    $spot += (length $_) + 1;
    return $line if $pos <= $spot;
    $line++;
  }
  return $line;
}

sub error_trace {
  my $str = shift;
  my $pos = shift;
  my $line_num = shift || 1;
  my $msg = shift;
  
  my @lines = (split /\n/, $str);
  my @output = ();
  my $spot = 0;
  my $marked_yet = 0;
  my $line = $line_num;
  push @output, "Trace:";
  foreach (@lines) {
    push @output, (sprintf "%05d: %s", $line, $_);
    $spot += (length $_) + 1;
    if( $pos <= $spot and !$marked_yet) {
      my $point = (length $_) - ($spot - $pos);
      my $out = '-------'.('-' x $point).'^'.((defined $msg)? " $msg": "");
      push @output, $out;
      $marked_yet = 1;
    }
    $line++;
  }
  return join("\n", @output);
}

sub report_error {
  my $data = shift;
  my $vnum = shift;
  my $str = shift;
  my $pos = shift;
  my $level = shift || "ERROR";
  my $msg = shift;
  my $do_trace = shift || ($level eq "FATAL" or $level eq "ERROR") ? 1 : 0;
  my @args = @_;

  my $vnum_exists = ((defined $data) && (defined $vnum) && (defined $data->{$vnum}));
  printf STDERR ((defined $msg)? "$level: $msg ": "$level: ")."in file %s, room #%d, near line %d\n",
                ($vnum_exists ? vNull($data->{$vnum}->{'Filename'}) : 'NULL', vNull($vnum)),
                find_error_line($str, $pos, ($vnum_exists? $data->{$vnum}->{'Line'}: 0)), (@args);
  printf STDERR "%s\n", error_trace($str, $pos, ($vnum_exists? $data->{$vnum}->{'Line'}: 0), $msg)
    if $do_trace;
}

1;

