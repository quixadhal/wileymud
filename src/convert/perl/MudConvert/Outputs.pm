#!/usr/bin/perl -w

package MudConvert::Outputs;

use strict;
use English;
use Data::Dumper;

use base 'Exporter';
use MudConvert::WileyMUD::Output;
use MudConvert::DUMP::Output;
use MudConvert::Report::Output;

our @EXPORT_OK = qw( %output_format validate_output );

our %output_format = (
                       'DUMP' => {
                                   'Name'	=> 'DUMP',
                                   'Type'	=> 'Data Dump',
                                   'CodeRef'	=> \&MudConvert::DUMP::Output::dump_game,
                                 },
                       'Report' => {
                                   'Name'	=> 'Report',
                                   'Type'	=> 'Human Parsable',
                                   'CodeRef'	=> \&MudConvert::Report::Output::dump_game,
                                 },
                      'WileyMUD' => {
                                  'Name'	=> 'WileyMUD',
                                  'Type'	=> 'DikuMUD',
                                  'CodeRef'	=> \&MudConvert::WileyMUD::Output::dump_game,
                                },
                     );

sub validate_output {
  my $choice = shift;

  return undef if !(defined $choice);
  return grep { $_ eq $choice } (keys %output_format);
}

1;

