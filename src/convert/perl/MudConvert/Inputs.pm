#!/usr/bin/perl -w

package MudConvert::Inputs;

use strict;
use English;
use base 'Exporter';
use MudConvert::WileyMUD::Input;
use MudConvert::DUMP::Input;

our @EXPORT_OK = qw( %input_format validate_input );

our %input_format = (
                      'DUMP' => {
                                  'Name'	=> 'DUMP',
                                  'Type'	=> 'Data Dump',
                                  'CodeRef'	=> \&MudConvert::DUMP::Input::load_game,
                                },
                      'WileyMUD' => {
                                  'Name'	=> 'WileyMUD',
                                  'Type'	=> 'DikuMUD',
                                  'CodeRef'	=> \&MudConvert::WileyMUD::Input::load_game,
                                },
                    );

sub validate_input {
  my $choice = shift;

  return undef if !(defined $choice);
  return grep { $_ eq $choice } (keys %input_format);
}

1;

