#!/usr/bin/perl -w

use Data::Dumper;

##SKILL
#Name         acetum primus~
#Type         Spell
#Info         909
#Flags        0
#Target       1
#Minpos       105
#Slot         302
#Mana         15
#Rounds       8
#Code         spell_acetum_primus
#Dammsg       Acetum Primus~
#Wearoff      !WEAROFF!~
#Minlevel     37
#End

my $file = '';
my $lines = 0;
my %sn = ();

open FP, "smaug_skills.dat" or die "Cannot open smaug_skills.dat";
while(<FP>) {
  $lines++;
  $file .= $_;
}
close FP;
printf STDERR "smaug_skills.dat - %d lines, %d bytes\n", $lines, length($file);

foreach my $sk (split /^\#SKILL$/ms, $file) {
  my ($name, $slot) = (undef,undef);
  foreach my $line (split /\n/, $sk) {
    my @word = split /\s+/, $line, 2;
    next if !(defined $word[0]) or !(defined $word[1]);
    $name = $word[1] if $word[0] eq 'Name';
    $slot = $word[1] if $word[0] eq 'Slot';
    $name =~ s/~$// if (defined $name);
    $sn{$name} = $slot if (defined $name) and (defined $slot);
    next if (defined $name) and (defined $slot) and (defined $sn{$name});
  }
}

#printf("%s:%d\n", $_, $sn{$_}) foreach (sort keys %sn);

my @tmp = (sort{ $a <=> $b } (values %sn));
my ($lowest, $highest) = ($tmp[0], $tmp[-1]);

   @tmp = (sort{ ((defined $a) ? length $a : 0) <=> ((defined $b) ? length $b: 0) } (keys %sn));
my $longest = $tmp[-1];

open FP, ">include/smaug_skills.h";

my %rev = (reverse %sn);
printf FP "char * const    smaug_skill_names     [] =\n{\n";
for(my $i = 0; $i <= $highest; $i++) {
  printf FP "  \"%s\",%s/* %s */\n", (defined $rev{$i}) ? $rev{$i} : "NONE",
                                  (defined $rev{$i}) ?
                                    (" "x((length $longest) - (length $rev{$i}))) :
                                    (" "x((length $longest) - 4)),
                                  (defined $rev{$i}) ? $i : "";
}
printf FP "  \"END\"\n};\n";

close FP;

