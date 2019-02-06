#!/usr/bin/perl -w

#package MudConvert;

use strict;
use English;
use Time::HiRes         qw( time );
use Data::Dumper;
use lib ".";

use MudConvert::Utils   qw( center );
use MudConvert::Usage   qw( parse_options do_help );
use MudConvert::Inputs  qw( %input_format validate_input );
use MudConvert::Outputs qw( %output_format validate_output );

our $VERSION       = '$Revision: 1.0 $'; $VERSION =~ s/^\D+//; $VERSION =~ s/\s+\$$//;
our $VERSION_DATE  = '$Date: 2003/12/16 15:53:13 $'; $VERSION_DATE =~ s/^\D+//; $VERSION_DATE =~ s/\s.*\$$//;
# Startup time is $BASETIME

$| = 1;

printf "--- %s ---\n",   center("Mud Converter Project, Version $VERSION ($VERSION_DATE)", 70);
printf "--- %s ---\n\n", center("Running as PID $PID under perl ".$PERL_VERSION." on $OSNAME", 70);
my $cfg = parse_options();
print STDERR Dumper($cfg) if $cfg->{'debug'};

my $input_choice = undef;
my @output_choices = ();
my %results = ();

if( !$cfg->{'source-dir'} or
    ! -d $cfg->{'source-dir'} ) {
  $cfg->{'source-dir'} = 'world';
  print STDERR "Using default \"world\" input directory\n\n";
}

if( !$cfg->{'destination-dir'} or
    ! -d $cfg->{'destination-dir'} ) {
  $cfg->{'destination-dir'} = 'output';
  print STDERR "Using default \"output\" directory\n\n";
}

if( $cfg->{'input-format'} and
    $cfg->{'input-format'}[-1] and
    validate_input($cfg->{'input-format'}[-1])) {
  $input_choice = $cfg->{'input-format'}[-1];
  print "--> $input_choice INPUT format selected\n";
} else {
  print STDERR "You must specify exactly one valid input-format!\n\n";
  do_help();
  exit 0;
}

if( $cfg->{'output-format'} and
    (scalar @{ $cfg->{'output-format'} } > 0)) {
  foreach (@{ $cfg->{'output-format'} }) {
    if( validate_output($_)) {
      push @output_choices, $_;
    } else {
      print STDERR "$_ is NOT a valid output-format!\n\n";
      do_help();
      exit 0;
    }
  }
} else {
  print STDERR "Not specifying ANY output format will only product debugging information!\n\n";
}


my $data = $input_format{$input_choice}{'CodeRef'}($cfg);
$results{$_} = $output_format{$_}{'CodeRef'}($cfg, $data) foreach (@output_choices);
print STDERR Dumper(\%results) if $cfg->{'debug'};

printf "Conversion process took %6.3f seconds.\n", time - $BASETIME;

exit 1;

