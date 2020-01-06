#!/usr/bin/perl -w

package MudConvert::DUMP::Input;

use strict;
use English;
use Data::Serializer;

use base 'Exporter';

our @EXPORT_OK = qw( load_game );

sub load_game {
  my $cfg = shift;
  my $dump = Data::Serializer->new();

  my $dump_dir = $cfg->{'source-dir'};
  my $dump_file = "$dump_dir/data.json";

  if( (! -d $dump_dir) or (! -r $dump_file) or !(open FOO, $dump_file) ) {
    printf STDERR "FATAL: Cannot open input DUMP file (%s) for DUMP!\n", $dump_file;
    return undef;
  }
  my @lines = ();
  push @lines, $_ while(<FOO>);
  close FOO;
  my $input_data = $dump->thaw(join("", @lines));
  
  return $input_data;
}

1;

