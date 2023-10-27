#!/usr/bin/perl -w

use strict;

open FP, "paths" or die "Cannot find path file: $!";
while(<FP>) {
  chomp;
  next if ! /^\/def\s+2/;
  /^\/def\s+2(\w+)\s+\=\s+say\s+Energize\!\%;brief\%;compact\%;(.*)\%;compact\%;brief\%;grin$/;
  my ($label,$body) = ($1,$2);
  my @path = split /\%;/, $body;
  my ($x,$y,$z) = (0,0,0);
  foreach (@path) {
    $x++ if $_ eq "e";
    $x-- if $_ eq "w";
    $y++ if $_ eq "n";
    $y-- if $_ eq "s";
    $z++ if $_ eq "u";
    $z-- if $_ eq "d";
  }
  printf "%-20s(%4d,%4d,%4d)\n", $label, $x, $y, $z;
}
close FP;
