#!/usr/bin/perl -w

package MudConvert::DUMP::Output;

use strict;
use English;
use Data::Serializer;

use base 'Exporter';

our @EXPORT_OK = qw( dump_game );

sub dump_game {
  my $cfg = shift;
  my $data = shift;
  my $dump = Data::Serializer->new(
					serializer		=> 'Data::Dumper',
					#serializer		=> 'YAML',
					digester		=> 'MD5',
					#cipher			=> 'Blowfish',
					#secret			=> 'tardis',
					portable		=> 0,
					compress		=> 0,
					serializer_token	=> 1,
				);

  return undef if !$data;
  my $dump_dir = $cfg->{'destination-dir'}."/DUMP";
  my $dump_file = "$dump_dir/data.dump";
  if( !(mkdir $dump_dir) ) {
    #system("echo rm -rf ".$cfg->{'destination-dir'}.'/WileyMUD' $cfg->{'destination-dir'}.'/WileyMUD.old');
    #system("echo mv ".$cfg->{'destination-dir'}.'/WileyMUD'." ".$cfg->{'destination-dir'}.'/WileyMUD.old');
    printf STDERR "FATAL: Cannot create output directory (%s) for DUMP!\n", $dump_dir;
    #return undef;
  }
  if( !(open DUMP, ">$dump_file") ) {
    printf STDERR "FATAL: Cannot create output DUMP file (%s) for DUMP!\n", $dump_file;
    return undef;
  }
  print DUMP $dump->freeze($data);
  close DUMP;
  return $data;
}

1;

