#!/usr/bin/perl -w

package MudConvert::Usage;

use strict;
use English;
use Getopt::Long;
use Data::Dumper;
use MudConvert::Inputs  qw( %input_format );
use MudConvert::Outputs qw( %output_format );
use base 'Exporter';

our @EXPORT_OK = qw( parse_options do_help );

sub parse_options {
  my @tmp_input = ();
  my @tmp_output = ();

  my %cfg = (
              'debug'				=> undef,
              'input-format'			=> [],
              'output-format'			=> [],
              'quiet'				=> undef,
              'verbose'				=> undef,
              'source-dir'			=> undef,
              'destination-dir'			=> undef,
              'include-short-in-long'		=> undef,
              'obvious-exits'			=> undef,
              'hard-returns'			=> undef,
              'pitch-black'			=> undef,
            );

  Getopt::Long::Configure("gnu_getopt");
  Getopt::Long::Configure("auto_version");
  GetOptions(
              'debug|D'				=> \$cfg{'debug'},
              'help|h'				=> sub { do_help() },
              'input-format|i=s'		=> \@tmp_input,
              'output-format|o=s',		=> \@tmp_output,
              'quiet|q',			=> \$cfg{'quiet'},
              'verbose|v+',			=> \$cfg{'verbose'},
              'source-dir|s=s',			=> \$cfg{'source-dir'},
              'destination-dir|d=s',		=> \$cfg{'destination-dir'},
              'include-short-in-long!',		=> \$cfg{'include-short-in-long'},
              'obvious-exits!',			=> \$cfg{'obvious-exits'},
              'hard-returns!',			=> \$cfg{'hard-returns'},
              'pitch-black!',			=> \$cfg{'pitch-black'},
            );

  $cfg{'input-format'} = \@tmp_input;
  $cfg{'output-format'} = \@tmp_output;
print Dumper(\%cfg);
  return \%cfg;
}

sub do_help {
  print STDERR <<EOM
Quixadhal's WileyMUD III conversion program: $main::VERSION

usage:  $PROGRAM_NAME [-Dhqv] [-s src] [-d dest] -i format -o format [-o format...]
long options:
  --debug                  - Enable debugging spam
  --help                   - This helpful help!
  --quiet                  - Run with minimal output (disables debug)
  --verbose                - Output blow-by-blow progress indications
  --input-format           - Select an appropriate input format to convert FROM
                             Supported formats:
EOM
  ;
  printf STDERR ("%29.29s%16.16s - %s\n", "", $_, $input_format{$_}{'Type'}) foreach (keys %input_format);
  print STDERR <<EOM
  --output-format          - Select one or more output formats to dump TO
                             Supported formats:
EOM
  ;
  printf STDERR ("%29.29s%16.16s - %s\n", "", $_, $output_format{$_}{'Type'}) foreach (keys %output_format);
  print STDERR <<EOM
  --source-dir             - Sets the source directory (default ./world)
  --destination-dir        - Sets the destination directory (default ./output)

Game options (prefix with "no-" to disable):
  --include-short-in-long  - Includes the short-description of a
                             room in the long-description field
                             DEFAULT:  OFF
  --obvious-exits          - Causes exits to be obvious by default
                             DEFAULT:  ON
  --hard-returns           - Force formatting to match original,
                             otherwise free-form is allowed
                             DEFAULT:  ON
  --one-big-domain         - Force all zones in source file to be put
                             into a single domain
                             DEFAULT:  OFF
  --pitch-black            - Make all "dark" rooms absolutely dark,
                             instead of just indoor-dark
                             DEFAULT:  OFF

EOM
  ;
  exit(1);
}

1;

