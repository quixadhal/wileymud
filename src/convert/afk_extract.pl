#!/usr/bin/perl -w

use Data::Dumper;

##SKILL
#Name         acetum primus~
#Type         Spell
#Info         10125
#Author      Smaug~
#Flags        6400
#Target       1
#Minpos       fighting~
#Saves        5
#Slot         302
#Mana         70
#Rounds       24
#Rent            29000
#Code         spell_smaug
#Dammsg       acetum primus~
#Wearoff      !Acetum Primus!~
#Hitchar      &GYou invoke a stream of corrosive acid which burns $N!&z~
#Hitvict      &GYou are corroded by a stream of acid from $n's hands!&z~
#Hitroom      &G$n burns $N horribly with a stream of acid from $s hands!&z ~
#Misschar     &GYour stream of acid misses $N by a hair!&z~
#Missvict     &G$n very nearly misses you with $s stream of acid!&z~
#Missroom     &G$n misses spraying $N with $s stream of acid!&z~
#Diechar      &GYou reduce $N into a pile of sticky mess with your acid stream!&z~
#Dievict      &GYou are reduced into a pile of sticky mess by the stream of acid!&z~
#Dieroom      &G$N is reduced to a bubbling mess by $n's acid stream!&z~
#Dice         ld6+(l/2) { 500~
#Minlevel     60
#End

my $file = '';
my $lines = 0;
my %sn = ();

open FP, "skills.dat" or die "Cannot open skills.dat";
while(<FP>) {
  $lines++;
  $file .= $_;
}
close FP;
printf STDERR "skills.dat - %d lines, %d bytes\n", $lines, length($file);

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

open FP, ">include/afk_skills.h";

my %rev = (reverse %sn);
printf FP "char * const    afk_skill_names     [] =\n{\n";
for(my $i = 0; $i <= $highest; $i++) {
  printf FP "  \"%s\",%s/* %s */\n", (defined $rev{$i}) ? $rev{$i} : "NONE",
                                  (defined $rev{$i}) ?
                                    (" "x((length $longest) - (length $rev{$i}))) :
                                    (" "x((length $longest) - 4)),
                                  (defined $rev{$i}) ? $i : "";
}
printf FP "  \"END\"\n};\n";

close FP;

